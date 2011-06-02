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

namespace reflib{

class reference:public env_object
{
public:
    reference(object * object)
    {
        m_object = object->get_shared_ptr();
    }
    
    reference(shared_ptr obj)
     :m_object(obj)
    {
        
    }
    
    ~reference()
    {
        
    }
    
    object_type get_object_type()const
    {
        return m_object->get_object_type();
    }
    
    any call(call_arguments & args,frame * aScope)
    {
        return m_object->call(args,aScope);
    }
    
    any value()
    {
        return m_object->value();
    }
    
    void assign(const any & src)
    {
        m_object->assign(src);
    }
    
    object * lookup_member(const_string id)
    {
        return m_object->lookup_member(id);
    }
private:
    shared_ptr m_object;
};

inline any define_ref(env_object::call_arguments & args,env_frame * aFrame)
{
    const_string symbol_id = args.safe_casted_front<const_string>();
    args.pop_front();
    
    env_object::shared_ptr obj = aFrame->lookup_required_object(symbol_id)->get_shared_ptr();
    
    while(!args.empty())
    {
        symbol_id = args.casted_front<const_string>();
        obj = obj->lookup_member(symbol_id);
        args.pop_front();
        if(!obj) throw error(UNKNOWN_SYMBOL,boost::make_tuple(symbol_id.copy()));
    }
    
    reference * ref = new reference(obj);
    ref->set_adopted();
    return obj;
}

} //namespace detail

void register_reference_functions(env & environment)
{
    static function<raw_function_type> define_ref_func(reflib::define_ref);
    environment.bind_global_object(&define_ref_func,FUNGU_OBJECT_ID("ref"));
    
    //TODO weak references
    //static function<raw_function_type> define_weakref_func(reflib::define_weakref);
    //environment.bind_global_object(&define_weakref_func,FUNGU_OBJECT_ID("weakref"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
