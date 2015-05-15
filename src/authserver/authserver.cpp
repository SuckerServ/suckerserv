// Original source code copied from engine/master.cpp found in Sauerbraten's source tree.

#ifdef WIN32
#define FD_SETSIZE 4096
#else
#include <sys/types.h>
#undef __FD_SETSIZE
#define __FD_SETSIZE 4096
#endif

#include "cube.h"
#include "hopmod.hpp"

#include <asio.hpp>
#include <asio/high_resolution_timer.hpp>

#include "hopmod/lua/modules.hpp"
#include "hopmod/main_io_service.hpp"

#include <enet/time.h>
#include <signal.h>
#include <iostream>

#define INPUT_LIMIT 4096
#define OUTPUT_LIMIT (64*1024)
#define CLIENT_TIME (3*60*1000)
#define AUTH_TIME (30*1000)
#define AUTH_LIMIT 100
#define AUTH_THROTTLE 1000
#define CLIENT_LIMIT 4096
#define DUP_LIMIT 16
#define KEEPALIVE_TIME (65*60*1000)
#define SERVER_LIMIT 4096
#define DEFAULT_SERVER_PORT 28787

using namespace asio;

static io_service main_io_service;

io_service & get_main_io_service()
{
    return main_io_service;
}

high_resolution_timer update_timer(main_io_service);

size_t tx_packets = 0 , rx_packets = 0, tx_bytes = 0, rx_bytes = 0;
bool reloaded = false;
bool restart_program;

namespace authserver {

int port = DEFAULT_SERVER_PORT;
string ip = "";
bool debug = true;

struct userkey
{
    const char *name;
    const char *desc;
    userkey() : name(NULL), desc(NULL) {}
    userkey(const char *name, const char *desc) : name(name), desc(desc) {}
};

static inline uint hthash(const userkey &k) { return ::hthash(k.name); }
static inline bool htcmp(const userkey &x, const userkey &y) { return !strcmp(x.name, y.name) && !strcmp(x.desc, y.desc); }

struct userinfo : userkey
{
    const char *pubkey;
    const char *privilege;
    userinfo() : pubkey(NULL), privilege(NULL) {}
    ~userinfo() { delete[] name; delete[] desc; delete[] pubkey; delete[] privilege; }
};
hashset<userinfo> users; 

void adduser(const char *name, const char *desc, const char *pubkey, const char *priv)
{
    userkey key(name, desc);
    userinfo &u = users[key];
    if(!u.pubkey) u.pubkey = newstring(pubkey);
    if(!u.name) u.name = newstring(name);
    if(!u.desc) u.desc = newstring(desc);
    if(!u.privilege) u.privilege = newstring(priv);
    event_adduser(event_listeners(), std::make_tuple(name, desc, pubkey, priv));
    if(debug) std::cout<<"Adding user "<<name<<"@"<<(desc[0] ? desc : "<none>")<<" with priv "<<priv<<std::endl;
} 

void deleteuser(const char *name, const char *desc)
{
    userkey key(name, desc);
    if(users.remove(key))
    {
        event_deleteuser(event_listeners(), std::make_tuple(name, desc));
        if(debug) std::cout<<"Removing user "<<name<<"@"<<(desc ? desc : "<none>")<<std::endl;
    }
    else
    {
        std::cerr<<"Error in removing user: user "<<name<<"@"<<(desc ? desc : "<none>")<<" not found."<<std::endl;
    }
} 

void clearusers()
{
    users.clear();
    event_clearusers(event_listeners(), std::make_tuple());
    if(debug) std::cout<<"Cleared all users"<<std::endl;
} 

struct authreq
{
    enet_uint32 reqtime;
    uint id;
    void *answer;
}; 

struct client
{
    ENetAddress address;
    ENetSocket socket;
    char input[INPUT_LIMIT];
    vector<char> output;
    int inputpos, outputpos;
    enet_uint32 connecttime, lastinput;
    int servport;
    enet_uint32 lastauth;
    vector<authreq> authreqs;

    bool authed;

    client() : inputpos(0), outputpos(0), servport(-1), lastauth(0), authed(false) {}
};  
vector<client *> clients;

void outputf(client &c, const char *fmt, ...);

ENetSocket serversocket = ENET_SOCKET_NULL;

time_t starttime;
enet_uint32 servtime = 0;

void fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char msg[256];
    vsprintf(msg, fmt, args);
    std::cerr<<msg<<std::endl;
    va_end(args);
    exit(EXIT_FAILURE);
}

