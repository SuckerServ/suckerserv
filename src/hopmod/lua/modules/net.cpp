/*
    A Lua Network Library module developed for Hopmod
    Copyright (C) 2009 Graham Daws
*/
#include <netinet/in.h> //byte ordering functions
#include <unistd.h> //unlink()
#include <iostream>
#include <boost/asio.hpp>
using namespace boost::asio;
#include "../../lib/buffered_socket.hpp"
#include "../../net/prefix_tree.hpp"
#include <boost/bind/protect.hpp>

#include <lua.hpp>
#include "lua/event.hpp"
#include "module.hpp"

// External
boost::asio::io_service & get_main_io_service();
lua::event_environment & event_listeners();

static const char * TCP_ACCEPTOR_MT = "lnetlib_tcp_acceptor";
static const char * BASIC_TCP_SOCKET_MT = "lnetlib_basic_tcp_socket";
static const char * TCP_CLIENT_SOCKET_MT = "lnetlib_tcp_client_socket";
static const char * BUFFER_MT = "lnetlib_buffer";
static const char * FILE_STREAM_MT = "lnetlib_file_stream";

static io_service * main_io;
static int library_instance = 0;

class tcp_socket: public ip::tcp::socket, 
                  public buffered_reading<ip::tcp::socket>
{
public:
    tcp_socket(boost::asio::io_service & service)
     :ip::tcp::socket(service),
      buffered_reading<ip::tcp::socket>(*static_cast<ip::tcp::socket *>(this)),
      m_resolver(service)
    {
        
    }
    
    template<typename ConnectHandler>
    void async_connect(const std::string & hostname, const std::string & port, ConnectHandler handler)
    {
        _async_connect(hostname, port, boost::protect(handler));
    }
private:
    template<typename ConnectHandler>
    void _async_connect(const std::string & hostname, const std::string & port, ConnectHandler handler)
    {
        ip::tcp::resolver::query query(hostname, port);
        m_resolver.async_resolve(query, boost::bind(&tcp_socket::resolve_handler<ConnectHandler>, this, library_instance, handler, _1, _2));
    }
    
    template<typename ConnectHandler>
    void resolve_handler(int instance, ConnectHandler handler, 
        const boost::system::error_code error, ip::tcp::resolver::iterator addresses)
    {
        if(instance != library_instance) return;
        
        if(error)
        {
            handler(error);
            return;
        }
        
        ip::tcp::socket * self = this;
        self->async_connect(addresses->endpoint(), handler);
    }
    
    ip::tcp::resolver m_resolver;
};

struct lnetlib_buffer
{
    char * start;
    char * end;
    char * produced;
    char * consumed;
    
    size_t get_read_left()const
    {
        return produced - consumed;
    }
    
    size_t get_write_left()const
    {
        return end - produced;
    }
    
    size_t get_size()const
    {
        return end - start;
    }
};

void * lua_aux_checkobject(lua_State * L, int narg, const char * tname)
{
    luaL_checktype(L, narg, LUA_TUSERDATA);
    luaL_getmetatable(L, tname);
    lua_getmetatable(L, narg);
    
    while(1)
    {
        if(lua_equal(L, -1, -2)) return lua_touserdata(L, narg);
        else
        {
            lua_getmetatable(L, -1);
            if(lua_type(L, -1) == LUA_TNIL) break;
            lua_replace(L, -2);
        }
    }
    
    luaL_argerror(L, narg, "incompatible type");
    return NULL;
}

template<typename EndpointType>
void push_endpoint(lua_State * L, const EndpointType & endpoint)
{
    lua_newtable(L);
    
    lua_pushinteger(L, endpoint.port());
    lua_setfield(L, -2, "port");
    
    lua_pushstring(L, endpoint.address().to_string().c_str());
    lua_setfield(L, -2, "ip");
    
    if(endpoint.address().is_v4())
    {
        lua_pushinteger(L, endpoint.address().to_v4().to_ulong());
        lua_setfield(L, -2, "iplong");
    }
}

void resolve_handler(int instance, lua_State * L, int luaFunctionCbRef, const boost::system::error_code ec, ip::tcp::resolver::iterator it)
{
    if(instance != library_instance) return;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, luaFunctionCbRef);
    luaL_unref(L, LUA_REGISTRYINDEX, luaFunctionCbRef);
    
    if(!ec)
    {
        lua_newtable(L);
        
        int count = 1;
        for(; it != ip::tcp::resolver::iterator(); ++it)
        {
            lua_pushinteger(L, count++);
            lua_pushstring(L, it->endpoint().address().to_string().c_str());
            lua_settable(L, -3);
        }
        
        if(lua_pcall(L, 1, 0, 0) != 0)
            event_listeners().log_error("net_resolve", lua_tostring(L, -1));
    }
    else
    {
        lua_pushstring(L, ec.message().c_str());
        
        if(lua_pcall(L, 1, 0, 0) != 0)
             event_listeners().log_error("net_resolve", lua_tostring(L, -1));
    }
}

