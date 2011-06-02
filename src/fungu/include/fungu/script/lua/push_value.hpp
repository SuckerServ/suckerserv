/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LUA_PUSH_VALUE_HPP
#define FUNGU_SCRIPT_LUA_PUSH_VALUE_HPP

extern "C"{
#include <lua.h>
}

namespace fungu{
namespace script{

template<typename Target,typename Source>
Target lexical_cast(const Source &);  //Forward declaration

namespace lua{

template<typename T>
void push_value(lua_State * L, const T & value)
{
    const_string value_str = fungu::script::lexical_cast<const_string, T>(value);
    lua_pushlstring(L, value_str.begin(), value_str.length());
}

void push_value(lua_State * L, int value);
void push_value(lua_State * L, unsigned int value);
void push_value(lua_State * L, long value);
void push_value(lua_State * L, unsigned long value);
void push_value(lua_State * L, short value);
void push_value(lua_State * L, unsigned short value);
void push_value(lua_State * L, bool value);
void push_value(lua_State * L, double value);
void push_value(lua_State * L, float value);
void push_value(lua_State * L, char value);
void push_value(lua_State * L, unsigned char value);

void push_value(lua_State * L, const std::string & value);
void push_value(lua_State * L, const const_string & value);
void push_value(lua_State * L, const char * value);

} //namespace lua
} //namespace script
} //namespace fungu

#endif
