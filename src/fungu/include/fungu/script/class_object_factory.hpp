/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CLASS_OBJECT_FACTORY_HPP
#define FUNGU_SCRIPT_CLASS_OBJECT_FACTORY_HPP

#include "class_object.hpp"
#include <map>

namespace fungu{
namespace script{

class class_object_factory
{
public:
    class constructor
    {
    public:
        virtual ~constructor(){}
        virtual env_object * create(call_arguments &)=0;
    };
    
    template<typename ClassType>
    class default_constructor:public constructor
    {
    public:
        env_object * create(call_arguments &)
        {
            ClassType * newobj = new ClassType;
            return new class_object<ClassType>(newobj);
        }
    };
    
    void register_class(const_string className, constructor * classCtor)
    {
        m_classes[className] = classCtor;
    }
    
    template<typename Class>
    void register_class(const_string className)
    {
        register_class(className, get_default_constructor<Class>());
    }
    
    env_object * create_object(const_string className, call_arguments & args)
    {
        class_map::const_iterator it = m_classes.find(className);
        if(it == m_classes.end()) return NULL;
        env_object * obj = it->second->create(args);
        return obj;
    }
private:
    template<typename Class>
    static default_constructor<Class> * get_default_constructor()
    {
        static default_constructor<Class> ctor;
        return &ctor;
    }
    
    typedef std::map<const_string, constructor *, const_string::less_than_comparator> class_map;
    class_map m_classes;
};

} //namespace script
} //namespace fungu

#endif