int async_resolve(lua_State * L)
{
    static ip::tcp::resolver dns(*main_io);
    const char * hostname = luaL_checkstring(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    ip::tcp::resolver::query query(hostname, "");
    dns.async_resolve(query, boost::bind(resolve_handler, library_instance, L, luaL_ref(L, LUA_REGISTRYINDEX), _1, _2));
    return 0;
}

int acceptor_listen(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    try
    {
        acceptor->listen();
    }
    catch(boost::system::system_error se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 0;
}

int acceptor_close(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    try
    {
        acceptor->close();
    }
    catch(boost::system::system_error se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 0;
}

void async_accept_handler(int instance, lua_State * L, int luaFunctionCbRef, int socketRef, boost::system::error_code error)
{
    if(instance != library_instance) return;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, luaFunctionCbRef);
    luaL_unref(L, LUA_REGISTRYINDEX, luaFunctionCbRef);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, socketRef);
    luaL_unref(L, LUA_REGISTRYINDEX, socketRef);
    
    if(error)
    {
        lua_pop(L, 1); //socket object
        lua_pushstring(L, error.message().c_str());
        
        if(lua_pcall(L, 2, 0, 0) != 0)
            event_listeners().log_error("net_accept", lua_tostring(L, -1));
    }
    else
    {
        if(lua_pcall(L, 1, 0, 0) != 0)
             event_listeners().log_error("net_accept", lua_tostring(L, -1));
    }
}

int acceptor_async_accept(lua_State * L)
{
    luaL_checktype(L, 2, LUA_TFUNCTION);
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    
    tcp_socket * socket = new (lua_newuserdata(L, sizeof(tcp_socket))) tcp_socket(*main_io);
    luaL_getmetatable(L, BASIC_TCP_SOCKET_MT);
    lua_setmetatable(L, -2);
    int socketRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pushvalue(L, 2);
    acceptor->async_accept(*socket, boost::bind(async_accept_handler, library_instance, L, luaL_ref(L, LUA_REGISTRYINDEX), socketRef, _1));
    
    return 0;
}

int acceptor_gc(lua_State * L)
{
    reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT))->~basic_socket_acceptor<ip::tcp>();
    return 0;
}

int create_tcp_acceptor(lua_State * L)
{
    const char * ip = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    
    ip::tcp::acceptor * acceptor = new (lua_newuserdata(L, sizeof(ip::tcp::acceptor))) ip::tcp::acceptor(*main_io);
    
    // Setup metatable with __gc function set before bind() so the user-data object will be destroyed if the bind op fails.
    luaL_getmetatable(L, TCP_ACCEPTOR_MT);
    lua_setmetatable(L, -2);
    
    ip::tcp::endpoint server_ep(ip::address_v4::from_string(ip), port);
    
    acceptor->open(server_ep.protocol());
    acceptor->set_option(ip::tcp::acceptor::reuse_address(true));
    
    boost::system::error_code error;
    acceptor->bind(server_ep, error);
    if(error)
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
        return 2;
    }
    
    // Return user-data object
    return 1;
}

int acceptor_set_option(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "enable_connection_aborted"))
    {
        boost::asio::socket_base::enable_connection_aborted option(luaL_checkint(L, 3));
        acceptor->set_option(option);
    }
    else if(!strcmp(optname, "reuse_address"))
    {
        boost::asio::socket_base::reuse_address option(luaL_checkint(L, 3));
        acceptor->set_option(option);
    }
    
    return 0;
}

int acceptor_get_option(lua_State * L)
{
    ip::tcp::acceptor * acceptor = reinterpret_cast<ip::tcp::acceptor *>(luaL_checkudata(L, 1, TCP_ACCEPTOR_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "enable_connection_aborted"))
    {
        boost::asio::socket_base::enable_connection_aborted option;
        acceptor->get_option(option);
        lua_pushboolean(L, option.value());
        return 1;
    }
    else if(!strcmp(optname, "reuse_address"))
    {
        boost::asio::socket_base::reuse_address option;
        acceptor->get_option(option);
        lua_pushboolean(L, option.value());
        return 1;
    }
    
    return 0;
}

void create_acceptor_metatable(lua_State * L)
{
    luaL_newmetatable(L, TCP_ACCEPTOR_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    
    static luaL_Reg funcs[] = {
        {"__gc", acceptor_gc},
        {"listen", acceptor_listen},
        {"close", acceptor_close},
        {"async_accept", acceptor_async_accept},
        {"set_option", acceptor_set_option},
        {"get_option", acceptor_get_option},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
    lua_pop(L, 1);
}

int socket_gc(lua_State * L)
{
    reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT))->~tcp_socket();
    return 0;
}

int socket_close(lua_State * L)
{
    try
    {
        reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT))->close();
    }
    catch(boost::system::system_error se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 0;
}

