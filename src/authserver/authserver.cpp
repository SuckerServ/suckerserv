// Original source code copied from engine/master.cpp found in Sauerbraten's source tree.

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include "cube.h"
#include "hopmod/hopmod.hpp"
#include "hopmod/string_var.hpp"
#include "hopmod/utils.hpp"

#include "hopmod/lua/modules.hpp"
#include "hopmod/main_io_service.hpp"

#include <enet/time.h>
#include <iostream>

#include <fungu/script.hpp>
using namespace fungu;

#ifdef HAS_LSQLITE3
extern "C"{
    int luaopen_lsqlite3(lua_State * L);
}
#endif

#define INPUT_LIMIT 4096
#define OUTPUT_LIMIT (64*1024)
#define CLIENT_TIME (3*60*1000)
#define AUTH_TIME (60*1000)
#define AUTH_LIMIT 100
#define CLIENT_LIMIT 8192
#define DUP_LIMIT 16
#define KEEPALIVE_TIME (65*60*1000)
#define SERVER_LIMIT (10*1024)
#define DEFAULT_SERVER_PORT 28787

static bool debug = false;

class key;

class challenge
{
friend class key;
public:
    ~challenge()
    {
        freechallenge(m_answer);
    }
    
    const char * get_challenge()const
    {
        return m_challenge.getbuf();
    }
    
    bool expected_answer(const char * foreign)const
    {
        return checkchallenge(foreign, m_answer);
    }
private:
    challenge(){}
    challenge(const challenge &){}
    vector<char> m_challenge;
    void * m_answer;
};

class key
{
public:
    key(const char * stringform)
    {
        m_key = parsepubkey(stringform);
    }
    
    ~key()
    {
        if(m_key) freepubkey(m_key);
    }
    
    challenge * create_challenge(const void * seed, size_t seedsize)const
    {
        challenge * chal = new challenge;
        chal->m_answer = genchallenge(m_key, seed, seedsize, chal->m_challenge);
        return chal;
    }
private:
    key(const key &){}
    void * m_key;
};

typedef boost::unordered_map<std::string, boost::shared_ptr<key> > users_map;
typedef boost::unordered_map<std::string, users_map> domains_map;
static domains_map users;

void adduser(const char * username, const char * domain, const char *pubkey)
{
    if(!domain) domain = "";
    users[domain][username] = boost::shared_ptr<key>(new key(pubkey));
    if(debug) std::cout<<"Adding user "<<username<<"@"<<(domain[0] ? domain : "<none>")<<std::endl;
}

void deleteuser(const char * name, const char * domain)
{
    domains_map::iterator domainIt = users.find(domain);
    
    if(domainIt == users.end())
    {
        std::cerr<<"Error in removing user: domain '"<<domain<<"' not found."<<std::endl;
        return;
    }
    
    if(domainIt->second.erase(name) == 0)
    {
        std::cerr<<"Error in removing user: user '"<<name<<"' not found."<<std::endl;
        return;
    }
    
    if(debug) std::cout<<"Removing user "<<name<<"@"<<(domain ? domain : "<none>")<<std::endl;
}

void clearusers()
{
    users.clear();
    if(debug) std::cout<<"Cleared all users"<<std::endl;
}

struct client;

struct authreq
{
    enet_uint32 reqtime; 
    uint id;
    challenge * authchal;
    client * c;
    
    authreq()
     :authchal(NULL)
    {
        
    }
    
    ~authreq()
    {
        delete authchal;
    }
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
    vector<authreq> authreqs;

    client() : inputpos(0), outputpos(0), servport(-1) {}
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

void conoutfv(int type, const char *fmt, va_list args)
{
    char msg[256];
    vsprintf(msg, fmt, args);
    std::cout<<msg<<std::endl;
}

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(CON_INFO, fmt, args);
    va_end(args);
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(type, fmt, args);
    va_end(args);
}

void log_status(const char * msg){std::cout<<msg<<std::endl;}
void log_error(const char * msg){std::cout<<msg<<std::endl;}

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
    conoutf("*** Starting auth server on %s %d at %s ***", ip ? ip : "localhost", port, ct);
}

void purgeauths(client &c)
{
    int expired = 0;
    loopv(c.authreqs)
    {
        if(ENET_TIME_DIFFERENCE(servtime, c.authreqs[i].reqtime) >= AUTH_TIME) 
        {
            outputf(c, "failauth %u\n", c.authreqs[i].id);
            expired = i + 1;
        }
        else break;
    }
    if(expired > 0) c.authreqs.remove(0, expired);
}

