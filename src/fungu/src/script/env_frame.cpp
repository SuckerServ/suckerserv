/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

class va_memfun:public env_object
{
public:
    typedef any (frame::*pointer)(call_arguments &);
    va_memfun(pointer ptr):m_fun(ptr){}
    any call(call_arguments & args,env_frame * x)
    {
        return (x->*m_fun)(args);
    }
    any value(){return get_shared_ptr();}
private:
    pointer m_fun;
};
    
env_frame::env_frame(env * envir)
 :m_env(envir),
  m_scope(this),
  m_last_binding(NULL),
  m_expired(false),
  m_detached(false)
{
    
}

env_frame::env_frame(env_frame * outer_frame)
 :m_env(outer_frame->m_env),
  m_scope(outer_frame->m_scope),
  m_last_binding(NULL),
  m_expired(false),
  m_detached(false)
{
    
}

env_frame::~env_frame()
{
    delete m_last_binding;
}

void env_frame::bind_object(env_object * obj, const_string id)
{
    return bind_object(obj, m_env->create_symbol(id));
}

void env_frame::bind_object(env_object * obj, env_symbol * sym)
{
    env_symbol_local * local = sym->push_local_object(obj, this);
    m_last_binding = local;
}

env_object * env_frame::lookup_object(const_string id)const
{
    const env_symbol * sym = m_env->lookup_symbol(id);
    if(!sym) return NULL;
    return lookup_object(sym);
}

env_object * env_frame::lookup_object(const env_symbol * sym)const
{
    return sym->lookup_object(this);
}

env_object * env_frame::lookup_required_object(const_string id)const
{
    env_object * obj = lookup_object(id);
    if(!obj) throw error(UNKNOWN_SYMBOL, boost::make_tuple(id.copy()));
    return obj;
}

env_object * env_frame::lookup_required_object(const env_symbol * sym)const
{
    env_object * obj = lookup_object(sym);
    if(!obj) throw error(UNKNOWN_SYMBOL); //FIXME add id tuple
    return obj;
}

bool env_frame::has_expired()const
{
    return m_scope->m_expired;
}

void env_frame::signal_return()
{
    m_scope->m_expired = true;
}

void env_frame::signal_return(any value)
{
    m_scope->m_result = value;
    m_scope->m_expired = true;
}

void env_frame::unset_return()
{
    m_scope->m_result.reset();
    m_scope->m_expired = false;
}

any env_frame::get_result_value()
{
    return m_scope->m_result;
}

env * env_frame::get_env()const
{
    return m_env;
}

env_frame * env_frame::get_scope_frame()const
{
    return m_scope;
}

void env_frame::register_functions(env & e)
{
    static va_memfun return_scriptfun(&env_frame::signal_return_scriptfun);
    e.bind_global_object(&return_scriptfun,FUNGU_OBJECT_ID("return"));

    static va_memfun result_scriptfun(&env_frame::set_result_scriptfun);
    e.bind_global_object(&result_scriptfun,FUNGU_OBJECT_ID("result"));

    static va_memfun bind_myvar_scriptfun_obj(&env_frame::bind_myvar_scriptfun);
    e.bind_global_object(&bind_myvar_scriptfun_obj,FUNGU_OBJECT_ID("my"));
    e.bind_global_object(&bind_myvar_scriptfun_obj,FUNGU_OBJECT_ID("local"));

    static va_memfun bind_globalvar_scriptfun_obj(&env_frame::bind_globalvar_scriptfun);
    e.bind_global_object(&bind_globalvar_scriptfun_obj,FUNGU_OBJECT_ID("global"));

    static va_memfun getvar_scriptfun_obj(&env_frame::getvar_scriptfun);
    e.bind_global_object(&getvar_scriptfun_obj,FUNGU_OBJECT_ID("get"));

    static va_memfun setvar_scriptfun_obj(&env_frame::setvar_scriptfun);
    e.bind_global_object(&setvar_scriptfun_obj,FUNGU_OBJECT_ID("set"));

    static va_memfun issymbol_scriptfun_obj(&env_frame::issymbol_scriptfun);
    e.bind_global_object(&issymbol_scriptfun_obj,FUNGU_OBJECT_ID("symbol?"));

    static va_memfun isnull_scriptfun_obj(&env_frame::isnull_scriptfun);
    e.bind_global_object(&isnull_scriptfun_obj,FUNGU_OBJECT_ID("null?"));

    static va_memfun retnull_scriptfun_obj(&env_frame::retnull_scriptfun);
    e.bind_global_object(&retnull_scriptfun_obj,FUNGU_OBJECT_ID("null"));

    static va_memfun isprocedure_scriptfun_obj(&env_frame::isprocedure_scriptfun);
    e.bind_global_object(&isprocedure_scriptfun_obj,FUNGU_OBJECT_ID("procedure?"));
}

