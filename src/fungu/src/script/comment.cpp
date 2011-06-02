/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

comment::comment()
 :m_first(const_string::null_const_iterator())
{
    
}

parse_state comment::parse(source_iterator * first,source_iterator last,env_frame *)
{
    if(m_first == const_string::null_const_iterator())
        m_first = *first;
    
    for(; *first <= last; ++(*first) )
        if(is_terminator(**first))
        {
            m_last = (*first) -1;
            return PARSE_COMPLETE;
        }
    
    return PARSE_PARSING;
}

any comment::eval(env_frame *)
{
    return const_string();
}

bool comment::is_eval_supported()const
{
    return false;
}

bool comment::is_string_constant()const
{
    return true;
}

std::string comment::form_source()const
{
    assert(m_first != const_string::null_const_iterator());
    const_string source(m_first,m_last);
    return std::string(2,'/') + std::string(source.begin(),source.end());
}

bool comment::is_terminator(const_string::value_type c)
{
    return c == '\r' || c=='\n';
}

} //namespace script
} //namespace fungu
