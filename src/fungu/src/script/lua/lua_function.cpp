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

lua_function::lua_function(lua_State * L,int index, const char * name)
 :m_location("")
{
    lua_getfield(L,index,name);
    assert(lua_type(L, -1) == LUA_TFUNCTION);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::lua_function(lua_State * L)
 :m_location("")
{
    assert(lua_type(L, -1) == LUA_TFUNCTION);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::lua_function(lua_State * L, int index)
 :m_location("")
{
    assert(lua_type(L, index) == LUA_TFUNCTION);
    lua_pushvalue(L, index);
    m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    m_lua = L;
}

lua_function::~lua_function()
{
    luaL_unref(m_lua, LUA_REGISTRYINDEX, m_ref);
}

static source_context * create_source_context(const lua_Debug & ar)
{
    if(!ar.source) return NULL;
    if(ar.source[0] == '@') return new file_source_context(ar.source+1, ar.currentline);
    else return new string_source_context(ar.short_src, ar.currentline);
}

static int call_error_handler(lua_State * L)
{
    error_trace ** errorTraceWriteback = reinterpret_cast<error_trace **>(lua_touserdata(L, lua_upvalueindex(1)));
    
    error error_object(LUA_ERROR, boost::make_tuple(L));
    
    lua_Debug ar;
    error_trace * parent = NULL;
    
    if(lua_getstack(L, 1, &ar))
    {
        lua_getinfo(L, "nlS", &ar);
        parent = new error_trace(error_object, std::string(ar.name ? ar.name : ""), create_source_context(ar));
    }
    else
    {
        return 1;
    }
    
    int level = 2;
    while(lua_getstack(L, level++, &ar))
    {
        lua_getinfo(L, "nlS", &ar);
        if(ar.what && ar.what[0] == 'C') continue; //cannot get source location of C function
        parent = new error_trace(parent, std::string(ar.name ? ar.name : ""), create_source_context(ar));
    }
    
    *errorTraceWriteback = parent;
    
    return 1;
}

any lua_function::call(call_arguments & args, env_frame * aFrame)
{
    lua_State * L = aFrame->get_env()->get_lua_state();
    
    error_trace * errorInfo = NULL;
    lua_pushlightuserdata(L, &errorInfo);
    lua_pushcclosure(L, &call_error_handler, 1);
    int errorHandlerIndex = lua_gettop(L);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
    
    int nargs = args.size();
    while(!args.empty())
    {
        args.front().push_value(L);
        args.pop_front();
    }
    
    int retstat = lua_pcall(L, nargs, 1, errorHandlerIndex);
    if(retstat != 0)
    {
        switch(retstat)
        {
            case LUA_ERRRUN:
                assert(errorInfo);
                throw errorInfo;
                break;
            case LUA_ERRMEM:
                throw error(LUA_ERROR, boost::make_tuple(L));
                break;
            case LUA_ERRERR:
            default:
                assert(false);
                break;
        }
    }
    else return get_argument_value(L);
}

int lua_function::call(lua_State * L)
{
    int argc = lua_gettop(L);
    
    lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
    lua_insert(L, 1);
    
    int retstat = lua_pcall(L, argc, LUA_MULTRET, 0);

    if(retstat != 0)
    {
        lua_error(L);
    }
    
    return lua_gettop(L);
}

const source_context * lua_function::get_source_context()const
{
    lua_rawgeti(m_lua, LUA_REGISTRYINDEX, m_ref);
    
    lua_Debug ar;
    ar.currentline = 0;
    ar.short_src[0] = '\0';
    if(lua_getinfo(m_lua, ">S", &ar))
    {
        m_location = file_source_context(ar.short_src);
        m_location.set_line_number(ar.linedefined);
    }
    
    return &m_location;
}

} //namespace lua
} //namespace script
} //namespace fungu
