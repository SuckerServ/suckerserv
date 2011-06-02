/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

block::block()
 :m_first(const_string::null_const_iterator()),
  m_nested(0)
{
    
}

block::~block()
{
    for(std::vector<boost::tuple<source_iterator,source_iterator,macro *> >::iterator it = m_macros.begin();
    it != m_macros.end(); ++it) delete boost::get<2>(*it);
    m_macros.clear();
}

parse_state block::parse(source_iterator * first,source_iterator last,env_frame * frame)
{
    if(m_first == const_string::null_const_iterator())
        m_first = *first;
    
    bool parsing_macro = !m_macros.empty() && boost::get<1>(m_macros.back()) == const_string::null_const_iterator();
    if(parsing_macro)
    {
        macro * m = boost::get<2>(m_macros.back());
        parse_state substate = m->parse(first,last,frame);
        if(substate != PARSE_COMPLETE) return substate;
        
        boost::get<1>(m_macros.back()) = *first;
        
        if(m->get_evaluation_level() <= m_nested)
        {
            delete m;
            m_macros.pop_back();
        }
    }
    
    for(; *first <= last; ++(*first))
    {
        switch(**first)
        {
            case '[': m_nested++; break;
            case ']':
                if(!m_nested)
                {
                    m_last = (*first)-1;
                    ++(*first);
                    return PARSE_COMPLETE;
                }
                m_nested--;
                break;
            case '@':
                m_macros.push_back(boost::make_tuple(*first,const_string::null_const_iterator(),new macro));
                ++(*first);
                return parse(first,last,frame);
            default:;
        }
    }
    
    return PARSE_PARSING;
}

any block::eval(env_frame * frame)
{
    assert(m_first != const_string::null_const_iterator());
    if(is_string_constant())
    {
        return const_string(m_first,m_last);
    }
    else
    {
        std::string result;
        result.reserve(m_last - m_first +1);
        
        source_iterator gap = m_first;
        for(std::vector<boost::tuple<source_iterator,source_iterator,macro *> >::iterator it = m_macros.begin();
            it != m_macros.end(); ++it)
        {
            source_iterator start = boost::get<0>(*it);
            source_iterator end = boost::get<1>(*it);
            if(start - gap) result.append(std::string(gap,start));
            result.append(boost::get<2>(*it)->eval(frame).to_string().copy());
            gap = end;
        }
        if(gap <= m_last) result.append(std::string(gap,m_last+1));
        
        return const_string(result);
    }
}

bool block::is_string_constant()const
{
    return m_macros.empty();
}

std::string block::form_source()const
{
    assert(m_first != const_string::null_const_iterator());
    return std::string(1,'[') + const_string(m_first,m_last).copy() + std::string(1,']');
}

} //namespace script
} //namespace fungu
