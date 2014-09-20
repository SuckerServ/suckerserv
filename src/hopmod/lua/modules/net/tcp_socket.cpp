#include "tcp_socket.hpp"
#include "weak_ref.hpp"
#include "module.hpp"
#include "../../register_class.hpp"
#include "../../to.hpp"
#include "../../create_object.hpp"
#include "../../pcall.hpp"
#include "../../error_handler.hpp"
#include "utils/protect_wrapper.hpp"

using namespace asio;
#include <cstring>

#include <iostream>

namespace lua{

static int push_error_code(lua_State * L, std::error_code & ec)
{
    if(ec)
    {
        lua_pushboolean(L, false);
        lua_pushstring(L, ec.message().c_str());
        return 2;
    }
    else
    {
        lua_pushboolean(L, true);
        return 1;
    }
}

template<typename EndpointType>
int push_endpoint(lua_State * L, const EndpointType & endpoint)
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
    
    return 1;
}

tcp_socket::tcp_socket(io_service & service)
 :socket(service), read_buffer(read_buffer_buffer)
{
    
}

const char * managed_tcp_socket::CLASS_NAME = "managed_tcp_socket";

int managed_tcp_socket::register_class(lua_State * L){
    static luaL_Reg member_functions[] = {
        {"__gc", &managed_tcp_socket::__gc},
        {"open", &managed_tcp_socket::open},
        {"close", &managed_tcp_socket::close},
        {"cancel", &managed_tcp_socket::cancel},
        {"shutdown", &managed_tcp_socket::shutdown},
        {"shutdown_send", &managed_tcp_socket::shutdown_send},
        {"shutdown_receive", &managed_tcp_socket::shutdown_receive},
        {"local_endpoint", &managed_tcp_socket::local_endpoint},
        {"remote_endpoint", &managed_tcp_socket::remote_endpoint},
        {"get_option", &managed_tcp_socket::get_option},
        {"set_option", &managed_tcp_socket::set_option},
        {"async_read", &managed_tcp_socket::async_read},
        {"async_read_until", &managed_tcp_socket::async_read_until},
        {"async_send", &managed_tcp_socket::async_send},
        {"async_connect", &managed_tcp_socket::async_connect},
        {"bind", &managed_tcp_socket::bind},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    return 0;
}

int managed_tcp_socket::create_object(lua_State * L)
{   
    managed_tcp_socket::target_type self = *lua::create_object<managed_tcp_socket>(L, 
        std::shared_ptr<tcp_socket>(new tcp_socket(get_main_io_service(L))));
    return 1;
}

int managed_tcp_socket::__gc(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code ec;
    self->socket.close(ec);
    self.~target_type();
    return 0;
}

int managed_tcp_socket::open(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code ec;
    self->socket.open(ip::tcp::v4(), ec);
    if(ec) luaL_error(L, "%s", ec.message().c_str());
    return 0;
}

int managed_tcp_socket::close(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code ec;
    self->socket.close(ec);
    return push_error_code(L, ec);
}

int managed_tcp_socket::cancel(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code ec;
    self->socket.cancel(ec);
    return push_error_code(L, ec);
}

int managed_tcp_socket::shutdown(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code ec;
    self->socket.shutdown(ip::tcp::socket::shutdown_both, ec);
    return push_error_code(L, ec);
}

int managed_tcp_socket::shutdown_send(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code ec;
    self->socket.shutdown(ip::tcp::socket::shutdown_send, ec);
    return push_error_code(L, ec);
}

int managed_tcp_socket::shutdown_receive(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code ec;
    self->socket.shutdown(ip::tcp::socket::shutdown_receive, ec);
    return push_error_code(L, ec);
}

int managed_tcp_socket::local_endpoint(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code error;
    asio::ip::tcp::endpoint endpoint = self->socket.local_endpoint(error);
    if(error) return push_error_code(L, error);
    else return push_endpoint(L, endpoint);
}

int managed_tcp_socket::remote_endpoint(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    std::error_code error;
    asio::ip::tcp::endpoint endpoint = self->socket.remote_endpoint(error);
    if(error) return push_error_code(L, error);
    else return push_endpoint(L, endpoint);
}

int managed_tcp_socket::get_option(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    const char * name = luaL_checkstring(L, 2);
    
    std::error_code ec;
    
    if(!strcmp(name, "keep_alive"))
    {
        asio::socket_base::keep_alive option;
        self->socket.get_option(option, ec);
        if(ec) return push_error_code(L, ec);
        lua_pushboolean(L, option.value());
        return 1;
    }
    else if(!strcmp(name, "linger"))
    {
        asio::socket_base::linger option;
        self->socket.get_option(option, ec);
        if(ec) return push_error_code(L, ec);
        lua_pushboolean(L, option.enabled());
        lua_pushinteger(L, option.timeout());
        return 2;
    }
    
    return 0;

}

int managed_tcp_socket::set_option(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    const char * name = luaL_checkstring(L, 2);
    
    std::error_code ec;
    
    if(!strcmp(name, "keep_alive"))
    {
        asio::socket_base::keep_alive option(luaL_checkint(L, 3));
        self->socket.set_option(option, ec);
        if(ec) return push_error_code(L, ec);
    }
    else if(!strcmp(name, "linger"))
    {
        asio::socket_base::linger option(luaL_checkint(L, 3), luaL_checkint(L, 4));
        self->socket.set_option(option, ec);
        if(ec) return push_error_code(L, ec);
    }
    
    return 0;
}

static void async_read_handler(
    managed_tcp_socket::target_type managed_socket,
    lua_State * L, lua::weak_ref callback, const char * event_name,
    const std::error_code & ec, const char * buffer, std::size_t length)
{
    if(callback.is_expired())
    {
        managed_socket->read_buffer.unlock_read();
        return;
    }
    
    lua::get_error_handler(L);
    int error_function = lua_gettop(L);
    
    callback.get(L);
    
    int args;
    
    if(ec)
    {
        lua_pushnil(L);
        lua_pushstring(L, ec.message().c_str());
        args = 2;
    }
    else
    {
        luaL_Buffer copy_buffer;
        luaL_buffinit(L, &copy_buffer);
        luaL_addlstring(&copy_buffer, buffer, length);
        luaL_pushresult(&copy_buffer);
        args = 1;
    }
    
    managed_socket->read_buffer.unlock_read();
    
    if(lua::pcall(L, args, 0, error_function) != 0) 
        log_error(L, event_name);
    
    lua_pop(L, 1);
    
    callback.unref(L);
}

int managed_tcp_socket::async_read(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    
    int read_size = luaL_checkint(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    
    self->read_buffer.async_read(self->socket, read_size, 
        protect(std::bind(&async_read_handler, self, L, lua::weak_ref::create(L), 
        "async_read", std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    
    return 0;
}

int managed_tcp_socket::async_read_until(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    
    const char * read_until = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    
    self->read_buffer.async_read_until(self->socket, read_until, 
        protect(std::bind(&async_read_handler, self, L, lua::weak_ref::create(L), 
        "async_read_until", std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));
    
    return 0;
}

static void async_send_handler(managed_tcp_socket::target_type, lua_State * L, char * string_copy, 
    lua::weak_ref callback, const std::error_code & ec, std::size_t bytes_transferred)
{
    delete [] string_copy;
    
    if(callback.is_expired()) return;
    
    lua::get_error_handler(L);
    int error_function = lua_gettop(L);
    
    callback.get(L);
    
    int args = 0;
    
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        args = 1;
    }
    
    if(lua::pcall(L, args, 0, error_function) != 0) 
        log_error(L, "async_send");
        
    lua_pop(L, 1);
    
    callback.unref(L);
}

int managed_tcp_socket::async_send(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    
    size_t string_len;
    const char * string = luaL_checklstring(L, 2, &string_len);
    
    char * string_copy = new char[string_len + 1];
    memcpy(string_copy, string, string_len + 1);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    lua::weak_ref callback_ref = lua::weak_ref::create(L);
    
    self->socket.async_send(asio::buffer(string_copy, string_len), protect(std::bind(
        async_send_handler, self, L, string_copy, callback_ref, std::placeholders::_1, std::placeholders::_2)));
    
    return 0;
}

static void async_connect_handler(managed_tcp_socket::target_type managed_socket,
    lua_State * L, lua::weak_ref callback, const std::error_code& ec)
{
    if(callback.is_expired()) return;
    
    lua::get_error_handler(L);
    int error_function = lua_gettop(L);
    
    callback.get(L);
    
    int args = 0;
    
    if(ec)
    {
        lua_pushstring(L, ec.message().c_str());
        args = 1;
    }
    
    if(lua::pcall(L, args, 0, error_function) != 0)
        log_error(L, "async_connect");
    
    lua_pop(L, 1);
    
    callback.unref(L);
}

static void async_connect_resolve_handler(std::shared_ptr<ip::tcp::resolver> resolver, 
    managed_tcp_socket::target_type managed_socket,
    lua_State * L, lua::weak_ref callback, 
    const std::error_code& ec, asio::ip::tcp::resolver::iterator iterator)
{
    if(callback.is_expired()) return;
    
    if(ec)
    {
        lua::get_error_handler(L);
        int error_function = lua_gettop(L);
        
        callback.get(L);
        lua_pushstring(L, ec.message().c_str());
        
        if(lua::pcall(L, 1, 0, error_function) != 0)
            log_error(L, "async_connect");
        
        lua_pop(L, 1);
        callback.unref(L);
        return;
    }
    
    managed_socket->socket.async_connect(iterator->endpoint(), std::bind(async_connect_handler,
        managed_socket, L, callback, std::placeholders::_1));
}

int managed_tcp_socket::async_connect(lua_State * L)
{
    target_type self = *lua::to<managed_tcp_socket>(L, 1);
    
    const char * hostname = luaL_checkstring(L, 2);
    const char * port = luaL_checkstring(L, 3);

    luaL_checktype(L, 4, LUA_TFUNCTION);
    lua_pushvalue(L, 4);
    lua::weak_ref callback_ref = lua::weak_ref::create(L);

    if(!self->socket.is_open())
    {
        std::error_code ec;
        self->socket.open(ip::tcp::v4(), ec);
        if(ec)
        {
            luaL_error(L, "%s", ec.message().c_str());
            return 0;
        }
    }
    
    ip::tcp::resolver::query query(hostname, port);
    std::shared_ptr<ip::tcp::resolver> resolver(new ip::tcp::resolver(get_main_io_service(L)));
    
    resolver->async_resolve(query, std::bind(async_connect_resolve_handler, resolver, self, L, 
        callback_ref, std::placeholders::_1, std::placeholders::_2));
    
    return 0;
}

int managed_tcp_socket::bind(lua_State * L)
{
    managed_tcp_socket::target_type self = *lua::to<managed_tcp_socket>(L, 1);
    
    if(!self->socket.is_open())
    {
        std::error_code ec;
        self->socket.open(ip::tcp::v4(), ec);
        if(ec)
        {
            luaL_error(L, "%s", ec.message().c_str());
            return 0;
        }
    }
    
    const char * ip = luaL_checkstring(L, 2);
    int port = luaL_checkinteger(L, 3);
    
    std::error_code ec;
    
    ip::address_v4 host = ip::address_v4::from_string(ip, ec);
    if(ec)
    {
        lua_pushboolean(L, 1);
        lua_pushstring(L, ec.message().c_str());
        return 2;
    }
    
    self->socket.bind(ip::tcp::endpoint(host, port), ec);
    if(ec)
    {
        lua_pushboolean(L, 1);
        lua_pushstring(L, ec.message().c_str());
        return 2;
    }
    
    return 0;
}

} // namespace lua