void async_send_handler(int instance, lua_State * L, int functionRef, int stringRef, const boost::system::error_code error, const size_t len)
{
    if(instance != library_instance) return;
    
    luaL_unref(L, LUA_REGISTRYINDEX, stringRef);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);

    if(error) lua_pushstring(L, error.message().c_str());
    else lua_pushnil(L);
    
    lua_pushinteger(L, len);
    
    if(lua_pcall(L, 2, 0, 0) != 0)
         event_listeners().log_error("net_send", lua_tostring(L, -1));
}

int socket_async_send_buffer(lua_State *);

int socket_async_send(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));
    
    if(lua_type(L, 2) != LUA_TSTRING) return socket_async_send_buffer(L);

    luaL_checktype(L, 3, LUA_TFUNCTION);
    
    size_t stringlen;
    const char * str = lua_tolstring(L, 2, &stringlen);
    lua_pushvalue(L, -1);
    int stringRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_pushvalue(L, 3);
    socket->async_send(boost::asio::buffer(str, stringlen), boost::bind(async_send_handler, library_instance, L, luaL_ref(L, LUA_REGISTRYINDEX), stringRef, _1, _2));
    
    return 0;
}

void async_read_until_handler(int instance, lua_State * L, int functionRef, boost::asio::streambuf * buf, const boost::system::error_code error, const size_t readlen)
{
    if(instance != library_instance) return;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
    
    if(!error)
    {
        lua_pushlstring(L, boost::asio::buffer_cast<const char *>(*buf->data().begin()), readlen);
        buf->consume(readlen);
        lua_pushnil(L);
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
    }
    
    if(lua_pcall(L, 2, 0, 0) != 0)
         event_listeners().log_error("net_read_until", lua_tostring(L, -1));
}

int socket_async_read_until(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));

    const char * delim = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    socket->async_read_until(delim, boost::bind(async_read_until_handler, library_instance, L, functionRef, &socket->read_buffer(), _1, _2));
    
    return 0;
}

void async_read_handler(int instance, lua_State * L, int functionRef, streambuf * socketbuf, lnetlib_buffer * rbuf, int bufferRef, int reqreadsize, const boost::system::error_code error, const size_t readsize)
{
    if(instance != library_instance) return;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
    
    int nargs = 0;
    
    if(error)
    {
        lua_pushstring(L, error.message().c_str());
        nargs = 1;
    }
    else
    {
        memcpy(rbuf->produced, buffer_cast<const void *>(socketbuf->data()), reqreadsize);
        socketbuf->consume(reqreadsize);
        rbuf->produced += reqreadsize;
    }
    
    if(lua_pcall(L, nargs, 0, 0) != 0)
        event_listeners().log_error("net_read", lua_tostring(L, -1));
    
    luaL_unref(L, LUA_REGISTRYINDEX, bufferRef);
}

int socket_async_read(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));
    lnetlib_buffer * rbuf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 2, BUFFER_MT));
    
    // Create reference to buffer to keep it alive
    lua_pushvalue(L, 2);
    int bufferRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    size_t reqreadsize = rbuf->get_write_left();
    
    int function_arg_index = 3;
    
    if(lua_type(L, 3) == LUA_TNUMBER)
    {
        reqreadsize = lua_tointeger(L, 3);
        if(reqreadsize > rbuf->get_write_left()) luaL_argerror(L, 3, "would cause overflow");
        function_arg_index = 4;
    }
    
    luaL_checktype(L, function_arg_index, LUA_TFUNCTION);
    lua_pushvalue(L, function_arg_index);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    socket->async_read(reqreadsize, boost::bind(async_read_handler, library_instance, L, functionRef, &socket->read_buffer(), rbuf, bufferRef, reqreadsize, _1, _2));
    
    return 0;
}

void async_send_buffer_handler(int instance, lua_State * L, int functionRef, lnetlib_buffer * buf, int bufferRef, const boost::system::error_code error, const size_t written)
{
    if(instance != library_instance) return;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
    
    int nargs = 0;
    
    if(error)
    {
        lua_pushstring(L, error.message().c_str());
        nargs = 1;
    }
    else buf->consumed += written;
    
    if(lua_pcall(L, nargs, 0, 0) != 0)
        event_listeners().log_error("net_send_buffer", lua_tostring(L, -1));

    
    luaL_unref(L, LUA_REGISTRYINDEX, bufferRef);
}

int socket_async_send_buffer(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 2, BUFFER_MT));
    
    lua_pushvalue(L, 2);
    int bufferRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    size_t writesize = buf->get_read_left();
    
    int function_arg_index = 3;
    
    if(lua_type(L, 3) == LUA_TNUMBER)
    {
        writesize = lua_tointeger(L, 3);
        function_arg_index = 4;
    }
    
    luaL_checktype(L, function_arg_index, LUA_TFUNCTION);
    lua_pushvalue(L, function_arg_index);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    socket->async_send(buffer(buf->consumed, writesize), boost::bind(async_send_buffer_handler, library_instance, L, functionRef, buf, bufferRef, _1, _2));
    
    return 0;
}

int return_endpoint(lua_State * L, const boost::asio::ip::tcp::endpoint & endpoint, const boost::system::error_code error)
{
    if(error)
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
        return 2;
    }
    
    push_endpoint(L, endpoint);
    
    return 1;
}

