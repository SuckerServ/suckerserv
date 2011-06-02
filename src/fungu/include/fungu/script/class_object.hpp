/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CLASS_OBJECT_HPP
#define FUNGU_SCRIPT_CLASS_OBJECT_HPP

#include "class_interface.hpp"

namespace fungu{
namespace script{

template<typename Class>
class class_object : public env_object
{
public:
    class_object(Class * object)
     :m_object(object)
    {
        
    }
    
    any call(call_arguments & args, frame * frm)
    {
        const_string methodName = args.safe_front().to_string();
        args.pop_front();
        return class_interface<Class>::get_declarator().call_method(m_object, methodName, args, frm);
    }
private:
    Class * m_object;
};

} //namespace script
} //namespace fungu

#endif