void log_status(const char * msg){std::cout<<msg<<std::endl;}
void log_error(const char * msg){std::cerr<<msg<<std::endl;}

void purgeclient(int n)
{
    client &c = *clients[n];
    enet_socket_destroy(c.socket);
    delete clients[n];
    clients.remove(n);
}

void output(client &c, const char *msg, int len = 0)
{
    if(!len) len = strlen(msg);
    c.output.put(msg, len);
}

void outputf(client &c, const char *fmt, ...)
{
    string msg;
    va_list args;
    va_start(args, fmt);
    vformatstring(msg, fmt, args);
    va_end(args);

    output(c, msg);
}

void setupserver(int port, const char *ip = NULL)
{
    if(enet_initialize()<0) fatal("Unable to initialise enet");
    atexit(enet_deinitialize);

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    if(ip)
    {
        if(enet_address_set_host(&address, ip)<0)
            fatal("failed to resolve server address: %s", ip);
    }
    serversocket = enet_socket_create(ENET_SOCKET_TYPE_STREAM);
    if(serversocket==ENET_SOCKET_NULL || 
       enet_socket_set_option(serversocket, ENET_SOCKOPT_REUSEADDR, 1) < 0 ||
       enet_socket_bind(serversocket, &address) < 0 ||
       enet_socket_listen(serversocket, -1) < 0)
        fatal("failed to create server socket");
    if(enet_socket_set_option(serversocket, ENET_SOCKOPT_NONBLOCK, 1)<0)
        fatal("failed to make server socket non-blocking");

    enet_time_set(0);
    
    starttime = time(NULL);
    char *ct = ctime(&starttime);
    if(strchr(ct, '\n')) *strchr(ct, '\n') = '\0';
    printf("*** Starting auth server on %s %d at %s ***\n", ip ? ip : "localhost", port, ct);
}

void purgeauths(client &c)
{
    int expired = 0;
    loopv(c.authreqs)
    {
        if(ENET_TIME_DIFFERENCE(servtime, c.authreqs[i].reqtime) >= AUTH_TIME) 
        {
            outputf(c, "failauth %u\n", c.authreqs[i].id);
            freechallenge(c.authreqs[i].answer);
            expired = i + 1; 
        }
        else break;
    }
    if(expired > 0) c.authreqs.remove(0, expired);
}

void reqauth(client & c, uint id, char * name, char * domain)
{
    if(ENET_TIME_DIFFERENCE(servtime, c.lastauth) < AUTH_THROTTLE)
        return;
    c.lastauth = servtime;

    purgeauths(c);
    
    time_t t = time(NULL);
    char *ct = ctime(&t);
    if(ct) 
    { 
        char *newline = strchr(ct, '\n');
        if(newline) *newline = '\0'; 
    }
    
    string ip;
    if(enet_address_get_host_ip(&c.address, ip, sizeof(ip)) < 0) copystring(ip, "-");
    printf("%s: attempting \"%s\"@\"%s\" as %u from %s\n", ct ? ct : "-", name, domain ? domain : "", id, ip);
    
    
    userinfo *u = users.access(userkey(name, domain));
    
    if(!u)
    {
        outputf(c, "failauth %u\n", id);
        printf("Failed %u from %s: user %s@%s not found.\n", id, ip, name, domain ? domain : "<none>");
        return;
    }
    
    if(c.authreqs.length() >= AUTH_LIMIT)
    {
        outputf(c, "failauth %u\n", c.authreqs[0].id);
        freechallenge(c.authreqs[0].answer);
        c.authreqs.remove(0);
    }
    
    authreq &a = c.authreqs.add();
    a.reqtime = servtime;
    a.id = id;
    uint seed[3] = { starttime, servtime, randomMT() };

    static vector<char> buf;
    buf.setsize(0);
    void *pubkey = parsepubkey(u->pubkey);
    a.answer = genchallenge(pubkey, seed, sizeof(seed), buf);
    freepubkey(pubkey);

    outputf(c, "chalauth %u %s\n", id, buf.getbuf());
}

void confauth(client &c, uint id, const char *val)
{
    purgeauths(c);

    loopv(c.authreqs) if(c.authreqs[i].id == id)
    {
        string ip;
        if(enet_address_get_host_ip(&c.address, ip, sizeof(ip)) < 0) copystring(ip, "-");
        
        if(checkchallenge(val, c.authreqs[i].answer))
        {
            outputf(c, "succauth %u\n", id);
            printf("succeeded %u from %s\n", id, ip);
        }
        else 
        {
            outputf(c, "failauth %u\n", id);
            printf("failed %u from %s\n", id, ip);
        }
        freechallenge(c.authreqs[i].answer);
        c.authreqs.remove(i--);
        return;
    }
    outputf(c, "failauth %u\n", id);
}