int socket_local_endpoint(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));
    
    boost::system::error_code error;
    boost::asio::ip::tcp::endpoint endpoint = socket->local_endpoint(error);
    
    return return_endpoint(L, endpoint, error);
}

int socket_remote_endpoint(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));
    
    boost::system::error_code error;
    boost::asio::ip::tcp::endpoint endpoint = socket->remote_endpoint(error);
    
    return return_endpoint(L, endpoint, error);
}

int socket_cancel(lua_State * L)
{
    try
    {
        reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT))->cancel();
    }
    catch(boost::system::system_error se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 0;
}

int socket_shutdown_send(lua_State * L)
{
    try
    {
        reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT))->shutdown(ip::tcp::socket::shutdown_send);
    }
    catch(boost::system::system_error se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 0;
}

int socket_shutdown_receive(lua_State * L)
{
    try
    {
        reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT))->shutdown(ip::tcp::socket::shutdown_receive);
    }
    catch(boost::system::system_error se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 0;
}

int socket_shutdown(lua_State * L)
{
    try
    {
        reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT))->shutdown(ip::tcp::socket::shutdown_both);
    }
        catch(boost::system::system_error se)
    {
        luaL_error(L, "%s", se.code().message().c_str());
        return 0;
    }
    return 0;
}

int socket_set_option(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "keep_alive"))
    {
        boost::asio::socket_base::keep_alive option(luaL_checkint(L, 3));
        socket->set_option(option);
    }
    else if(!strcmp(optname, "linger"))
    {
        boost::asio::socket_base::linger option(luaL_checkint(L, 3), luaL_checkint(L, 4));
        socket->set_option(option);
    }
    
    return 0;
}

int socket_get_option(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, BASIC_TCP_SOCKET_MT));
    const char * optname = luaL_checkstring(L, 2);
    
    if(!strcmp(optname, "keep_alive"))
    {
        boost::asio::socket_base::keep_alive option;
        socket->get_option(option);
        lua_pushboolean(L, option.value());
        return 1;
    }
    else if(!strcmp(optname, "linger"))
    {
        boost::asio::socket_base::linger option;
        socket->get_option(option);
        
        lua_pushboolean(L, option.enabled());
        lua_pushinteger(L, option.timeout());
        return 2;
    }
    
    return 0;
}

void create_basic_socket_metatable(lua_State * L)
{
    luaL_newmetatable(L, BASIC_TCP_SOCKET_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    
    static luaL_Reg funcs[] = {
        {"__gc", socket_gc},
        {"close", socket_close},
        {"async_send", socket_async_send},
        {"async_read_until", socket_async_read_until},
        {"async_read", socket_async_read},
        {"local_endpoint", socket_local_endpoint},
        {"remote_endpoint", socket_remote_endpoint},
        {"cancel", socket_cancel},
        {"shutdown_send", socket_shutdown_send},
        {"shutdown_receive", socket_shutdown_receive},
        {"shutdown", socket_shutdown},
        {"set_option", socket_set_option},
        {"get_option", socket_get_option},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
    lua_pop(L, 1);
}

void async_read_from_handler(int instance, lua_State * L, int functionRef, lnetlib_buffer * rbuf, int bufferRef, ip::udp::endpoint * endpoint, const boost::system::error_code error, size_t readsize)
{
    if(instance != library_instance) return;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
    
    int nargs = 1;
    
    if(error)
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
        nargs = 2;
    }
    else
    {
        push_endpoint(L, *endpoint);
        delete endpoint;
        
        rbuf->produced += readsize;
    }
    
    if(lua_pcall(L, nargs, 0, 0) != 0)
         event_listeners().log_error("net_read_from", lua_tostring(L, -1));
    
    luaL_unref(L, LUA_REGISTRYINDEX, bufferRef);
}

int create_tcp_client(lua_State * L)
{
    tcp_socket * socket = new (lua_newuserdata(L, sizeof(tcp_socket))) tcp_socket(*main_io);
    
    // Setup metatable with __gc function set before bind() so the user-data object will be destroyed if the bind op fails.
    luaL_getmetatable(L, TCP_CLIENT_SOCKET_MT);
    lua_setmetatable(L, -2);
    
    boost::system::error_code ec;
    
    socket->open(ip::tcp::v4(), ec);
    if(ec)
    {
        luaL_error(L, "%s", ec.message().c_str());
        return 0;
    }
    
    // Return user-data object
    return 1;
}

void async_connect_handler(int instance, lua_State * L, int functionRef, const boost::system::error_code & error)
{
    if(instance != library_instance) return;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, functionRef);
    luaL_unref(L, LUA_REGISTRYINDEX, functionRef);
    
    int nargs = 0;
    
    if(error)
    {
        lua_pushstring(L, error.message().c_str());
        nargs = 1;
    }
    
    if(lua_pcall(L, nargs, 0, 0) != 0)
         event_listeners().log_error("net_connect", lua_tostring(L, -1));
}

