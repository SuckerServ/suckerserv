// Original source code copied from engine/master.cpp found in Sauerbraten's source tree.

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <signal.h>

#include "cube.h"
#include "hopmod/hopmod.hpp"

#include "hopmod/lua/modules.hpp"
#include "hopmod/main_io_service.hpp"

#include <enet/time.h>
#include <iostream>

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

static bool debug = true;

static boost::thread::id main_thread;

static lua_State * L = NULL;
static lua::event_environment * event_environment = NULL;
static int lua_stack_size = 0;

static boost::asio::io_service main_io_service;

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

typedef boost::unordered_map<boost::shared_ptr<key>, const char* > prop_map;
typedef boost::unordered_map<std::string, prop_map > users_map;
typedef boost::unordered_map<std::string, users_map> domains_map;
static domains_map users;

/* Forward declaration of Lua value io functions */
#include "lua/push_function_fwd.hpp"
namespace lua{
void push(lua_State * L, string value);
void push(lua_State * L, __uid_t value);
} //namespace lua

#include "lua/push_function.hpp"

/*
    Lua value io functions for cube2 types
*/
namespace lua{
void push(lua_State * L, string value)
{
    lua_pushstring(L, value);   
}
void push(lua_State * L, __uid_t value)
{
    lua_pushinteger(L, value);
}
} //namespace lua

template<typename FunctionPointerType>
static void bind_function(lua_State * L, int table, const char * name, FunctionPointerType function)
{
    lua_pushstring(L, name);
    lua::push_function(L, function);
    lua_settable(L, table);
}

static void bind_function(lua_State * L, int table, const char * name, lua_CFunction function)
{
    lua_pushstring(L, name);
    lua_pushcfunction(L, function);
    lua_settable(L, table);
}

template<typename T, bool READ_ONLY, bool WRITE_ONLY>
static int variable_accessor(lua_State * L)
{
    T * var = reinterpret_cast<T *>(lua_touserdata(L, lua_upvalueindex(1)));
    if(lua_gettop(L) > 0) // Set variable
    {
        if(READ_ONLY) luaL_error(L, "variable is read-only");
        *var = lua::to(L, 1, lua::return_tag<T>());
        event_varchanged(event_listeners(), boost::make_tuple(lua_tostring(L, lua_upvalueindex(2))));
        return 0;
    }
    else // Get variable
    {
        if(WRITE_ONLY) luaL_error(L, "variable is write-only");
        lua::push(L, *var);
        return 1;
    }
}

template<bool READ_ONLY, bool WRITE_ONLY>
static int string_accessor(lua_State * L)
{
    char * var = reinterpret_cast<char *>(lua_touserdata(L, lua_upvalueindex(1)));
    if(lua_gettop(L) > 0) // Set variable
    {
        if(READ_ONLY) luaL_error(L, "variable is read-only");
        copystring(var, lua_tostring(L, 1));
        event_varchanged(event_listeners(), boost::make_tuple(lua_tostring(L, lua_upvalueindex(2))));
        return 0;
    }
    else // Get variable
    {
        if(WRITE_ONLY) luaL_error(L, "variable is write-only");
        lua::push(L, var);
        return 1;
    }
}

template<typename T>
static void bind_var(lua_State * L, int table, const char * name, T & var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, &var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, variable_accessor<T, false, false>, 2);
    lua_settable(L, table);
}

static void bind_var(lua_State * L, int table, const char * name, string var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, string_accessor<false, false>, 2);
    lua_settable(L, table);
}

lua::event_environment & event_listeners()
{
    static lua::event_environment unready_event_environment;
    if(!event_environment) return unready_event_environment;
    lua_stack_size = lua_gettop(L);
    return *event_environment;
}

