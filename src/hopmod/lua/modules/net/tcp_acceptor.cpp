#include "tcp_acceptor.hpp"
#include "tcp_socket.hpp"
#include "weak_ref.hpp"
#include "module.hpp"
#include "../../register_class.hpp"
#include "../../to.hpp"
#include "../../create_object.hpp"
#include "../../pcall.hpp"
#include "../../error_handler.hpp"
#include <boost/asio.hpp>
using namespace boost::asio;
#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>

namespace lua{

const char * tcp_acceptor::CLASS_NAME = "tcp_acceptor";

int tcp_acceptor::register_class(lua_State * L){
    static luaL_Reg member_functions[] = {
        {"__gc", &tcp_acceptor::__gc},
        {"listen", &tcp_acceptor::listen},
        {"close", &tcp_acceptor::close},
        {"async_accept", &tcp_acceptor::async_accept},
        {"set_option", &tcp_acceptor::set_option},
        {"get_option", &tcp_acceptor::get_option},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    return 0;
}

int tcp_acceptor::create_object(lua_State * L)
{
    const char * ip = luaL_checkstring(L, 1);
    int port = luaL_checkint(L, 2);
    
    ip::tcp::endpoint endpoint(ip::address_v4::from_string(ip), port);
    
    boost::shared_ptr<ip::tcp::acceptor> acceptor(new ip::tcp::acceptor(get_main_io_service(L)));
    lua::create_object<tcp_acceptor>(L, acceptor);
    
    acceptor->open(endpoint.protocol());
    acceptor->set_option(ip::tcp::acceptor::reuse_address(true));
    
    boost::system::error_code error;
    acceptor->bind(endpoint, error);
    if(error)
    {
        lua_pushnil(L);
        lua_pushstring(L, error.message().c_str());
        return 2;
    }
    
    return 1;
}

int tcp_acceptor::__gc(lua_State * L)
{
    tcp_acceptor::target_type self = *lua::to<tcp_acceptor>(L, 1);
    boost::system::error_code ec;
    self->close(ec);
    self.~target_type();
    return 0;
}

int tcp_acceptor::listen(lua_State * L)
{
    target_type self = *lua::to<tcp_acceptor>(L, 1);
    boost::system::error_code ec;
    self->listen(socket_base::max_connections, ec);
    if(ec) return luaL_error(L, ec.message().c_str());
    return 0;
}

int tcp_acceptor::close(lua_State * L)
{
    target_type self = *lua::to<tcp_acceptor>(L, 1);
    boost::system::error_code ec;
    self->close(ec);
    if(ec) return luaL_error(L, ec.message().c_str());
    return 0;
}

static void async_accept_handler(lua_State * L, lua::weak_ref socket, lua::weak_ref callback, 
    const boost::system::error_code & ec)
{
    if(callback.is_expired()) return;
    
    lua::get_error_handler(L);
    int error_function = lua_gettop(L);
    
    callback.get(L);
    socket.get(L);
    
    int args = 1;
    
    if(ec)
    {
        lua_pushnil(L);
        lua_replace(L, -2);
        lua_pushstring(L, ec.message().c_str());
        args = 2;
    }
    
    if(lua::pcall(L, args, 0, error_function) != 0)
        log_error(L, "async_accept");
    
    lua_pop(L, 1);
    
    socket.unref(L); 
    callback.unref(L);
}

int tcp_acceptor::async_accept(lua_State * L)
{
    target_type self = *lua::to<tcp_acceptor>(L, 1);
    
    managed_tcp_socket::create_object(L);
    managed_tcp_socket::target_type managed_socket = *lua::to<managed_tcp_socket>(L, lua_gettop(L));
    lua::weak_ref socket_ref = lua::weak_ref::create(L);
    
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    lua::weak_ref callback_ref = lua::weak_ref::create(L);
    
    self->async_accept(managed_socket->socket, boost::bind(async_accept_handler, 
        L, socket_ref, callback_ref, _1));
    
    return 0;
}

int tcp_acceptor::set_option(lua_State * L)
{
    target_type self = *lua::to<tcp_acceptor>(L, 1);
    
    const char * name = luaL_checkstring(L, 2);
    
    boost::system::error_code ec;
    
    if(!strcmp(name, "enable_connection_aborted"))
    {
        boost::asio::socket_base::enable_connection_aborted option(luaL_checkint(L, 3));
        self->set_option(option, ec);
    }
    else if(!strcmp(name, "reuse_address"))
    {
        boost::asio::socket_base::reuse_address option(luaL_checkint(L, 3));
        self->set_option(option, ec);
    }
    
    if(ec) return luaL_error(L, ec.message().c_str());
    
    return 0;
}

int tcp_acceptor::get_option(lua_State * L)
{
    target_type self = *lua::to<tcp_acceptor>(L, 1);
    
    const char * name = luaL_checkstring(L, 2);
    
    boost::system::error_code ec;
    
    if(!strcmp(name, "enable_connection_aborted"))
    {
        boost::asio::socket_base::enable_connection_aborted option;
        self->get_option(option, ec);
        if(ec) return luaL_error(L, ec.message().c_str());
        lua_pushboolean(L, option.value());
        return 1;
    }
    else if(!strcmp(name, "reuse_address"))
    {
        boost::asio::socket_base::reuse_address option;
        self->get_option(option, ec);
        if(ec) return luaL_error(L, ec.message().c_str());
        lua_pushboolean(L, option.value());
        return 1;
    }
    
    return 0;
}

} //namespace lua