int client_async_connect(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, TCP_CLIENT_SOCKET_MT));
    
    const char * hostname = luaL_checkstring(L, 2);
    const char * port= luaL_checkstring(L, 3);

    luaL_checktype(L, 4, LUA_TFUNCTION);
    lua_pushvalue(L, 4);
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);
    
    if(!socket->is_open())
    {
        boost::system::error_code ec;
        socket->open(ip::tcp::v4(), ec);
        if(ec)
        {
            luaL_error(L, "%s", ec.message().c_str());
            return 0;
        }
    }
    
    socket->async_connect(hostname, port, boost::bind(async_connect_handler, library_instance, L, functionRef, _1));
    
    return 0;
}

int tcp_client_bind(lua_State * L)
{
    tcp_socket * socket = reinterpret_cast<tcp_socket *>(lua_aux_checkobject(L, 1, TCP_CLIENT_SOCKET_MT));
    
    if(!socket->is_open())
    {
        boost::system::error_code ec;
        socket->open(ip::tcp::v4(), ec);
        if(ec)
        {
            luaL_error(L, "%s", ec.message().c_str());
            return 0;
        }
    }
    
    const char * ip = luaL_checkstring(L, 2);
    int port = luaL_checkinteger(L, 3);
    
    boost::system::error_code ec;
    
    ip::address_v4 host = ip::address_v4::from_string(ip, ec);
    if(ec)
    {
        lua_pushboolean(L, 1);
        lua_pushstring(L, ec.message().c_str());
        return 2;
    }
    
    socket->bind(ip::tcp::endpoint(host, port), ec);
    if(ec)
    {
        lua_pushboolean(L, 1);
        lua_pushstring(L, ec.message().c_str());
        return 2;
    }
    
    return 0;
}

void create_client_socket_metatable(lua_State * L)
{
    luaL_newmetatable(L, TCP_CLIENT_SOCKET_MT);
    
    luaL_getmetatable(L, BASIC_TCP_SOCKET_MT);
    lua_setmetatable(L, -2);
    
    static luaL_Reg funcs[] = {
        {"__gc", socket_gc},
        {"async_connect", client_async_connect},
        {"bind", tcp_client_bind},
        {NULL, NULL}
    };
    luaL_register(L, NULL, funcs);
    
    lua_setfield(L, -1, "__index");
    lua_pop(L, 1);
}

int buffer_to_string(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    unsigned int len = buf->produced - buf->consumed;
    lua_pushlstring(L, buf->consumed, len);
    buf->consumed += len;
    return 1;
}

int buffer_size(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    lua_pushinteger(L, buf->get_size());
    return 1;
}

int buffer_read_left(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    lua_pushinteger(L, buf->get_read_left());
    return 1;
}

int buffer_write_left(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    lua_pushinteger(L, buf->get_write_left());
    return 1;
}

int buffer_read_uint8(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    
    if(buf->consumed == buf->produced) return 0;
    
    unsigned char value = *buf->consumed;
    buf->consumed += 1;
    
    lua_pushinteger(L, value);
    
    return 1;
}

int buffer_read_uint16(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    
    if(buf->consumed + 2 > buf->produced) return 0;
    
    char * data = buf->consumed;
    unsigned char hi = *data;
    unsigned char lo = *(data + 1);
    unsigned short value = (hi << 8) + lo;
    buf->consumed += 2;
    
    lua_pushinteger(L, value);
    
    return 1;
}

int buffer_read_uint32(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    
    if(buf->consumed + 4 > buf->produced) return 0;
    
    char * data = buf->consumed;
    unsigned char b1 = *data;
    unsigned char b2 = *(data + 1);
    unsigned char b3 = *(data + 2);
    unsigned char b4 = *(data + 3);
    unsigned long value = (b1 << 24)  + (b2 << 16) + (b3 << 8) + b4;
    buf->consumed += 4;
    
    lua_pushinteger(L, value);
    
    return 1;
}

int buffer_read_string(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    int len = luaL_checkint(L, 2);
    if(buf->consumed + len > buf->produced) return 0;
    lua_pushlstring(L, buf->consumed, len);
    buf->consumed += len;
    return 1;
}

int buffer_reset(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    buf->consumed = buf->start;
    buf->produced = buf->start;
    return 0;
}

int buffer_write_uint8(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    if(buf->produced == buf->end) return 0;    
    int value = luaL_checkint(L, 2);
    *buf->produced = value;
    buf->produced++;
    lua_pushboolean(L, 1);
    return 1;
}

int buffer_write_uint16(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    if(buf->produced + 2 > buf->end) return 0;
    unsigned short value = luaL_checkint(L, 2);
    *buf->produced = value >> 8;
    *(buf->produced + 1) = value & 0xFF;
    buf->produced += 2;
    lua_pushboolean(L, 1);
    return 1;
}