void queryid(client & c, uint id, char * name, char * domain)
{
    
    string ip;
    if(enet_address_get_host_ip(&c.address, ip, sizeof(ip)) < 0) copystring(ip, "-");
    
    userinfo *u = users.access(userkey(name, domain));
    
    if(!u)
    {
        outputf(c, "NotFound %u\n", id);
        printf("Failed %u from %s: user %s@%s not found.\n", id, ip, name, domain ? domain : "<none>");
        return;
    }
    
    outputf(c, "FoundId %u %s\n", id, u->privilege);
}

bool checkclientinput(client &c)
{
    if(c.inputpos<0) return true;
    char *end = (char *)memchr(c.input, '\n', c.inputpos);
    while(end)
    {
        *end++ = '\0';
        c.lastinput = servtime;
        
        uint id;
        string user, domain, val;
        user[0]='\0';
        domain[0]='\0';
        val[0]='\0';

        if (debug)
        {
            std::cout<<c.input<<std::endl;
        }
        
        if(sscanf(c.input, "reqauth %u %100s %100s", &id, user, domain) >= 2)
        {
            reqauth(c, id, user, domain);
        }
        else if(sscanf(c.input, "confauth %u %100s", &id, val) == 2)
        {
            confauth(c, id, val);
        }
        else if(sscanf(c.input, "QueryId %u %100s %100s", &id, user, domain) == 3)
        {
            queryid(c, id, user, domain);
        }

        c.inputpos = &c.input[c.inputpos] - end;
        memmove(c.input, end, c.inputpos);

        end = (char *)memchr(c.input, '\n', c.inputpos);
    }
    return c.inputpos<(int)sizeof(c.input);
}

ENetSocketSet readset, writeset;

void checkclients()
{    
    ENetSocketSet readset, writeset;
    ENetSocket maxsock = serversocket;
    ENET_SOCKETSET_EMPTY(readset);
    ENET_SOCKETSET_EMPTY(writeset);
    ENET_SOCKETSET_ADD(readset, serversocket);
    
    loopv(clients)
    {
        client &c = *clients[i];
        if(c.outputpos < c.output.length()) ENET_SOCKETSET_ADD(writeset, c.socket);
        else ENET_SOCKETSET_ADD(readset, c.socket);
        maxsock = max(maxsock, c.socket);
    }
    if(enet_socketset_select(maxsock, &readset, &writeset, 1000)<=0) return;

    if(ENET_SOCKETSET_CHECK(readset, serversocket))
    {
        ENetAddress address;
        ENetSocket clientsocket = enet_socket_accept(serversocket, &address);
        if(clients.length()>=CLIENT_LIMIT) enet_socket_destroy(clientsocket);
        else if(clientsocket!=ENET_SOCKET_NULL)
        {
            int dups = 0, oldest = -1;
            loopv(clients) if(clients[i]->address.host == address.host) 
            {
                dups++;
                if(oldest<0 || clients[i]->connecttime < clients[oldest]->connecttime) oldest = i;
            }
            if(dups >= DUP_LIMIT) purgeclient(oldest);
                
            client *c = new client;
            c->address = address;
            c->socket = clientsocket;
            c->connecttime = servtime;
            c->lastinput = servtime;
            clients.add(c);
        }
    }
    
    loopv(clients)
    {
        client &c = *clients[i];
        if((c.outputpos < c.output.length()) && ENET_SOCKETSET_CHECK(writeset, c.socket))
        {
            const char *data = c.output.getbuf();
            int len = c.output.length();
            ENetBuffer buf;
            buf.data = (void *)&data[c.outputpos];
            buf.dataLength = len-c.outputpos;
            tx_bytes += buf.dataLength;
            tx_packets++;
            int res = enet_socket_send(c.socket, NULL, &buf, 1);
            if(res>=0) 
            {
                c.outputpos += res;
                if(c.outputpos>=len)
                {
                    c.output.setsize(0);
                    c.outputpos = 0;
                }
            }
            else { purgeclient(i--); continue; }
        }
        if(ENET_SOCKETSET_CHECK(readset, c.socket))
        {
            ENetBuffer buf;
            buf.data = &c.input[c.inputpos];
            buf.dataLength = sizeof(c.input) - c.inputpos;
            rx_bytes += buf.dataLength;
            rx_packets++;
            int res = enet_socket_receive(c.socket, NULL, &buf, 1);
            if(res>0)
            {
                c.inputpos += res;
                c.input[min(c.inputpos, (int)sizeof(c.input)-1)] = '\0';
                if(!checkclientinput(c)) { purgeclient(i--); continue; }
            }
            else { purgeclient(i--); continue; }
        }
        if(c.output.length() > OUTPUT_LIMIT) { purgeclient(i--); continue; }
        if(ENET_TIME_DIFFERENCE(servtime, c.lastinput) >= CLIENT_TIME) { purgeclient(i--); continue; }
    }
}

