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

arguments::arguments(lua_State * stack,const std::vector<any> * default_args,std::size_t func_arity)
 :m_stack(stack),
  m_arg_index(1),
  m_default_args(default_args),
  m_skip_defarg_index(0)
{
    m_argc = static_cast<int>(size());
    if(m_argc < static_cast<int>(func_arity) && default_args)
    {
        int missing = static_cast<int>(func_arity) - m_argc;
        m_skip_defarg_index = m_argc - (static_cast<int>(func_arity) - m_default_args->size());
        for(int i = 1; i<= missing; i++) lua_pushnil(stack); //correct arguments::size()
    }
}

arguments::value_type & arguments::front()
{
    return m_arg_index;
}

void arguments::pop_front()
{
    m_arg_index++;
}

std::size_t arguments::size()const
{
    return lua_gettop(m_stack);
}

bool arguments::deserialize(value_type arg,type_tag<bool>)
{
    if(arg >= m_argc + 1) return lexical_cast<bool>((*m_default_args)[(arg - m_argc - 1) + m_skip_defarg_index]);
    return lua_toboolean(m_stack, arg);
}

int arguments::deserialize(value_type arg,type_tag<int>)
{
    if(arg >= m_argc + 1) return lexical_cast<int>((*m_default_args)[(arg - m_argc - 1) + m_skip_defarg_index]);
    if(!lua_isnumber(m_stack, arg))
    {
        luaL_argerror(m_stack, arg, "expected integer");
        assert(false);
        return 0;
    }
    return lua_tointeger(m_stack, arg);
}

unsigned int arguments::deserialize(value_type arg, type_tag<unsigned int>)
{
    if(arg >= m_argc + 1) return lexical_cast<unsigned int>((*m_default_args)[(arg - m_argc - 1) + m_skip_defarg_index]);
    int n = lua_tointeger(m_stack, arg);
    if(!lua_isnumber(m_stack, arg) || n < 0)
    {
        luaL_argerror(m_stack, arg, "expected positive integer");
        assert(false);
        return 0;
    }
    return n;
}

unsigned short arguments::deserialize(value_type arg, type_tag<unsigned short>)
{
    if(arg >= m_argc + 1) return lexical_cast<unsigned short>((*m_default_args)[(arg - m_argc - 1) + m_skip_defarg_index]);
    int n = lua_tointeger(m_stack, arg);
    if(!lua_isnumber(m_stack, arg) || n < 0 || n > std::numeric_limits<unsigned short>::max())
    {
        luaL_argerror(m_stack, arg, "expected positive short integer");
        assert(false);
        return 0;
    }
    return static_cast<unsigned short>(n);
}

static const char * get_cstr_from_lua_stack(lua_State * stack, int index)
{
    const char * str = lua_tostring(stack, index);
    if(!str)
    {
        if(lua_isboolean(stack, index)) return (lua_toboolean(stack,index) ? "1" : "0");
        luaL_argerror(stack, index, "expected string");
        assert(false);
        return NULL;
    }
    return str;
}

const char * arguments::deserialize(value_type arg, type_tag<const char *>)
{
    if(arg >= m_argc + 1) return lexical_cast<const char *>((*m_default_args)[(arg - m_argc - 1) + m_skip_defarg_index]);
    return get_cstr_from_lua_stack(m_stack, arg);
}

std::string arguments::deserialize(value_type arg, type_tag<std::string>)
{
    if(arg >= m_argc + 1) return lexical_cast<std::string>((*m_default_args)[(arg - m_argc - 1) + m_skip_defarg_index]);
    const char * val = get_cstr_from_lua_stack(m_stack, arg);
    return std::string(val ? val : "");
}

arguments::value_type arguments::serialize(const char * str)
{
    lua_pushstring(m_stack,str);
    return 1;
}

arguments::value_type arguments::serialize(const std::string & str)
{
    return serialize(str.c_str());
}

arguments::value_type arguments::serialize(int n)
{
    lua_pushinteger(m_stack, n);
    return 1;
}

arguments::value_type arguments::serialize(bool value)
{
    lua_pushboolean(m_stack, value);
    return 1;
}

arguments::value_type arguments::serialize(unsigned long value)
{
    lua_pushnumber(m_stack, static_cast<lua_Number>(value));
    return 1;
}

arguments::value_type arguments::get_void_value()
{
    return 0;
}

any get_argument_value(lua_State * L, int index)
{
    switch(lua_type(L, index))
    {
        case LUA_TNIL:
            return any::null_value();
        case LUA_TNUMBER:
        {
            lua_Integer i = lua_tointeger(L, index);
            lua_Number n = lua_tonumber(L, index);
            if(n==i) return any(static_cast<int>(i));
            else return any(n);
        }
        case LUA_TBOOLEAN:
            return static_cast<bool>(lua_toboolean(L, index));
        case LUA_TSTRING:
            return const_string(lua_tostring(L, index));
        case LUA_TFUNCTION:
        {
            env_object * obj = new lua_function(L, index);
            obj->set_adopted();
            return obj->get_shared_ptr();
        }
        #if 0
        case LUA_TTABLE:
        {
            table * outT = new table();

            lua_pushnil(L);
            while(lua_next(L, -2) != 0)
            {
                try
                {
                    any value = get_argument_value(L);
                    lua_pop(L,1);
                    std::string name = get_argument_value(L).to_string().copy();
                    outT->assign(name, value);
                }
                catch(error)
                {
                    delete outT;
                    throw;
                }
            }
            
            outT->set_adopted();
            return outT->get_shared_ptr();
        }
        #endif
        default:
            return any::null_value();
    }
}

env_object * lua_value_to_env_object(lua_State * L, int index)
{
    if(lua_type(L, index) == LUA_TFUNCTION)
    {
        env_object * obj = new lua_function(L, index);
        obj->set_adopted();
        return obj;
    }
    else
    {
        any_variable * var = new any_variable;
        var->set_adopted();
        try
        {
            var->assign(get_argument_value(L, index));
        }
        catch(error_exception)
        {
            delete var;
        }
        return var;
    }
}

} //namespace lua
} //namespace script
} //namespace fungu