void reqauth(client & c, uint id, char * name, char * domain)
{
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
    conoutf("%s: attempting \"%s\"@\"%s\" as %u from %s", ct ? ct : "-", name, domain ? domain : "", id, ip);
    
    bool found = false;
    domains_map::const_iterator domainIt = users.find(domain ? domain : "");
    users_map::const_iterator userIt;
    
    if(domainIt != users.end())
    {
        userIt = domainIt->second.find(name);
        if(userIt != domainIt->second.end()) found = true;
    }
    
    if(c.authreqs.length() >= AUTH_LIMIT)
    {
        outputf(c, "failauth %u\n", c.authreqs[0].id);
        c.authreqs.remove(0);
    }
    
    if(!found)
    {
        outputf(c, "failauth %u\n", id);
        conoutf("failed %u from %s", id, ip);
        return;
    }
    
    authreq &a = c.authreqs.add();
    a.reqtime = servtime;
    a.id = id;
    uint seed[3] = { starttime, servtime, randomMT() };
    a.authchal = userIt->second->create_challenge(seed, sizeof(seed));
    
    outputf(c, "chalauth %u %s\n", id, a.authchal->get_challenge());
}

void confauth(client &c, uint id, const char *val)
{
    purgeauths(c);

    loopv(c.authreqs) if(c.authreqs[i].id == id)
    {
        string ip;
        if(enet_address_get_host_ip(&c.address, ip, sizeof(ip)) < 0) copystring(ip, "-");
        
        if(c.authreqs[i].authchal->expected_answer(val))
        {
            outputf(c, "succauth %u\n", id);
            conoutf("succeeded %u from %s", id, ip);
        }
        else 
        {
            outputf(c, "failauth %u\n", id);
            conoutf("failed %u from %s", id, ip);
        }
        c.authreqs.remove(i--);
        
        return;
    }
    outputf(c, "failauth %u\n", id);
}

void queryid(client & c, uint id, char * name, char * domain)
{
    domains_map::const_iterator domainIt = users.find(domain);
    users_map::const_iterator userIt;
    
    if(domainIt == users.end())
    {
        outputf(c, "DomainNotFound %u\n", id);
        return;
    }
    
    userIt = domainIt->second.find(name);
    if(userIt == domainIt->second.end())
    {
        outputf(c, "NameNotFound %u\n", id);
        return;
    }
    
    outputf(c, "FoundId %u\n", id);
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

static void shutdown_from_signal(int i)
{
    signal_shutdown(SHUTDOWN_NORMAL);
    exit(0);
}

static void _shutdown()
{
    shutdown_from_signal(SIGTERM);
}

int main(int argc, char **argv)
{
    struct sigaction terminate_action;
    sigemptyset(&terminate_action.sa_mask);
    terminate_action.sa_handler = shutdown_from_signal;
    terminate_action.sa_flags = 0;
    sigaction(SIGINT, &terminate_action, NULL);
    sigaction(SIGTERM, &terminate_action, NULL);
    
    int port = DEFAULT_SERVER_PORT;
    string ip = "";
    
    init_scripting();
    
    script::env & e = get_script_env();
    lua_State * L = e.get_lua_state();
    
    lua::module::open_filesystem(L);
    lua::module::open_crypto(L);
    
    #ifdef HAS_LSQLITE3
    luaopen_lsqlite3(L);
    #endif
    
    script::bind_var(port, "serverport", e);
    script::bind_var(ip, "serverip", e);
    
    script::bind_freefunc(adduser, "adduser", e);
    script::bind_freefunc(deleteuser, "deleteuser", e);
    script::bind_freefunc(clearusers, "clearusers", e);
    
    script::bind_freefunc(log_status, "log_status", e);
    script::bind_freefunc(log_error, "log_error", e);
    
    script::bind_freefunc(file_exists, "file_exists", e);
    script::bind_freefunc(dir_exists, "dir_exists", e);
    
    script::bind_freefunc(_shutdown, "shutdown", e);
    
    script::bind_var(debug, "debug", e);
    
    register_signals(e);
    
    init_script_pipe();
    open_script_pipe("authexec", 511, e);
    
    if(luaL_dofile(L, "./script/base/auth/server/init.lua")==1)
        log_error(lua_tostring(L, -1));
    
    setupserver(port, (ip[0] ? ip : NULL));
    
    signal_started();
    
    std::cout<<"*READY*"<<std::endl;
    std::cout.flush();
    
    for(;;)
    {
        servtime = enet_time_get();
        checkclients();
        run_script_pipe_service(servtime);
        cleanup_dead_slots();
    }
    
    return EXIT_SUCCESS;
}
