#include <lua.hpp>
#include "lua/pcall.hpp"

namespace lua{

int pcall(lua_State * L, int nargs, int nresults, int error_function)
{
    int error_code = lua_pcall(L, nargs, nresults, error_function);
    if(error_code != 0)
    {
        if(lua_type(L, -1) == LUA_TTABLE)
        {
            lua_pushinteger(L, 1);
            lua_gettable(L, -2);
            lua_replace(L, -2);
        }
    }
    return error_code;
}

} //namespace lua