#include "lua/library_extensions.hpp"
void load_extra_os_functions(lua_State * L)
{
    lua_getglobal(L, "os");
    
    if(lua_type(L, -1) != LUA_TTABLE)
    {
        std::cerr<<"Lua init error: the os table is not loaded"<<std::endl;
        return;
    }
    
    lua_pushliteral(L, "getcwd");
    lua_pushcfunction(L, lua::getcwd);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "mkfifo");
    lua_pushcfunction(L, lua::mkfifo);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "open_fifo");
    lua_pushcfunction(L, lua::open_fifo);
    lua_settable(L, -3);
    
    lua_pushliteral(L, "usleep");
    lua_pushcfunction(L, lua::usleep);
    lua_settable(L, -3);
    
    lua_pop(L, 1);
}

boost::asio::io_service & get_main_io_service()
{
    return main_io_service;
}

void adduser(const char * username, const char * domain, const char *pubkey, const char *rights)
{
    if(!domain) domain = "";
    users[domain][username][boost::shared_ptr<key>(new key(pubkey))] = rights;
    //users[domain][username] = rights;
    if(debug) std::cout<<"Adding user "<<username<<"@"<<(domain[0] ? domain : "<none>") << " with " << rights << " rights" <<std::endl;
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
    a.authchal = userIt->second.begin()->first->create_challenge(seed, sizeof(seed));
    
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
    const char *rights = userIt->second.begin()->second;
    
    outputf(c, "FoundId %u %s\n", id, rights);
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

void shutdown_lua()
{
    if(!L) return;
    
    delete event_environment;
    lua_close(L);
    
    event_environment = NULL;
    L = NULL;
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
    
    main_thread = boost::this_thread::get_id();

    signal_shutdown.connect(boost::bind(&shutdown_lua));
    signal_shutdown.connect(&cleanup_info_files_on_shutdown);

    L = luaL_newstate();
    luaL_openlibs(L);
    
    load_extra_os_functions(L);
    
#ifndef NO_CORE_TABLE
    lua_newtable(L);
    int core_table = lua_gettop(L);
    
    
    bind_function(L, core_table, "adduser", adduser);
    bind_function(L, core_table, "deleteuser", deleteuser);
    bind_function(L, core_table, "clearusers", clearusers);
    
    bind_function(L, core_table, "log_status", log_status);
    bind_function(L, core_table, "log_error", log_error);
    
    bind_function(L, core_table, "file_exists", file_exists);
    bind_function(L, core_table, "dir_exists", dir_exists);
    
    bind_function(L, core_table, "shutdown", _shutdown);
    
    
    lua_pushliteral(L, "vars");
    lua_newtable(L);
    int vars_table = lua_gettop(L);
    
    bind_var(L, vars_table, "serverport", port);
    bind_var(L, vars_table, "serverip", ip);    
    bind_var(L, vars_table, "debug", debug);
    
    lua_settable(L, -3); // Add vars table to core table
    lua_setglobal(L, "core"); // Add core table to global table
#endif
    event_environment = new lua::event_environment(L, NULL);
#ifndef NO_EVENTS
    register_event_idents(*event_environment); // Setup and populate the event table
#endif
    
    static const char * INIT_SCRIPT = "script/base/auth/server/init.lua";
    
    lua::module::open_filesystem(L);
    lua::module::open_crypto(L);
    lua::module::open_net2(L);
    lua::module::open_cubescript(L);
    
    #ifdef HAS_LSQLITE3
    luaopen_lsqlite3(L);
    #endif

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
    
    event_init(event_listeners(), boost::make_tuple());
    
    setupserver(port, (ip[0] ? ip : NULL));
    
    event_started(event_listeners(), boost::make_tuple());
    
    std::cout<<"*READY*"<<std::endl;
    std::cout.flush();

    domains_map::const_iterator domainIt;
    for(domainIt = users.begin(); domainIt != users.end(); domainIt++)
    {
        std::cout<< domainIt->first <<std::endl;
        users_map::const_iterator userIt;
        for(userIt = domainIt->second.begin(); userIt != domainIt->second.end(); userIt++)
        {
            std::cout<<"    "<< userIt->first <<std::endl;
        }
    }
    
    for(;;)
    {
        servtime = enet_time_get();
        checkclients();
    }
    
    return EXIT_SUCCESS;
}
