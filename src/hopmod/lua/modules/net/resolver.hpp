#ifndef HOPMOD_LUA_NET_RESOLVER_HPP
#define HOPMOD_LUA_NET_RESOLVER_HPP

#include <luajit-2.0/lua.hpp>

namespace lua{

int async_resolve(lua_State  * L);

} //namespace lua

#endif