int buffer_write_uint32(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    if(buf->produced + 4 > buf->end) return 0;    
    int value = luaL_checkint(L, 2);
    *buf->produced = value >> 24;
    *(buf->produced + 1) = (value >> 16) & 0xFF;
    *(buf->produced + 2) = (value >> 8) & 0xFF;
    *(buf->produced + 3) = value & 0xFF; 
    buf->produced++;
    lua_pushboolean(L, 1);
    return 1;
}

int buffer_write_string(lua_State * L)
{
    lnetlib_buffer * buf = reinterpret_cast<lnetlib_buffer *>(luaL_checkudata(L, 1, BUFFER_MT));
    size_t len;
    const char * str = luaL_checklstring(L, 2, &len);
    if(buf->produced + len > buf->end) return 0;
    strncpy(buf->produced, str, len);
    buf->produced += len;
    lua_pushboolean(L, 1);
    return 1;
}

int create_buffer(lua_State * L)
{
    int size = luaL_checkint(L, 1);
    if(size <= 0) return luaL_argerror(L, 1, "invalid size");
    
    void * bufalloc = lua_newuserdata(L, size + sizeof(lnetlib_buffer));
    luaL_getmetatable(L, BUFFER_MT);
    lua_setmetatable(L, -2);
    
    lnetlib_buffer * rbuf = reinterpret_cast<lnetlib_buffer *>(bufalloc);
    rbuf->start = reinterpret_cast<char *>(rbuf) + sizeof(lnetlib_buffer);
    rbuf->end = rbuf->start + size;
    rbuf->produced = rbuf->start;
    rbuf->consumed = rbuf->start;
    
    //assert(reinterpret_cast<unsigned int>(rbuf) % sizeof(unsigned int) == 0);//check for correct memory alignment
    
    return 1;
}

void create_buffer_metatable(lua_State * L)
{
    luaL_newmetatable(L, BUFFER_MT);
    lua_pushvalue(L, -1);
    lua_setfield(L, -1, "__index");
    
    static luaL_Reg funcs[] = {
        {"reset", buffer_reset},
        {"to_string", buffer_to_string},
        {"size", buffer_size},
        {"read_left", buffer_read_left},
        {"read_uint8", buffer_read_uint8},
        {"read_uint16", buffer_read_uint16},
        {"read_uint32", buffer_read_uint32},
        {"read_string", buffer_read_string},
        {"write_left", buffer_write_left},
        {"write_uint8", buffer_write_uint8},
        {"write_uint16", buffer_write_uint16},
        {"write_uint32", buffer_write_uint32},
        {"write_string", buffer_write_string},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
    lua_pop(L, 1);
}

static int file_stream_gc(lua_State * L)
{
    posix::stream_descriptor * file_stream = reinterpret_cast<posix::stream_descriptor *>(lua_aux_checkobject(L, 1, FILE_STREAM_MT));
    file_stream->~basic_stream_descriptor();
    return 0;
}

void file_stream_async_read_until_handler(int instance, lua_State * L, 
    streambuf * buffer, int function_ref, const boost::system::error_code& ec, 
    std::size_t bytes_transferred)
{
    if(instance != library_instance)
    {
        delete buffer;
        return;
    }
    
    L = L;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, function_ref);
    luaL_unref(L, LUA_REGISTRYINDEX, function_ref);
    
    if(!ec)
    {
        lua_pushlstring(L, boost::asio::buffer_cast<const char *>(*buffer->data().begin()), bytes_transferred);
        buffer->consume(bytes_transferred);
        lua_pushnil(L);
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, ec.message().c_str());
    }
    
    if(lua_pcall(L, 2, 0, 0) != 0)
         event_listeners().log_error("file_stream_async_read_until", lua_tostring(L, -1));
    
    delete buffer;
}

static int file_stream_async_read_until(lua_State * L)
{
    posix::stream_descriptor * file_stream = reinterpret_cast<posix::stream_descriptor *>(lua_aux_checkobject(L, 1, FILE_STREAM_MT));
    
    const char * delim = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    streambuf * buffer = new streambuf;
    
    async_read_until(*file_stream, *buffer, delim, 
        boost::bind(file_stream_async_read_until_handler, library_instance, L, buffer, function_ref, _1, _2));
    
    return 0;
}

static void file_stream_async_read_some_handler(int instance, lua_State * L, int function_ref, char * char_buffer,
    const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if(instance != library_instance)
    {
        delete [] char_buffer;
        return;
    }
    
    L = L;
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, function_ref);
    luaL_unref(L, LUA_REGISTRYINDEX, function_ref);
    
    if(!error)
    {
        lua_pushlstring(L, char_buffer, bytes_transferred);
        lua_pushnil(L);
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
    }
    
    if(lua_pcall(L, 2, 0, 0) != 0)
         event_listeners().log_error("file_stream_async_read_some", lua_tostring(L, -1));
    
    delete [] char_buffer;
}

