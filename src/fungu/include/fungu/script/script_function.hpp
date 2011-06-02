/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_SCRIPT_FUNCTION_HPP
#define FUNGU_SCRIPT_SCRIPT_FUNCTION_HPP

#include "callargs.hpp"
#include "callargs_serializer.hpp"
#include "../generic_script_function.hpp"

namespace fungu{
namespace script{

template<typename Signature>
class script_function:public generic_script_function<Signature, std::vector<any>, callargs_serializer, error>
{
public:
    typedef any (* error_handler_function)(error_trace *);
    
    script_function(env_object::shared_ptr object, env * environment, error_handler_function error_handler)
     :m_object(object),
      m_env(environment),
      m_error_handler(error_handler)
    {
        
    }
protected:
    std::vector<any>::value_type call(std::vector<any> * args)
    {
        any result;

        try
        {
            env_frame callframe(m_env);
            callargs callargs(*args);
            
            result = m_object->call(callargs, &callframe);
        }
        catch(error e)
        {
            result = m_error_handler(create_error_trace(e, m_env->get_source_context()));
        }
        catch(error_trace * errinfo)
        {
            result = m_error_handler(errinfo);
        }
        
        return result;
    }
    
    std::vector<any>::value_type error_handler(int arg, error err)
    {
        return m_error_handler(create_error_trace(err,NULL));
    }
private:
    error_trace * create_error_trace(error e,const source_context * ctx)
    {
        source_context * newCtx = ctx ? ctx->clone() : NULL;
        return new error_trace(e,"",newCtx);
    }
    
    env_object::shared_ptr m_object;
    env * m_env;
    error_handler_function m_error_handler;
};

} //namespace script
} //namespace fungu

#endif
