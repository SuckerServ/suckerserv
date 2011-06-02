/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace corelib{
    
namespace exceptionlib{

inline void _throw(const std::string & msg)
{
    throw error(SCRIPT_THROW, boost::make_tuple(msg));
}

template<typename T>
inline void rethrow(T e)
{
    throw e;
}

inline any _try(env_object::call_arguments & args,env_frame * frame)
{
    if(args.size() < 2) throw error(NOT_ENOUGH_ARGUMENTS,boost::make_tuple(1));
    callargs_serializer cs(args,frame);
    
    code_block trycode = cs.deserialize(args.front(), type_tag<code_block>());
    args.pop_front();
    
    code_block catchcode = cs.deserialize(args.front(), type_tag<code_block>());
    args.pop_front();
    
    try
    {
        return trycode.eval_each_expression(frame);
    }
    catch(error_trace * errinfo)
    {
        env_frame inner_frame(frame);
        
        function<void ()> rethrow_func(boost::bind(rethrow<error_trace *>,errinfo));
        rethrow_func.set_temporary();
        inner_frame.bind_object(&rethrow_func,FUNGU_OBJECT_ID("rethrow"));
        
        int error_code = errinfo->get_error().get_error_code();
        std::string error_message = errinfo->get_error().get_error_message();
        
        managed_variable<int> error_code_var(error_code);
        error_code_var.set_temporary();
        error_code_var.lock_write(true);
        inner_frame.bind_object(&error_code_var,FUNGU_OBJECT_ID("errcode"));
        
        managed_variable<std::string> error_message_var(error_message);
        error_message_var.set_temporary();
        error_message_var.lock_write(true);
        inner_frame.bind_object(&error_message_var,FUNGU_OBJECT_ID("errmsg"));
        
        any result = catchcode.eval_each_expression(&inner_frame);
        delete errinfo;
        return result;
    }
    catch(error err)
    {
        env_frame inner_frame(frame);
        
        function<void ()> rethrow_func(boost::bind(rethrow<error>,err));
        rethrow_func.set_temporary();
        inner_frame.bind_object(&rethrow_func,FUNGU_OBJECT_ID("rethrow"));
        
        int error_code = err.get_error_code();
        std::string error_message = err.get_error_message();
        
        managed_variable<int> error_code_var(error_code);
        error_code_var.set_temporary();
        error_code_var.lock_write(true);
        inner_frame.bind_object(&error_code_var,FUNGU_OBJECT_ID("errcode"));
        
        managed_variable<std::string> error_message_var(error_message);
        error_message_var.set_temporary();
        error_message_var.lock_write(true);
        inner_frame.bind_object(&error_message_var,FUNGU_OBJECT_ID("errmsg"));
        
        return catchcode.eval_each_expression(&inner_frame);
    }
}

} //namespace detail

void register_exception_functions(env & environment)
{
    static function<void (const std::string &)> throw_func(exceptionlib::_throw);
    environment.bind_global_object(&throw_func, FUNGU_OBJECT_ID("throw"));
    
    static function<raw_function_type> try_func(exceptionlib::_try);
    environment.bind_global_object(&try_func, FUNGU_OBJECT_ID("try"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
