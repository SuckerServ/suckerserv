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

namespace funclib{

class anonymous_function:public env_object
{
public:
    anonymous_function(const std::vector<const_string> & params, const_string code, frame * aFrame)
     :m_code(code, aFrame->get_env()->get_source_context()),
      m_recursion_depth(0)
    {
        m_code.compile(aFrame);
        
        m_params.reserve(params.size());
        for(std::vector<const_string>::const_iterator it = params.begin(); it != params.end(); ++it)
            m_params.push_back(aFrame->get_env()->create_symbol(*it));
    }
    
    ~anonymous_function()
    {
        
    }
    
    any call(call_arguments & callargs, frame * aFrame)
    {
        if(++m_recursion_depth > env::recursion_limit)
            throw error(HIT_RECURSION_LIMIT,boost::make_tuple(env::recursion_limit));
        
        if(callargs.size() < m_params.size()) throw error(NOT_ENOUGH_ARGUMENTS);
        
        std::vector<any_variable> arg(m_params.size());
        
        std::vector<any_variable>::iterator argIt = arg.begin();
        std::vector<env_symbol *>::iterator paramIt = m_params.begin();
        
        frame func_frame(aFrame->get_env());
        
        for(; paramIt != m_params.end(); ++argIt, ++paramIt)
        {
            (*argIt).set_temporary();
            (*argIt).assign(callargs.front());
            
            func_frame.bind_object(&(*argIt), *paramIt);
            
            callargs.pop_front();
        }
        
        try
        {
            any result = m_code.eval_each_expression(&func_frame);
            m_recursion_depth--;
            return result;
        }
        catch(...)
        {
            m_recursion_depth--;
            throw;
        }
    }
    
    any value()
    {
        return get_shared_ptr();
    }
    
    const source_context * get_source_context()const
    {
        return m_code.get_source_context();
    }
    
    static any define_anonymous_function(call_arguments & args, frame * aFrame)
    {
        std::vector<const_string> params;
        parse_array<std::vector<const_string>,true>(
            args.safe_casted_front<const_string>(), aFrame, params);
        args.pop_front();
        
        const_string code = args.safe_casted_front<const_string>();
        args.pop_front();
        
        anonymous_function * func_obj = NULL;
        try
        {
            func_obj = new anonymous_function(params, code, aFrame);
        }
        catch(error_trace *){delete func_obj; throw;}
        catch(error_exception){delete func_obj; throw;}
        
        func_obj->set_adopted();
        return func_obj->get_shared_ptr();
    }
private:
    code_block m_code;
    std::vector<env_symbol *> m_params;
    int m_recursion_depth;
};

} //namespace detail

void register_anonfunc_functions(env & environment)
{
    static function<raw_function_type> define_anonfun_func(funclib::anonymous_function::define_anonymous_function);
    environment.bind_global_object(&define_anonfun_func, FUNGU_OBJECT_ID("func"));
    environment.bind_global_object(&define_anonfun_func, FUNGU_OBJECT_ID("function"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
