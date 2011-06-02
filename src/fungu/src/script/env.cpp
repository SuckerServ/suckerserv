/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.h"
#endif

#include "fungu/stringutils.hpp"

#include "fungu/script/env.hpp"
#include "fungu/script/callargs.hpp"
#include "fungu/script/any_variable.hpp"

#ifdef FUNGU_WITH_LUA
#include "fungu/script/lua/push_value.hpp"
#include "fungu/script/lua/arguments.hpp"
#endif

#include "env_frame.cpp"
#include "env_object.cpp"
#include "env_symbol.cpp"

namespace fungu{
namespace script{

const int env::recursion_limit;

env::env()
  :m_source_ctx(new local_source_context)
{
    
}

env::~env()
{
    for(boost::unordered_map<const_string, env_symbol *>::iterator it = m_symbol.begin();
        it != m_symbol.end(); ++it) delete it->second;
    
    delete m_source_ctx;
}

env_symbol * env::lookup_symbol(const_string id)const
{
    boost::unordered_map<const_string, env_symbol *>::const_iterator it = m_symbol.find(id);
    if(it == m_symbol.end()) return NULL;
    return it->second;
}

env_symbol * env::create_symbol(const_string id)
{
    std::pair<boost::unordered_map<const_string, env_symbol *>::iterator, bool> insert_status = m_symbol.insert(std::pair<const_string,env_symbol *>(id,NULL));
    if(insert_status.second == false) return insert_status.first->second;
    env_symbol * newSymbol = new env_symbol;
    insert_status.first->second = newSymbol;
    return newSymbol;
}

void env::set_source_context(const source_context * source_ctx)
{
    m_source_ctx = source_ctx;
}

const source_context * env::get_source_context() const
{
    return m_source_ctx;
}

void env::bind_global_object(env_object * obj, const_string id)
{
    env_symbol * sym = create_symbol(id);
    sym->set_global_object(obj);
    if(m_bind_observer) m_bind_observer(id, obj);
}

void env::unbind_global_object(const_string id)
{
    env_symbol * sym = create_symbol(id);
    if(m_unbind_observer) m_unbind_observer(id, sym->get_global_object());
    sym->unset_global_object();
}

env_object * env::lookup_global_object(const_string id)const
{
    env_symbol * sym = lookup_symbol(id);
    if(!sym) return NULL;
    return sym->lookup_object(NULL);
}

env::observer_function env::unset_bind_observer()
{
    observer_function old_function = m_bind_observer;
    m_bind_observer = observer_function();
    return old_function;
}

env::observer_function env::unset_unbind_observer()
{
    observer_function old_function = m_unbind_observer;
    m_unbind_observer = observer_function();
    return old_function;   
}

#ifdef FUNGU_WITH_LUA
lua_State * env::get_lua_state()const
{
    return m_lua_state;
}

void env::set_lua_state(lua_State * state)
{
    m_lua_state = state;
}
#endif

void initialize_base_cast_operations();

void initialize_library()
{
    initialize_base_cast_operations();
}

} //namespace script
} //namespace fungu
