#include "lua/register_class.hpp"

namespace lua{

void register_class(lua_State * L, const char * class_name, const luaL_Reg * member_functions)
{
    luaL_newmetatable(L, class_name);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, NULL, member_functions);
    lua_pop(L, 1);
}

} //namespace lua

