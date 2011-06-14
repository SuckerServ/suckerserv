#include "ssl_tcp_stream.hpp"
#include "weak_ref.hpp"
#include "module.hpp"
#include "../../register_class.hpp"
#include "../../to.hpp"
#include "../../create_object.hpp"
#include "../../pcall.hpp"
#include "../../error_handler.hpp"
using namespace boost::asio;

namespace lua{

ssl_tcp_stream::ssl_tcp_stream(boost::shared_ptr<tcp_socket> tcp_socket, 
    boost::shared_ptr<boost::asio::ssl::context> ssl_context_arg)
 :socket(tcp_socket), ssl_context(ssl_context_arg), stream(tcp_socket->socket, *ssl_context_arg)
{
    
}

const char * managed_ssl_tcp_stream::CLASS_NAME = "ssl_tcp_stream ";

int managed_ssl_tcp_stream::register_class(lua_State * L){

    static luaL_Reg member_functions[] = {
        {"__gc", &managed_ssl_tcp_stream ::__gc},
        {"async_handshake", &managed_ssl_tcp_stream ::async_handshake},
        {"async_read", &managed_ssl_tcp_stream ::async_read},
        {"async_read_until", &managed_ssl_tcp_stream::async_read_until},
        {"async_send", &managed_ssl_tcp_stream::async_send},
        {"async_shutdown", &managed_ssl_tcp_stream ::async_shutdown},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    
    return 0;
}

int managed_ssl_tcp_stream::create_object(lua_State * L)
{
    managed_tcp_socket::target_type socket = *lua::to<managed_tcp_socket>(L, 1);
    ssl_context::target_type ctx = *lua::to<ssl_context>(L, 2);
    
    lua::create_object<managed_ssl_tcp_stream>(L, 
        boost::shared_ptr<ssl_tcp_stream>(new ssl_tcp_stream(socket, ctx)));
    
    return 1;
}

int managed_ssl_tcp_stream::push_constants(lua_State * L)
{
    lua_pushinteger(L, ssl::stream<boost::asio::ip::tcp::socket &>::client);
    lua_setfield(L, -2, "HANDSHAKE_CLIENT");
    
    lua_pushinteger(L, ssl::stream<boost::asio::ip::tcp::socket &>::server);
    lua_setfield(L, -2, "HANDSHAKE_SERVER");
    return 0;
}

int managed_ssl_tcp_stream::__gc(lua_State * L)
{
    lua::to<managed_ssl_tcp_stream>(L, 1)->~target_type();
    return 0;
}

static void async_handshake_handler(
    managed_ssl_tcp_stream::target_type managed_socket, lua_State * L, lua::weak_ref callback, 
    const boost::system::error_code & ec)
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
        log_error(L, "async_handshake");
    
    lua_pop(L, 1);
    
    callback.unref(L);
}

int managed_ssl_tcp_stream::async_handshake(lua_State * L)
{
    target_type self = *lua::to<managed_ssl_tcp_stream>(L, 1);
    
    int handshake = luaL_checkint(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    
    self->stream.async_handshake(
        static_cast<ssl::stream<boost::asio::ip::tcp::socket &>::handshake_type>(handshake), 
        boost::bind(async_handshake_handler, self, L, lua::weak_ref::create(L), _1));
    
    return 0;
}

static void async_read_handler(
    managed_ssl_tcp_stream::target_type managed_socket,
    lua_State * L, lua::weak_ref callback, const char * event_name,
    const boost::system::error_code & ec, const char * buffer, std::size_t length)
{
    if(callback.is_expired())
    {
        managed_socket->socket->read_buffer.unlock_read();
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
    
    managed_socket->socket->read_buffer.unlock_read();
    
    if(lua::pcall(L, args, 0, error_function) != 0) 
        log_error(L, event_name);
        
    lua_pop(L, 1);
    
    callback.unref(L);
}

int managed_ssl_tcp_stream::async_read(lua_State * L)
{
    target_type self = *lua::to<managed_ssl_tcp_stream>(L, 1);
    
    int read_size = luaL_checkint(L, 2);
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    
    self->socket->read_buffer.async_read(self->stream, read_size, 
        boost::protect(boost::bind(&async_read_handler, self, L, lua::weak_ref::create(L), 
        "async_read", _1, _2, _3)));
    
    return 0;
}

int managed_ssl_tcp_stream::async_read_until(lua_State * L)
{
    target_type self = *lua::to<managed_ssl_tcp_stream>(L, 1);
    
    const char * read_until = luaL_checkstring(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    
    self->socket->read_buffer.async_read_until(self->stream, read_until, 
        boost::protect(boost::bind(&async_read_handler, self, L, lua::weak_ref::create(L), 
        "async_read_until", _1, _2, _3)));
    
    return 0;
}

static void async_send_handler(managed_ssl_tcp_stream::target_type, lua_State * L, char * string_copy, 
    lua::weak_ref callback, const boost::system::error_code & ec, std::size_t bytes_transferred)
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

int managed_ssl_tcp_stream::async_send(lua_State * L)
{
    target_type self = *lua::to<managed_ssl_tcp_stream>(L, 1);
    
    size_t string_len;
    const char * string = luaL_checklstring(L, 2, &string_len);
    
    char * string_copy = new char[string_len + 1];
    memcpy(string_copy, string, string_len + 1);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    lua::weak_ref callback_ref = lua::weak_ref::create(L);
    
    self->stream.async_write_some(boost::asio::buffer(string_copy, string_len),
        boost::protect(boost::bind(async_send_handler, self, L, string_copy, callback_ref, _1, _2)));
    
    return 0;
}

static void async_shutdown_handler(managed_ssl_tcp_stream::target_type managed_socket, 
    lua_State * L, lua::weak_ref callback, const boost::system::error_code & ec)
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
        log_error(L, "async_shutdown");
    
    lua_pop(L, 1);
    
    callback.unref(L);
}

int managed_ssl_tcp_stream::async_shutdown(lua_State * L)
{
    target_type self = *lua::to<managed_ssl_tcp_stream>(L, 1);
    
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    
    self->stream.async_shutdown(boost::bind(async_shutdown_handler, 
        self, L, lua::weak_ref::create(L), _1));
    
    return 0;
}

} //namespace lua

