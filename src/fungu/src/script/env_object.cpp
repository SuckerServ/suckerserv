/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

env_object::env_object()
:m_refcount(0),
 m_flags(0)
{
    
}

env_object::~env_object()
{
    
}

env_object::object_type env_object::get_object_type()const
{
    return env_object::UNCLASSIFIED_OBJECT;
}

#ifdef FUNGU_WITH_LUA

int env_object::call(lua_State * L)
{
    int argc = lua_gettop(L);
    
    std::vector<any> args;
    for(int i = 1; i <= argc; i++)
        args.push_back(lua::get_argument_value(L));
    
    lua_getfield(L, LUA_REGISTRYINDEX, "fungu_script_env");
    env * environment = reinterpret_cast<env *>(lua_touserdata(L, -1));
    if(!environment) return luaL_error(L, "missing 'fungu_script_env' field in lua registry");
    
    callargs callargs(args);
    frame callframe(environment);
    
    try
    {
        any result = call(callargs, &callframe);
        if(result.empty()) return 0;
        else
        {
            lua_pushstring(L, result.to_string().copy().c_str());
            return 1;
        }
    }
    catch(missing_args)
    {
        return luaL_error(L, "missing arguments");
    }
    catch(error err)
    {
        return luaL_error(L, "%s", err.get_error_message().c_str());
    }
    catch(error_trace * errinfo)
    {
        std::string msg = errinfo->get_error().get_error_message();
        delete errinfo;
        return luaL_error(L, "%s", msg.c_str());
    }
}

void env_object::value(lua_State * L)
{
    //return lua::push_value(L, value().to_string());
    value().push_value(L);
}

#endif

any env_object::value()
{
    return get_shared_ptr();
}

void env_object::assign(const any &)
{
    throw error(NO_WRITE);
}

env_object * env_object::lookup_member(const_string id)
{
    return NULL;
}

env_object::member_iterator * env_object::first_member()const
{
    return NULL;
}

void env_object::add_ref()
{
    m_refcount++;
}

env_object & env_object::unref()
{
    m_refcount--;
    return *this;
}

unsigned int env_object::get_refcount()const
{
    return m_refcount;
}

env_object & env_object::set_temporary()
{
    assert(m_refcount == 0);
    m_flags |= TEMPORARY_OBJECT;
    return *this;
}

env_object & env_object::set_adopted()
{
    assert(!is_temporary());
    m_flags |= ADOPTED_OBJECT;
    return *this;
}

bool env_object::is_temporary()const
{
    return (m_flags & TEMPORARY_OBJECT);
}

bool env_object::is_adopted()const
{
    return (m_flags & ADOPTED_OBJECT);
}

env_object::shared_ptr env_object::get_shared_ptr()
{
    if(is_temporary()) throw error(UNSUPPORTED);//TODO specialised error code
    return shared_ptr(this);
}

env_object::shared_ptr env_object::get_shared_ptr(env_object * obj)
{
    if(obj) return obj->get_shared_ptr();
    else return NULL;
}

} //namespace script
} //namespace fungu
