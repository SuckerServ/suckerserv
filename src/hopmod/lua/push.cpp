#include "lua/push.hpp"

namespace lua{

nil_type nil;

void push(lua_State  * L, nil_type)
{
    lua_pushnil(L);
}

void push(lua_State  * L, bool value)
{
    lua_pushboolean(L, static_cast<int>(value));
}

void push(lua_State  * L, int value)
{
    lua_pushinteger(L, value);
}

void push(lua_State  * L, lua_Number value)
{
    lua_pushnumber(L, value);
}

void push(lua_State  * L, lua_CFunction value)
{
    lua_pushcfunction(L, value);
}

void push(lua_State  * L, const char * value)
{
    lua_pushstring(L, value);
}

void push(lua_State  * L, const char * value, std::size_t length)
{
    lua_pushlstring(L, value, length);
}

void push(lua_State  * L, const std::string & value)
{
    lua_pushlstring(L, value.data(), value.length());
}

} //namespace lua

