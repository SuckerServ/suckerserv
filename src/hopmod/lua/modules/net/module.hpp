#ifndef HOPMOD_LUA_NET_MODULE_HPP
#define HOPMOD_LUA_NET_MODULE_HPP

#include <lua.hpp>
#include <boost/asio.hpp>

void log_error(lua_State * L, const char *);
boost::asio::io_service & get_main_io_service(lua_State * L);

#endif

