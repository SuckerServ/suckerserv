/*   
 *   The Fungu Scripting Engine Library
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include <iostream>

namespace fungu{
namespace script{
namespace lua{

namespace detail{

int object_wrapper(lua_State * L)
{
    env_object ** obj = reinterpret_cast<env_object **>(lua_touserdata(L, lua_upvalueindex(1)));
    return (*obj)->call(L);
}

static int __gc(lua_State * L)
{
    luaL_checktype(L, 1, LUA_TUSERDATA);
    env_object ** hold_ptr = reinterpret_cast<env_object **>(lua_touserdata(L, 1));
    intrusive_ptr_release(*hold_ptr);
}

} //namespace detail

void push_object(lua_State * L, env_object * obj)
{
    env_object ** hold_ptr = reinterpret_cast<env_object **>(lua_newuserdata(L, sizeof(env_object *)));
    *hold_ptr = obj;
    obj->add_ref();
    
    lua_newtable(L);
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, detail::__gc);
    lua_settable(L, -3);
    
    lua_setmetatable(L, -2);
    
    lua_pushcclosure(L, &detail::object_wrapper, 1);
}

void register_object(lua_State * L, int index,env_object * obj, const char * name)
{
    push_object(L,obj);
    lua_setfield(L, index, name);
}

void register_object(lua_State * L, env_object * obj, const char * name)
{
    register_object(L, -2, obj, name);
}

env_object * get_object(lua_State * L, int index, const char * name)
{
    lua_getfield(L,index,name);
    return get_object(L,-1);
}

env_object * get_object(lua_State * L,int index)
{
    lua_CFunction func = lua_tocfunction(L,index);
    if(!func || func != (const void *)&detail::object_wrapper) return NULL;
    lua_getupvalue(L,index,1);
    env_object ** obj = reinterpret_cast<env_object **>(lua_touserdata(L,-1));
    lua_pop(L,1);
    return *obj;
}

} //namespace lua
} //namespace script
} //namespace fungu
