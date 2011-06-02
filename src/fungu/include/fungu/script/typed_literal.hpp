/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_TYPED_LITERAL_HPP
#define FUNGU_SCRIPT_TYPED_LITERAL_HPP

#include "construct.hpp"

namespace fungu{
namespace script{

template<typename T>
class typed_literal:public construct
{
public:
    typed_literal(const T & value)
     :m_value(value)
    {
    
    }
    
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame)
    {
        assert(0);
        throw error(UNSUPPORTED);
    }
    
    any eval(env_frame *)
    {
        return m_value;
    }
    
    bool is_string_constant()const
    {
        return true;
    }
    
    std::string form_source()const
    {
        return lexical_cast<std::string>(m_value);
    }
private:
    T m_value;
};

} //namespace script
} //namespace fungu

#endif
