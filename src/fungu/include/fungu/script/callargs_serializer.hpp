/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CALLARGS_SERIALIZER_HPP
#define FUNGU_SCRIPT_CALLARGS_SERIALIZER_HPP

#include "any.hpp"
#include "callargs.hpp"
#include "lexical_cast.hpp"
#include "../dynamic_caller.hpp"
#include "parse_array.hpp"
#include "code_block.hpp"

#include <list>
#include <boost/intrusive_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace fungu{
namespace script{

class callargs_serializer
{
public:
    typedef any return_type;
    typedef callargs::value_type serialized_argument_type;
    
    callargs_serializer();
    callargs_serializer(callargs & argv, env_frame * frame);
    
    template<typename T>
    return_type serialize(const T & value)
    {
        return value;
    }
    
    any serialize(any value);
    
    template<typename T>
    T deserialize_return_value(const serialized_argument_type & value,type_tag<T>)
    {
        return lexical_cast<T>(value);
    }
    
    void deserialize_return_value(const serialized_argument_type &, type_tag<void>);
    
    template<typename T>
    T deserialize(const serialized_argument_type &, type_tag<T>)
    {
        return m_argv.casted_front<T>();
    }
    
    serialized_argument_type & deserialize(serialized_argument_type & value, type_tag<serialized_argument_type>);
    
    const char * deserialize(const serialized_argument_type & value, type_tag<const char *>);
    
    template<typename T>
    std::vector<T> deserialize(const serialized_argument_type & value, type_tag<std::vector<T> >)
    {
        std::vector<T> a;
        parse_array<std::vector<T>,true>(value.to_string(), m_frame, a);
        return a;
    }
    
    template<typename T>
    boost::shared_ptr<T> deserialize(const serialized_argument_type & value, type_tag<boost::shared_ptr<T> >)
    {
        return any_cast<boost::shared_ptr<T> >(value);
    }
    
    template<typename T>
    boost::intrusive_ptr<T> deserialize(const serialized_argument_type & value, type_tag<boost::intrusive_ptr<T> >)
    {
        return any_cast<boost::intrusive_ptr<T> >(value);
    }
    
    code_block deserialize(const serialized_argument_type &, type_tag<code_block>);
    return_type get_void_value();
    void clear();
private:
    callargs & m_argv; 
    env_frame * m_frame;
    std::list<const_string> m_string_list;
};

} //namespace script
} //namespace fungu

#endif
