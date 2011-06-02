/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

env_symbol::env_symbol()
 :m_local(NULL)
{
    
}

env_symbol::env_symbol(const env_symbol &)
{
    assert(false);
}

env_symbol::~env_symbol()
{
    assert(!m_local);
}

env_symbol_local * env_symbol::push_local_object(env_object * obj, const env_frame * frm)
{
    if(m_local && m_local->get_frame() == frm) 
    {
        m_local->set_object(obj);
        return m_local;
    }
    
    env_symbol_local * newLocal = new env_symbol_local(this, frm);
    
    newLocal->set_object(obj);
    newLocal->attach();
    
    return newLocal;
}

env_object * env_symbol::get_global_object()
{
    return m_global.get();
}

void env_symbol::set_global_object(env_object * obj)
{
    assert(obj != NULL);
    m_global = env_object::shared_ptr(obj);
}

void env_symbol::unset_global_object()
{
    m_global = env_object::shared_ptr();
}

env_object * env_symbol::lookup_object(const env_frame * frm)const
{
    env_object * obj = (m_local && frm ? m_local->lookup_object(frm) : NULL);
    if(!obj) obj = m_global.get();
    return obj;
}

env_symbol_local::env_symbol_local(env_symbol * sym, const env_frame * frm)
 :m_symbol(sym),
  m_frame(frm),
  m_frame_sibling(frm->get_last_bind())
{
    
}

env_symbol_local::env_symbol_local(const env_symbol_local &)
{
    assert(false);
}

env_symbol_local::~env_symbol_local()
{
    if(is_latest_attachment()) detach();
    #ifndef NDEBUG
    else //check that the local is not attached at any level
    {
        env_symbol_local * cur = m_symbol->m_local;
        while(cur)
        {
            assert(cur != this);
            cur = cur->m_super;
        }
    }
    #endif
    
    delete m_frame_sibling;
}

void env_symbol_local::attach()
{
    m_super = m_symbol->m_local;
    m_symbol->m_local = this;
}

void env_symbol_local::detach()
{
    assert(is_latest_attachment());
    m_symbol->m_local = m_super;
}

void env_symbol_local::set_object(env_object * obj)
{
    m_object = env_object::shared_ptr(obj);
}

env_object * env_symbol_local::get_object()const
{
    return m_object.get();
}

env_object * env_symbol_local::lookup_object(const env_frame * frm)const
{
    bool inScope = frm == m_frame || frm->get_scope_frame() == m_frame->get_scope_frame();
    if(inScope) return m_object.get();
    else return NULL;
}

const env_frame * env_symbol_local::get_frame()const
{
    return m_frame;
}

env_symbol_local * env_symbol_local::get_next_frame_sibling()const
{
    return m_frame_sibling;
}

bool env_symbol_local::is_latest_attachment()const
{
    return m_symbol->m_local == this;
}

} //namespace script
} //namespace fungu
