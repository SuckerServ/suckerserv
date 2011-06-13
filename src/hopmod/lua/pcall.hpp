#ifndef HOPMOD_LUA_PCALL_HPP
#define HOPMOD_LUA_PCALL_HPP

namespace lua{

int pcall(lua_State * L, int nargs, int nresults, int error_function);

} //namespace lua

#endif

