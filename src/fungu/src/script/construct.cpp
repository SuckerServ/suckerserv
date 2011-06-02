/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

construct::construct()
 :m_next_sibling(NULL)
{
    
}

construct::construct(const construct &)
{
    
}

construct::~construct()
{
    delete m_next_sibling;
}

void construct::set_next_sibling(construct * next_sibling)
{
    m_next_sibling = next_sibling;
}

construct * construct::get_next_sibling()
{
    return m_next_sibling;
}

construct * construct::get_tail_sibling()
{
    construct * tail = this;
    while(tail->get_next_sibling())
        tail = tail->get_next_sibling();
    return tail;
}

bool construct::is_eval_supported() const
{
    return true;
}

} //namespace script
} //namespace fungu