void env_frame::attach_locals()
{
    assert(m_detached);
    if(!m_detached) return;

    for(env_symbol_local * local = m_last_binding; local; 
        local = local->get_next_frame_sibling() ) local->attach();
    
    m_detached = false;
}

void env_frame::detach_locals()
{
    assert(!m_detached);
    if(m_detached) return;
    
    for(env_symbol_local * local = m_last_binding; local; 
        local = local->get_next_frame_sibling() ) local->detach();
    
    m_detached = true;
}

bool env_frame::is_detached_from_env()const
{
    return m_detached;
}

env_symbol_local * env_frame::get_last_bind()const
{
    return m_last_binding;
}

env_frame::env_frame(const env_frame &)
{
    assert(false);
}

any env_frame::signal_return_scriptfun(callargs & args)
{
    if(!args.empty())
    {
        signal_return(args.front());
        args.pop_front();
    }
    else signal_return();
    return any::null_value();
}

any env_frame::set_result_scriptfun(callargs & args)
{
    m_scope->m_result = args.safe_front();
    args.pop_front();
    return m_result;
}

any env_frame::bind_myvar_scriptfun(callargs & args)
{
    return bind_var_scriptfun(args,1);
}

any env_frame::bind_localvar_scriptfun(callargs & args)
{
    return bind_var_scriptfun(args,2);
}

any env_frame::bind_globalvar_scriptfun(callargs & args)
{
    return bind_var_scriptfun(args,3);
}

any env_frame::bind_var_scriptfun(callargs & args,int scope)
{
    if(args.empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    const_string id = args.casted_front<const_string>();
    args.pop_front();

    any_variable * var = new any_variable;
    var->set_adopted();
    
    switch(scope)
    {
        case 1: //my
            bind_object(var, id.copy());
            break;
        case 3: //global
            m_env->bind_global_object(var, id.copy());
            break;
    }

    if(!args.empty())
    {
        var->assign(args.front());
        args.pop_front();
    }

    return var->value();
}

any env_frame::getvar_scriptfun(callargs & args)
{
    any subject = args.safe_front();
    args.pop_front();

    env_object * obj;
    if(subject.get_type() == typeid(env_object::shared_ptr))
        obj = any_cast<env_object::shared_ptr>(subject).get();
    else obj = lookup_required_object(subject.to_string());

    return obj->value();
}

any env_frame::setvar_scriptfun(callargs & args)
{
    any subject = args.safe_front();
    args.pop_front();

    env_object * obj;
    if(subject.get_type() == typeid(env_object::shared_ptr))
        obj = any_cast<env_object::shared_ptr>(subject).get();
    else obj = lookup_required_object(subject.to_string());

    obj->assign(args.safe_front());
    args.pop_front();

    return obj->value();
}

any env_frame::issymbol_scriptfun(callargs & args)
{
    const_string id = args.safe_casted_front<const_string>();
    args.pop_front();
    return (bool)lookup_object(id);
}

any env_frame::isnull_scriptfun(callargs & args)
{
    any value = args.safe_front();
    args.pop_front();
    return value.empty();
}

any env_frame::retnull_scriptfun(callargs & args)
{
    return any();
}

any env_frame::isprocedure_scriptfun(callargs & args)
{
    bool isProcedure = args.safe_front().get_type() == typeid(env_object::shared_ptr);
    args.pop_front();
    return isProcedure;
}

} //namespace script
} //namespace fungu
