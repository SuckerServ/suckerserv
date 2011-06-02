/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LUA_ARGUMENTS_HPP
#define FUNGU_SCRIPT_LUA_ARGUMENTS_HPP

#include "../../dynamic_call.hpp"
#include "../any.hpp"
#include "push_value.hpp"
#include <vector>
#include <cstddef>
#include <assert.h>

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace fungu{
namespace script{
namespace lua{

class arguments //...also serves as the Serialization class
{
public:
    typedef int value_type;
    
    arguments(lua_State * stack,const std::vector<any> *,std::size_t);
    value_type & front();
    
    void pop_front();
    
    std::size_t size()const;
    
    template<typename T>
    T deserialize(value_type arg, type_tag<T>)
    {
        luaL_argerror(m_stack, arg, "unsupported type");
        assert(false);
        throw; //used to supress no-return-value warning
    }
    
    bool deserialize(value_type arg,type_tag<bool>);
    int deserialize(value_type arg,type_tag<int>);
    unsigned int deserialize(value_type arg, type_tag<unsigned int>);
    unsigned short deserialize(value_type arg, type_tag<unsigned short>);
    unsigned long deserialize(value_type arg, type_tag<unsigned long>);
    const char * deserialize(value_type arg, type_tag<const char *>);
    std::string deserialize(value_type arg, type_tag<std::string>);
    
    template<typename T>
    value_type serialize(const T &)
    {
        luaL_error(m_stack,"return value is of an unsupported type");
        return 0;
    }
    
    value_type serialize(const char * str);
    value_type serialize(const std::string & str);
    value_type serialize(int n);
    value_type serialize(unsigned long);
    value_type serialize(bool);
    
    template<typename T>
    value_type serialize(const std::vector<T> & in)
    {
        lua_newtable(m_stack);
        
        int index = 1;
        for(typename std::vector<T>::const_iterator it = in.begin(); it != in.end(); ++it)
        {
            lua_pushinteger(m_stack, index++);
            push_value(m_stack, *it);
            lua_settable(m_stack, -3);
        }
        
        return 1;
    }
    
    value_type get_void_value();
private:
    lua_State * m_stack;
    int m_arg_index;
    const std::vector<any> * m_default_args;
    int m_skip_defarg_index;
    int m_argc;
};

any get_argument_value(lua_State *,int index = -1);
env_object * lua_value_to_env_object(lua_State *, int index = -1);

} //namespace lua
} //namespace script
} //namespace fungu

#endif
