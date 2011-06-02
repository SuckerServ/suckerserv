/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_EXPRESSION_HPP
#define FUNGU_SCRIPT_EXPRESSION_HPP

#include "construct.hpp"
#include <vector>

namespace fungu{
namespace script{

class env_symbol;
class source_context;

class expression:public construct
{
public:
    struct word_exit_terminals
    {
        static bool is_member(const_string::value_type c);
    };
    
    expression();
    ~expression();
    
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame);
    
    any eval(env_frame * frame);
    
    bool is_string_constant()const;
    
    bool is_empty_expression()const;
    
    std::string form_source()const;
private:
    void add_child_construct(construct * child);
    
    bool is_alias_assignment(env_frame * frame)const;
    void translate_alias_assignment(env_frame * frame);
    
    void fill_constarg_vector(env_frame * frame);
    void reset_placeholders();
public:
    source_context * get_source_context()const;
private:
    construct * m_parsing;
    construct * m_first_construct;

    std::vector<any> m_arguments;
    std::vector<unsigned char> m_placeholders;
    
    env_symbol * m_operation_symbol;
    
    // information for debugging
    unsigned short m_line;
    const source_context * m_source_ctx;
};

class base_expression:public expression
{
public:
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame);
    std::string form_source()const;
};

} //namespace script
} //namespace fungu

#endif
