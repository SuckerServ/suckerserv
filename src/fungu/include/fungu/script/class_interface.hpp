/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CLASS_INTERFACE_HPP
#define FUNGU_SCRIPT_CLASS_INTERFACE_HPP

#include "../dynamic_method_call.hpp"
#include "callargs_serializer.hpp"
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

namespace fungu{
namespace script{

template<typename Class>
class class_interface
{
private:
    class method_caller_base
    {
    public:
        virtual ~method_caller_base(){}
        virtual any call(Class *, env_object::call_arguments &, env_frame *)=0;
    };
public:
    template<typename MemberFunctionPointer>
    class_interface<Class> & method(MemberFunctionPointer member_function, const_string methodName)
    {
        class method_caller:public method_caller_base
        {
        public:
            method_caller(MemberFunctionPointer fun)
             :m_fun(fun)
            {
                
            }
            
            any call(Class * nativeObject, env_object::call_arguments & args, env_frame * frm)
            {
                callargs_serializer serializer(args, frm);
                return dynamic_method_call<MemberFunctionPointer>(m_fun, nativeObject, args, serializer);
            }
        private:
            MemberFunctionPointer m_fun;
        };
        
        method_caller_base * derivedCaller = new method_caller(member_function);
        m_members[methodName] = boost::shared_ptr<method_caller_base>(derivedCaller);
        
        return *this;
    }
    
    any call_method(Class * object, const_string methodName,
        env_object::call_arguments & args, env_frame * frame)
    {
        typename boost::unordered_map<const_string, boost::shared_ptr<method_caller_base> >::iterator it = m_members.find(methodName);
        if(it == m_members.end()) throw error(UNKNOWN_SYMBOL, boost::make_tuple(methodName.copy()));
        return it->second->call(object, args, frame);
    }
    
    static class_interface<Class> & get_declarator()
    {
        static class_interface<Class> instance;
        return instance;
    }
private:
    boost::unordered_map<const_string, boost::shared_ptr<method_caller_base> > m_members;
};

template<typename Class>
inline class_interface<Class> & class_members()
{
    return class_interface<Class>::get_declarator();
}

} //namespace script
} //namespace fungu

#endif
