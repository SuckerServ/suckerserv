/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

quote::quote()
 :m_escaped(false)
{
    
}

parse_state quote::parse(source_iterator * first,source_iterator last,env_frame *)
{
    for(; *first <= last; ++(*first))
    {
        char c = **first;
        
        if(m_escaped)
        {
            switch(c)
            {
                case '\"': c='\"'; break;
                case '\\': c='\\'; break; 
                case 'n':  c='\n'; break;
                case 'r':  c='\r'; break;
                case 't':  c='\t'; break;
                case 'f':  c='\f'; break;
                case 'b':  c='\b'; break;
                default:;//throw unexpected symbol?
            }
            m_escaped = false;
        }
        else
        {
            switch(c)
            {
                case '\\':
                case '^':
                    m_escaped = true;
                    break;
                case '\"':
                    ++(*first);
                    return PARSE_COMPLETE;
                case '\r':
                case '\n':
                    throw error(EXPECTED_SYMBOL,boost::make_tuple('\"'));
                default:;
            }
        }
        
        if(!m_escaped) m_string.push_back(c);
    }
    
    return PARSE_PARSING;
}

any quote::eval(env_frame *)
{
    return const_string(m_string);
}

bool quote::is_string_constant()const
{
    return true;
}

std::string quote::form_source()const
{
    std::stringstream out;
    out<<"\""<<lexical_cast_detail::write_string_literal(m_string.begin(), m_string.end())<<"\"";
    return out.str();
}

} //namespace script
} //namespace fungu