void stopauthserver(int)
{
    if(serversocket != ENET_SOCKET_NULL) enet_socket_destroy(serversocket);
    serversocket = ENET_SOCKET_NULL;

    std::error_code error;

    update_timer.cancel(error);
    if(error)
        std::cerr<<"Error while trying to stop the update timer: "<<error.message()<<std::endl;
}

void signal_shutdown(int val)
{
    shutdown_lua();
    delete_temp_files_on_shutdown(val);
}

static void initiate_shutdown()
{
    authserver::stopauthserver(SHUTDOWN_NORMAL);

    event_shutdown(event_listeners(), std::make_tuple(static_cast<int>(SHUTDOWN_NORMAL)));
    authserver::signal_shutdown(SHUTDOWN_NORMAL);

    // Now wait for the main event loop to process work that is remaining and then exit
    get_main_io_service().stop();
}

void shutdown()
{
    get_main_io_service().post(initiate_shutdown);
}

static void shutdown_from_signal(int i)
{
    authserver::shutdown();
}

#include "authserver_functions.cpp"

} //namespace authserver

void update_server(const std::error_code & error);

void sched_next_update()
{
    std::chrono::duration<long int, std::ratio<1l, 1000000000l> > expires_from_now = update_timer.expires_from_now();

    if(expires_from_now < std::chrono::duration<long int, std::ratio<1l, 1000000000l> >::zero())
    {
        update_timer.expires_from_now(std::chrono::duration<long int, std::milli>(5));
        update_timer.async_wait(update_server);
    }
}

void update_server(const std::error_code & error = std::error_code())
{
    sched_next_update();

    authserver::servtime = enet_time_get();
    authserver::checkclients();
}

static void init_authserver()
{
    init_lua();
    lua_State * L = get_lua_state();
    
    static const char * INIT_SCRIPT = "script/base/auth/server/init.lua";

    if(luaL_loadfile(L, INIT_SCRIPT) == 0)
    {
     	event_listeners().add_listener("init"); // Take the value of the top of the stack to add
        // to the init listeners table
    }
    else
    {
        std::cerr<<"error during initialization: "<<lua_tostring(L, -1)<<std::endl;
       	lua_pop(L, 1);
    }
    
    event_init(event_listeners(), std::make_tuple());
}

static void reload_authserver_now()
{
    event_reloadhopmod(event_listeners(), std::make_tuple());

    reloaded = true;

    event_shutdown(event_listeners(), std::make_tuple(static_cast<int>(SHUTDOWN_RELOAD)));

    authserver::signal_shutdown(SHUTDOWN_RELOAD);

    init_authserver();

    event_started(event_listeners(), std::make_tuple());

    std::cout<<"-> Reloaded Authserver."<<std::endl;

    reloaded = false;
}

void reload_authserver()
{
    get_main_io_service().post(reload_authserver_now);
}

void restart_now()
{
    std::cout<<"-> Authserver restarting..."<<std::endl;
    restart_program = true;
    authserver::shutdown();
}

int main(int argc, char **argv)
{
    restart_program = false;

    struct sigaction terminate_action;
    sigemptyset(&terminate_action.sa_mask);
    terminate_action.sa_handler = authserver::shutdown_from_signal;
    terminate_action.sa_flags = 0;
    sigaction(SIGINT, &terminate_action, NULL);
    sigaction(SIGTERM, &terminate_action, NULL);

    init_authserver();
    
    authserver::setupserver(authserver::port, (authserver::ip[0] ? authserver::ip : NULL));
    
    event_started(event_listeners(), std::make_tuple());
    
    std::cout<<"*READY*"<<std::endl;
    std::cout.flush();
    
    sched_next_update();
    
    try
    {
        main_io_service.run();
    }
    catch(const std::system_error & se)
    {
        std::cerr<<se.what()<<std::endl;
           throw;
    }
    
    if(restart_program)
    {
       	execv(argv[0], argv);
    }

    return EXIT_SUCCESS;
}
