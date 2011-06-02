/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LUA_OBJECT_WRAPPER_HPP
#define FUNGU_SCRIPT_LUA_OBJECT_WRAPPER_HPP

struct lua_State;

namespace fungu{
namespace script{

class env_object;

namespace lua{

void push_object(lua_State * L, env_object *obj);

void register_object(lua_State * L, int index, env_object * obj, const char * name);
void register_object(lua_State * L, env_object * obj, const char * name);

env_object * get_object(lua_State *, int index, const char * name);
env_object * get_object(lua_State *, int index);

} //namespace lua
} //namespace script
} //namespace fungu

#endif
