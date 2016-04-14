#ifndef HOPMOD_LUA_REGISTER_CLASS_HPP
#define HOPMOD_LUA_REGISTER_CLASS_HPP

#include <lua.hpp>

namespace lua{

void register_class(lua_State *, const char *, const luaL_Reg *);

} //namespace lua

#endif

