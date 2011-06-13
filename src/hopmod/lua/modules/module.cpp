#include <lua.hpp>
#include "module.hpp"

namespace lua{

void on_shutdown(lua_State * L, lua_CFunction callback)
{
    lua_newuserdata(L, 1);
    lua_newtable(L);
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, callback);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
    luaL_ref(L, LUA_REGISTRYINDEX);
}

} //namespace lua

