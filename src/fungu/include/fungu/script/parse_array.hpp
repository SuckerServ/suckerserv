/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_PARSE_ARRAY_HPP
#define FUNGU_SCRIPT_PARSE_ARRAY_HPP

namespace fungu{
namespace script{

class env_frame;

struct array_word_exit_terminals
{
    static bool is_member(const_string::value_type c);
};

template<typename ForwardContainer,bool throw_exception>
bool parse_array(const_string string, env_frame * frame, ForwardContainer & container);

} //namespace script
} //namespace fungu

#endif
