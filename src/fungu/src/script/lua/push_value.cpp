/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace lua{

void push_value(lua_State * L, int value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void push_value(lua_State * L, unsigned int value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void push_value(lua_State * L, long value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void push_value(lua_State * L, unsigned long value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void push_value(lua_State * L, short value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void push_value(lua_State * L, unsigned short value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void push_value(lua_State * L, bool value)
{
    lua_pushboolean(L, static_cast<int>(value));
}

void push_value(lua_State * L, double value)
{
    lua_pushnumber(L, static_cast<lua_Number>(value));
}

void push_value(lua_State * L, float value)
{
    lua_pushnumber(L, static_cast<lua_Number>(value));
}

void push_value(lua_State * L, char value)
{
    lua_pushlstring(L, &value, 1);
}

void push_value(lua_State * L, unsigned char value)
{
    lua_pushinteger(L, static_cast<lua_Integer>(value));
}

void push_value(lua_State * L, const std::string & value)
{
    lua_pushstring(L, value.c_str());
}

void push_value(lua_State * L, const const_string & value)
{
    lua_pushlstring(L, value.begin(), value.length());
}

void push_value(lua_State * L, const char * value)
{
    lua_pushstring(L, value);
}    

} //namespace lua
} //namespace script
} //namespace fungu