static int file_stream_async_read_some(lua_State * L)
{
    posix::stream_descriptor * file_stream = reinterpret_cast<posix::stream_descriptor *>(lua_aux_checkobject(L, 1, FILE_STREAM_MT));
    
    int max_length = luaL_checkint(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    int function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    char * char_buffer;
    try
    {
        char_buffer = new char[max_length];
    }
    catch(std::bad_alloc)
    {
        luaL_error(L, "memory allocation error");
    }
    
    file_stream->async_read_some(buffer(char_buffer, max_length), boost::bind(file_stream_async_read_some_handler, library_instance, L, function_ref, char_buffer, _1, _2));
    
    return 0;
}

static int file_stream_cancel(lua_State * L)
{
    posix::stream_descriptor * file_stream = reinterpret_cast<posix::stream_descriptor *>(lua_aux_checkobject(L, 1, FILE_STREAM_MT));
    try
    {
        file_stream->cancel();
    }
    catch(const boost::system::system_error & error)
    {
        return luaL_argerror(L, 2, error.code().message().c_str());
    }
    return 0;
}


static int file_stream_close(lua_State * L)
{
    posix::stream_descriptor * file_stream = reinterpret_cast<posix::stream_descriptor *>(lua_aux_checkobject(L, 1, FILE_STREAM_MT));
    try
    {
        file_stream->close();
    }
    catch(const boost::system::system_error & error)
    {
        return luaL_argerror(L, 2, error.code().message().c_str());
    }
    return 0;
}

void create_file_stream_metatable(lua_State * L)
{
    luaL_newmetatable(L, FILE_STREAM_MT);
    lua_pushvalue(L, -1);
    
    static luaL_Reg funcs[] = {
        {"__gc", file_stream_gc},
        {"async_read_until", file_stream_async_read_until},
        {"async_read_some", file_stream_async_read_some},
        {"cancel", file_stream_cancel},
        {"close", file_stream_close},
        {NULL, NULL}
    };
    
    luaL_register(L, NULL, funcs);
    
    lua_setfield(L, -1, "__index");
    lua_pop(L, 1);
}

static int create_file_stream(lua_State * L)
{
    int fd = luaL_checkint(L, 1);
    
    posix::stream_descriptor * file_stream;
    
    try
    {
        file_stream = new (lua_newuserdata(L, sizeof(posix::stream_descriptor))) posix::stream_descriptor(*main_io, fd);
    }
    catch(const boost::system::system_error & error)
    {
        return luaL_error(L, "%s", error.code().message().c_str());
    }
    
    luaL_getmetatable(L, FILE_STREAM_MT);
    lua_setmetatable(L, -2);
    
    try
    {
        posix::stream_descriptor::non_blocking_io command(true);
        file_stream->io_control(command);
    }
    catch(const boost::system::system_error & error)
    {
        return luaL_error(L, "%s", error.code().message().c_str());
    }
    
    return 1;
}

class netmask_wrapper:public hopmod::ip::address_prefix
{
public:
    static const char * MT;
    
    netmask_wrapper(const hopmod::ip::address_prefix & src):hopmod::ip::address_prefix(src){}
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
        
        lua_pushvalue(L, -1);
        lua_setfield(L, -1, "__index");
        
        static luaL_Reg funcs[] = {
            {"__gc", &netmask_wrapper::__gc},
            {"__tostring", &netmask_wrapper::__tostring},
            {"__eq", &netmask_wrapper::__eq},
            {"to_string", &netmask_wrapper::__tostring},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        lua_pop(L, 1);
    }
    
    static int create_object(lua_State * L)
    {
        hopmod::ip::address_prefix tmp;
        int argc = lua_gettop(L);
        if(argc == 0) luaL_error(L, "missing argument");
        
        if(lua_type(L, 1) == LUA_TSTRING)
        {
            try
            {
                tmp = hopmod::ip::address_prefix::parse(lua_tostring(L, 1));
            }
            catch(std::bad_cast)
            {
                return luaL_error(L, "invalid ip prefix");
            }
        }
        else if(lua_type(L, 1) == LUA_TNUMBER)
        {
            unsigned long prefix = static_cast<unsigned long>(lua_tointeger(L, 1));
            int bits = 32;
            
            if(argc > 1 && lua_isnumber(L, 2)) 
            {
                bits = lua_tointeger(L, 2);
                if(bits < 1 || bits > 32) luaL_argerror(L, 2, "invalid value");
            }
            
            tmp = hopmod::ip::address_prefix(hopmod::ip::address(prefix), bits);
        }
        else
        {
            netmask_wrapper * src = reinterpret_cast<netmask_wrapper *>(luaL_checkudata(L, 1, MT));
            tmp = *src;
        }
        
        new (lua_newuserdata(L, sizeof(netmask_wrapper))) netmask_wrapper(tmp);
        luaL_getmetatable(L, MT);
        lua_setmetatable(L, -2);
        return 1;
    }
    
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<netmask_wrapper *>(luaL_checkudata(L, 1, MT))->~netmask_wrapper();
        return 0;
    }
    
    static int __tostring(lua_State * L)
    {
        netmask_wrapper * obj = reinterpret_cast<netmask_wrapper *>(luaL_checkudata(L, 1, MT));
        std::string ipmaskstring = obj->to_string();
        lua_pushlstring(L, ipmaskstring.c_str(), ipmaskstring.length());
        return 1;
    }
    
    static int __eq(lua_State * L)
    {
        netmask_wrapper * op1 = reinterpret_cast<netmask_wrapper *>(luaL_checkudata(L, 1, MT));
        netmask_wrapper * op2 = reinterpret_cast<netmask_wrapper *>(luaL_checkudata(L, 2, MT));
        lua_pushboolean(L, *op1 == *op2);
        return 1;
    }
};

