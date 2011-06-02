/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_FUNCTION_HPP
#define FUNGU_SCRIPT_FUNCTION_HPP

#include "env.hpp"
#include "../dynamic_call.hpp"
#include "callargs_serializer.hpp"

#ifdef FUNGU_WITH_LUA
#include "lua/arguments.hpp"
#endif

#include <boost/function.hpp>
#include <vector>

namespace fungu{
namespace script{

/**
    @brief C++ function wrapper.
*/
template<typename Signature>
class function:public env_object
{
public:
    template<typename Functor>
    function(Functor aFunctor)
     :m_function(aFunctor), m_default_args(NULL)
    {
        
    }
    
    template<typename Functor>
    function(Functor fun, const std::vector<any> * default_args)
     :m_function(fun), 
      m_default_args(default_args)
    {
        
    }
    
    object_type get_object_type()const
    {
        return FUNCTION_OBJECT;
    }
    
    any call(call_arguments & call_args, frame * aFrame)
    {
        // Fill in missing arguments with default values
        if(call_args.size() < boost::function_traits<Signature>::arity && 
            m_default_args && 
            call_args.size() + m_default_args->size() >= boost::function_traits<Signature>::arity)
        {
            int skip = call_args.size() - (boost::function_traits<Signature>::arity - m_default_args->size());
            for(std::vector<any>::const_iterator it = m_default_args->begin(); it != m_default_args->end(); ++it)
                if(--skip < 0) call_args.push_back(*it);
        }
        
        callargs_serializer serializer(call_args, aFrame);
        try
        {
            return dynamic_call<Signature>(m_function, call_args, serializer);
        }
        catch(missing_args)
        {
            throw error(NOT_ENOUGH_ARGUMENTS);
        }
    }
    
    #ifdef FUNGU_WITH_LUA
    int call(lua_State * L)
    {
        lua::arguments args(L, m_default_args, boost::function_traits<Signature>::arity);
        try
        {
            return dynamic_call<Signature>(m_function, args, args);
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
    #endif
private:
    boost::function<Signature> m_function;
    const std::vector<any> * m_default_args;
};

typedef any (raw_function_type)(env_object::call_arguments &, env_frame *);

template<>
class function<raw_function_type>:public env_object
{
public:
    template<typename Functor> function(Functor aFunctor):m_function(aFunctor){}
    
    any call(call_arguments & call_args, frame * aScope)
    {
        return m_function(call_args,aScope);
    }
private:
    boost::function<any (env_object::call_arguments &,env_frame *)> m_function;
};

} //namespace script
} //namespace fungu

#endif
