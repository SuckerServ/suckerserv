/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_ENV_SYMBOL_HPP
#define FUNGU_SCRIPT_ENV_SYMBOL_HPP

#include "env_object.hpp"

namespace fungu{
namespace script{

class env_symbol_local;

class env_symbol
{
    friend class env_symbol_local;
public:
    env_symbol();
    ~env_symbol();
    env_symbol_local * push_local_object(env_object *, const env_frame *);
    env_object * get_global_object();
    void set_global_object(env_object *);
    void unset_global_object();
    env_object * lookup_object(const env_frame *)const;
private:
    env_symbol(const env_symbol &);
    
    env_symbol_local * m_local;
    env_object::shared_ptr m_global;
};

class env_symbol_local
{
public:
    env_symbol_local(env_symbol *, const env_frame *);
    ~env_symbol_local();
    void attach();
    void detach();
    void set_object(env_object *);
    env_object * get_object()const;
    env_object * lookup_object(const env_frame *)const;
    const env_frame * get_frame()const;
    env_symbol_local * get_next_frame_sibling()const;
private:
    env_symbol_local(const env_symbol_local &);
    bool is_latest_attachment()const;
    
    env_symbol * m_symbol;
    env_symbol_local * m_super;
    const env_frame * m_frame;
    env_symbol_local * m_frame_sibling;
    env_object::shared_ptr m_object;
};

} //namespace script
} //namespace fungu

#endif
