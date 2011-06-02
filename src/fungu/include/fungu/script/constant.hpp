/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CONSTANT_HPP
#define FUNGU_SCRIPT_CONSTANT_HPP

#include "callargs_serializer.hpp"

#ifdef FUNGU_WITH_LUA
#include "lua/push_value.hpp"
#endif

namespace fungu{
namespace script{

class env_object;

/**
    @brief Constant value wrapper.
*/
template<typename T>
class constant:public env_object
{
public:
    constant(const T & value)
     :m_value(value)
    {
        
    }
    
    /**
        @brief Returns DATA_OBJECT value.
    */
    object_type get_object_type()const
    {
        return DATA_OBJECT;
    }
    
    /**
        @brief Throws error(NO_WRITE).
    */
    any call(call_arguments & args,env_frame *)
    {
        throw error(NO_WRITE);
    }
    
    /**
        @brief Returns constant value. 
    */
    any value()
    {
        return m_value;
    }
    
    #ifdef FUNGU_WITH_LUA
    void value(lua_State * L)
    {
        lua::push_value(L, m_value);
    }
    #endif
private:
    const T m_value;
};

} //namespace script
} //namespace fungu

#endif
