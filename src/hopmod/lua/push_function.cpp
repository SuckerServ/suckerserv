#include "push_function.hpp"

namespace lua{

void push(lua_State  * L, bool value)
{
    lua_pushboolean(L, static_cast<int>(value));
}

void push(lua_State  * L, int value)
{
    lua_pushinteger(L, value);
}

void push(lua_State * L, long value)
{
    lua_pushnumber(L, static_cast<lua_Number>(value));
}

void push(lua_State * L, unsigned long value)
{
    lua_pushnumber(L, static_cast<lua_Number>(value));
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

int to(lua_State * L, int index, return_tag<int>)
{
    return luaL_checkint(L, index);
}

unsigned int to(lua_State * L, int index, return_tag<unsigned int>)
{
    return static_cast<unsigned int>(luaL_checknumber(L, index));
}

long to(lua_State * L, int index, return_tag<long>)
{
    return static_cast<long>(luaL_checklong(L, index));
}

unsigned long to(lua_State * L, int index, return_tag<unsigned long>)
{
    return static_cast<unsigned long>(luaL_checknumber(L, index));
}

lua_Number to(lua_State * L, int index, return_tag<lua_Number>)
{
    return luaL_checknumber(L, index);
}

bool to(lua_State * L, int index, return_tag<bool>)
{
    luaL_checkany(L, index);
    
    if(lua_type(L, index) == LUA_TNUMBER && lua_tointeger(L, index) == 0)
        return false;
    
    return static_cast<bool>(lua_toboolean(L, index));
}

const char * to(lua_State * L, int index, return_tag<const char *>)
{
    return luaL_checkstring(L, index);
}

std::string to(lua_State * L, int index, return_tag<std::string>)
{
    const char * c_str = luaL_checkstring(L, index);
    return std::string(c_str);
}

} //namespace lua

