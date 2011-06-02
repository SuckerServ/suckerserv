/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

macro::macro()
 :m_escape(1),m_con(NULL)
{
    
}

macro::~macro()
{
    if(m_con) delete m_con;
}

parse_state macro::parse(source_iterator * first,source_iterator last,env_frame * frame)
{
    if(m_con) return m_con->parse(first,last,frame);
    
    switch(**first)
    {
        case '@': m_escape++;                ++(*first); break;
        case '(': m_con = new subexpression; ++(*first); break;
        default: m_con = new symbol<expression::word_exit_terminals>;
    }
    
    return parse(first,last,frame);
}

any macro::eval(env_frame * frame)
{
    assert(m_con);
    return m_con->eval(frame);
}

int macro::get_evaluation_level()const
{
    return m_escape;
}

bool macro::is_string_constant()const
{
    return m_con->is_string_constant();
}

std::string macro::form_source()const
{
    return std::string(m_escape,'@') + m_con->form_source();
}

} //namespace script
} //namespace fungu
