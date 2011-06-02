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

namespace controlflow{

inline int predicate(const_string code,env_frame * aScope)
{
    return lexical_cast<int>(execute_text(code,aScope));
}

inline int predicate(code_block & cb,env_frame * aScope)
{
    return lexical_cast<int>(cb.eval_each_expression(aScope));
}

any if_(env_object::call_arguments & args, env_frame * frame)
{
    callargs_serializer cs(args,frame);
    
    if(args.size() < 2) throw error(NOT_ENOUGH_ARGUMENTS);
    
    code_block cond = cs.deserialize(args.front(),type_tag<code_block>());
    args.pop_front();
    
    code_block true_code = cs.deserialize(args.front(),type_tag<code_block>());
    args.pop_front();
    
    code_block false_code(const_string(),NULL);
    
    // code for false condition is an optional argument
    if(!args.empty())
    {
        false_code = cs.deserialize(args.front(),type_tag<code_block>());
        args.pop_front();
    }
    
    if(predicate(cond, frame)) return true_code.eval_each_expression(frame);
    else return false_code.eval_each_expression(frame);
}

any ternary(env_object::call_arguments & args, env_frame * frame)
{
    callargs_serializer cs(args,frame);
    
    code_block cond = cs.deserialize(args.front(),type_tag<code_block>());
    args.pop_front();
    
    any tc_value = args.safe_front();
    args.pop_front();
    
    any fc_value = args.safe_front();
    args.pop_front();
    
    return (predicate(cond, frame) ? tc_value : fc_value);
}

any while_(env_object::call_arguments & args,env_frame * aScope)
{
    env_frame while_scope(aScope);
    nullary_setter _continue;
    _continue.set_temporary();
    nullary_setter _break;
    _break.set_temporary();
    while_scope.bind_object(&_continue,FUNGU_OBJECT_ID("continue"));
    while_scope.bind_object(&_break,FUNGU_OBJECT_ID("break"));
    
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    code_block condition(lexical_cast<const_string>(args.front())); args.pop_front();
    condition.compile(&while_scope);
    
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    code_block body(lexical_cast<const_string>(args.front())); args.pop_front();
    body.compile(&while_scope);
    
    any while_body_result;
    
    while(predicate(condition,&while_scope) && 
         !while_scope.has_expired() && !_break.is_set())
    {
        for(code_block::iterator body_it=body.begin();
            body_it!=body.end() && !_continue.is_set() && !_break.is_set();
            ++body_it )
        {
            while_body_result = body_it->eval(&while_scope);
        }
    }
    
    return while_body_result;
}

inline any loop(env_object::call_arguments & args,env_frame * aScope)
{
    env_frame loop_frame(aScope);
    
    unsigned int counter=0;
    variable<unsigned int> counter_var(counter);
    
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    loop_frame.bind_object(&counter_var,lexical_cast<const_string>(args.front()));
    args.pop_front();
    
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    unsigned int count_to=lexical_cast<unsigned int>(args.front()); 
    args.pop_front();
    
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    code_block body(lexical_cast<const_string>(args.front())); 
    args.pop_front();
    body.compile(aScope);
    
    any body_result;
    
    for(; counter < count_to && !aScope->has_expired(); ++counter) 
        body_result = body.eval_each_expression(&loop_frame);
    
    return body_result;
}

inline any or_(env_object::call_arguments & args,env_frame * aFrame)
{
    bool retval = false;
    while(retval == false && !args.empty())
    {
        retval = retval || predicate(args.casted_front<const_string>(),aFrame);
        args.pop_front();
    }
    return retval;
}

inline any and_(env_object::call_arguments & args,env_frame * aFrame)
{
    bool retval = true;
    while(retval == true && !args.empty())
    {
        retval = retval && predicate(args.casted_front<const_string>(),aFrame);
        args.pop_front();
    }
    return retval;
}

} //namespace detail

void register_controlflow_functions(env & environment)
{
    static function<raw_function_type> conditional_if_func(controlflow::if_);
    environment.bind_global_object(&conditional_if_func,FUNGU_OBJECT_ID("if"));
    
    static function<raw_function_type> conditional_ternary_func(controlflow::ternary);
    environment.bind_global_object(&conditional_ternary_func,FUNGU_OBJECT_ID("?"));
    
    static function<raw_function_type> loop_while(controlflow::while_);
    environment.bind_global_object(&loop_while,FUNGU_OBJECT_ID("while"));
    
    static function<raw_function_type> loop_times(controlflow::loop);
    environment.bind_global_object(&loop_times,FUNGU_OBJECT_ID("loop"));
    
    static function<raw_function_type> or_(controlflow::or_);
    environment.bind_global_object(&or_,FUNGU_OBJECT_ID("||"));
    
    static function<raw_function_type> and_(controlflow::and_);
    environment.bind_global_object(&and_,FUNGU_OBJECT_ID("&&"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
