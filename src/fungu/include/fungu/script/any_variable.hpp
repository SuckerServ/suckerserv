/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_ANY_VARIABLE_HPP
#define FUNGU_SCRIPT_ANY_VARIABLE_HPP

#include "any.hpp"
#include "env_object.hpp"

namespace fungu{
namespace script{

class any_variable:public env_object
{
public:
    any_variable();
    ~any_variable();
    env_object::object_type get_object_type()const;
    void assign(const any & value);
    any call(call_arguments & args, env_frame * frame);    
    #ifdef FUNGU_WITH_LUA
    int call(lua_State * L);
    void value(lua_State *);
    #endif
    any value();
    env_object * lookup_member(const_string id);
    const any & get_any()const;
private:
    any m_any;
    bool m_procedure;
};

} //namespace script
} //namespace fungu

#endif
