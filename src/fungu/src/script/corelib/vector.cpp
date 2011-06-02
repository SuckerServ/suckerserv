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

namespace vectorlib{

inline const_string at(const std::vector<const_string> & v, int i)
{
    if(i >= static_cast<int>(v.size()) || i < 0) 
        throw error(OPERATION_ERROR,boost::make_tuple(std::string("index out of bounds")));
    return v[i];
}

inline int listlen(const std::vector<const_string> & v)
{
    return static_cast<int>(v.size());
}

any foreach_member(env_object::call_arguments & args,env_frame * frame)
{
    env_object::shared_ptr obj = any_cast<env_object::shared_ptr>(args.front());
    args.pop_front();
    
    callargs_serializer cs(args,frame);
    code_block cb = cs.deserialize(args.front(), type_tag<code_block>());
    args.pop_front();
    
    env_frame inner_frame(frame);
    
    const_string name_arg;
    managed_variable<const_string> name_arg_var(name_arg);
    name_arg_var.set_temporary();
    name_arg_var.lock_write(true);
    inner_frame.bind_object(&name_arg_var, FUNGU_OBJECT_ID("arg1"));
    
    any_variable value_arg;
    value_arg.set_temporary();
    inner_frame.bind_object(&value_arg, FUNGU_OBJECT_ID("arg2"));
    
    env_object::member_iterator * curmem = obj->first_member();
    any result;
    
    while(curmem)
    {
        name_arg = curmem->get_name();
        value_arg.assign(curmem->get_object()->value());
        result = cb.eval_each_expression(&inner_frame);
        
        if(!curmem->next())
        {
            delete curmem;
            curmem = NULL;
        }
    }
    
    return result;
}

any foreach(const_string varname, env_object::call_arguments & args,env_frame * frame)
{
    if(args.size() < 2) throw error(NOT_ENOUGH_ARGUMENTS,boost::make_tuple(2));
    
    if(args.front().get_type() == typeid(env_object::shared_ptr)) 
        return foreach_member(args,frame);
    
    callargs_serializer cs(args,frame);
    
    std::vector<const_string> v = cs.deserialize(args.front(), type_tag<std::vector<const_string> >());
    args.pop_front();
    
    code_block cb = cs.deserialize(args.front(), type_tag<code_block>());
    args.pop_front();
    
    any result;
    
    env_frame inner_frame(frame);
    
    const_string current_arg;
    managed_variable<const_string> current_arg_var(current_arg);
    current_arg_var.set_temporary();
    current_arg_var.lock_write(true);
    
    inner_frame.bind_object(&current_arg_var, varname);
    
    for(std::vector<const_string>::const_iterator it = v.begin();
         it != v.end(); it++)
    {
        current_arg = *it;
        result = cb.eval_each_expression(&inner_frame);
    }
    
    return result;
}

inline any looplist(env_object::call_arguments & args, env_frame * frame)
{
    const_string varname = args.safe_casted_front<const_string>();
    args.pop_front();
    return foreach(varname, args, frame);
}

bool is_member_of(const_string member, const std::vector<const_string> & v)
{
    for(std::vector<const_string>::const_iterator it = v.begin();
        it != v.end(); ++it) if( *it == member ) return true;
    return false;
}

} //namespace detail

void register_vector_functions(env & environment)
{
    static function<const_string (const std::vector<const_string> &,int)> at_func(vectorlib::at);
    environment.bind_global_object(&at_func, FUNGU_OBJECT_ID("at"));
    
    static function<int (const std::vector<const_string> &)> listlen_func(vectorlib::listlen);
    environment.bind_global_object(&listlen_func, FUNGU_OBJECT_ID("listlen"));
    
    static function<raw_function_type> foreach_func(boost::bind(vectorlib::foreach,FUNGU_OBJECT_ID("arg1"),_1,_2));
    environment.bind_global_object(&foreach_func, FUNGU_OBJECT_ID("foreach"));
    
    static function<raw_function_type> looplist_func(vectorlib::looplist);
    environment.bind_global_object(&looplist_func, FUNGU_OBJECT_ID("looplist"));
    
    static function<bool (const_string, const std::vector<const_string> &)> is_member_of_func(vectorlib::is_member_of);
    environment.bind_global_object(&is_member_of_func, FUNGU_OBJECT_ID("member?"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