const char * netmask_wrapper::MT = "netmask";

class netmask_table
{
public:
    static const char * MT;
    
    static void register_class(lua_State * L)
    {
        luaL_newmetatable(L, MT);
    
        static luaL_Reg funcs[] = {
            {"__gc", &netmask_table::__gc},
            {"__index", &netmask_table::__index},
            {"__newindex", &netmask_table::__newindex},
            {NULL, NULL}
        };
        
        luaL_register(L, NULL, funcs);
        lua_pop(L, 1);
    }
    
    static int create_object(lua_State * L)
    {
        new (lua_newuserdata(L, sizeof(netmask_table))) netmask_table;
        luaL_getmetatable(L, MT);
        lua_setmetatable(L, -2);
        return 1;
    }
private:
    static int __gc(lua_State * L)
    {
        reinterpret_cast<netmask_table *>(luaL_checkudata(L, 1, MT))->~netmask_table();
        return 0;
    }
    
    static int __index(lua_State * L)
    {
        netmask_table * table = reinterpret_cast<netmask_table *>(luaL_checkudata(L, 1, MT));
        
        hopmod::ip::address_prefix ipmask;
        
        if(lua_type(L, 2) == LUA_TSTRING)
        {
            try
            {
                ipmask = hopmod::ip::address_prefix::parse(lua_tostring(L, 2));
            }
            catch(std::bad_cast)
            {
                luaL_argerror(L, 2, "invalid ip prefix");
            }
        }
        else
        {
            ipmask = *(reinterpret_cast<netmask_wrapper *>(luaL_checkudata(L, 2, netmask_wrapper::MT)));
        }
        
        lua_newtable(L);        
        int i = 1;
        
        hopmod::ip::address_prefix_tree<int, -1> * child = &table->m_masks;
        while((child = child->next_match(&ipmask)))
        {
            lua_pushinteger(L, i++);
            lua_rawgeti(L, LUA_REGISTRYINDEX, child->value());
            lua_settable(L, -3);
        }
        
        return 1;
    }
    
    static int __newindex(lua_State * L)
    {
        netmask_table * table = reinterpret_cast<netmask_table *>(luaL_checkudata(L, 1, MT));

        hopmod::ip::address_prefix ipmask;
        if(lua_type(L, 2) == LUA_TSTRING)
        {
            try
            {
                ipmask = hopmod::ip::address_prefix::parse(lua_tostring(L, 2));
            }
            catch(std::bad_cast)
            {
                luaL_argerror(L, 2, "invalid ip prefix");
            }
        }
        else if(lua_type(L, 1) == LUA_TNUMBER)
        {
            unsigned long prefix = static_cast<unsigned long>(lua_tointeger(L, 2));
            ipmask = hopmod::ip::address_prefix(hopmod::ip::address(prefix), 32);
        }
        else
        {
            ipmask = *(reinterpret_cast<netmask_wrapper *>(luaL_checkudata(L, 2, netmask_wrapper::MT)));
        }
        
        if(lua_type(L, 3) != LUA_TNIL)
        {
            lua_pushvalue(L, 3);
            table->m_masks.insert(ipmask, luaL_ref(L, LUA_REGISTRYINDEX));
        }
        else table->m_masks.erase(ipmask);
        
        return 0;
    }
    
    hopmod::ip::address_prefix_tree<int, -1> m_masks;
};

const char * netmask_table::MT = "netmask_table";

static int shutdown_net(lua_State *)
{
    library_instance++;
    return 0;
}

namespace lua{
namespace module{

void open_net(lua_State * L)
{
    lua::on_shutdown(L, shutdown_net);
    
    main_io = &get_main_io_service();
    
    create_acceptor_metatable(L);
    create_basic_socket_metatable(L);
    create_client_socket_metatable(L);
    create_buffer_metatable(L);
    create_file_stream_metatable(L);
    
    netmask_wrapper::register_class(L);
    netmask_table::register_class(L);
    
    static luaL_Reg net_funcs[] = {
        {"async_resolve", async_resolve},
        {"tcp_acceptor", create_tcp_acceptor},
        {"tcp_client", create_tcp_client},
        {"buffer", create_buffer},
        {"ipmask", netmask_wrapper::create_object},
        {"ipmask_table", netmask_table::create_object},
        {"file_stream", create_file_stream},
        {NULL, NULL}
    };
    
    luaL_register(L, "net", net_funcs);
}

} //namespace module
} //namespace lua
