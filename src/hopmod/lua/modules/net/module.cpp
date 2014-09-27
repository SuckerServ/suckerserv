#include <luajit-2.0/lua.hpp>
#include "../../event.hpp"
#include "tcp_socket.hpp"
#include "tcp_acceptor.hpp"
#include "ipmask.hpp"
#include "file_stream.hpp"
#include "resolver.hpp"

#ifndef DISABLE_SSL
#include "ssl_context.hpp"
#include "ssl_tcp_stream.hpp"
#endif

asio::io_service & get_main_io_service();
lua::event_environment & event_listeners();

void log_error(lua_State * L, const char * event_name)
{
    event_listeners().log_error(event_name, lua_tostring(L, -1));
    lua_pop(L, 1);
}

asio::io_service & get_main_io_service(lua_State *)
{
    return get_main_io_service();
}

namespace lua{
namespace module{

void open_net2(lua_State * L)
{
    lua::managed_tcp_socket::register_class(L);
    lua::tcp_acceptor::register_class(L);
    lua::ipmask::register_class(L);
    lua::ipmask_table::register_class(L);
    lua::file_stream::register_class(L);
    
    static luaL_Reg net_funcs[] = {
        {"async_resolve", lua::async_resolve},
        {"tcp_client", lua::managed_tcp_socket::create_object},
        {"tcp_acceptor", lua::tcp_acceptor::create_object},
        {"ipmask", lua::ipmask::create_object},
        {"ipmask_table", lua::ipmask_table::create_object},
        {"file_stream", lua::file_stream::create_object},
        {NULL, NULL}
    };
    
    luaL_register(L, "net", net_funcs);
    
#ifndef DISABLE_SSL
    
    lua::ssl_context::register_class(L);
    lua::managed_ssl_tcp_stream::register_class(L);
    
    lua_newtable(L);
    
    static luaL_Reg ssl_funcs[] = {
        {"context", lua::ssl_context::create_object},
        {"tcp_stream", lua::managed_ssl_tcp_stream::create_object},
        {NULL, NULL}
    };
    luaL_register(L, NULL, ssl_funcs);
    
    lua::ssl_context::push_constants(L);
    lua::managed_ssl_tcp_stream::push_constants(L);
    
    lua_setfield(L, -2, "ssl");
#endif
    
    lua_pop(L, 1);
}

} //namespace module
} //namespace lua

