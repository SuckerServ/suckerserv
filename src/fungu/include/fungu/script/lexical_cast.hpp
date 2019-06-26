/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_LEXICAL_CAST_HPP
#define FUNGU_SCRIPT_LEXICAL_CAST_HPP

#include "../string.hpp"
#include "../type_tag.hpp"
#include "error.hpp"
#include "any.hpp"

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/intrusive_ptr.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <vector>
#include <deque>

namespace fungu{
namespace script{

namespace any_detail{
struct empty;
}

namespace lexical_cast_detail{

template<typename Target,typename Source>
struct nocastop{
    Target operator()(const Source & src)
    {
        throw error(NO_CAST);
    }
};

template<typename Target,typename Source>
struct nocastneeded
{
    Target operator()(const Source & src)
    {
        return src;
    }
};

template<typename Target,typename Source>
inline Target lexical_cast(const Source & src, type_tag<Target>)
{
    typename boost::mpl::if_<
        boost::mpl::bool_<boost::is_same<Target,Source>::value>,
        nocastneeded<Target,Source>,
        nocastop<Target,Source> >::type t;
    return t(src);
}

const_string    lexical_cast(const std::string & src,type_tag<const_string>);
std::string     lexical_cast(const const_string & src, type_tag<std::string>);
int             lexical_cast(const const_string & src, type_tag<int>);
unsigned int    lexical_cast(const const_string & src, type_tag<unsigned int>);
const_string    lexical_cast(int src, type_tag<const_string>);
int             lexical_cast(const std::string & src, type_tag<int>);
std::string     lexical_cast(int src, type_tag<std::string>);
const_string    lexical_cast(bool src, type_tag<const_string>);
bool            lexical_cast(const const_string & src, type_tag<bool>);
char            lexical_cast(const const_string & src, type_tag<char>);
const_string    lexical_cast(char src, type_tag<const_string>);
float           lexical_cast(const const_string & src,type_tag<float>);
const_string    lexical_cast(float src,type_tag<const_string>);
unsigned short  lexical_cast(const const_string & src,type_tag<unsigned short>);
const_string    lexical_cast(unsigned short src, type_tag<const_string>);
const_string    lexical_cast(unsigned int src, type_tag<const_string>);
const_string    lexical_cast(const char * src,type_tag<const_string>);
const_string    lexical_cast(const any_detail::empty &, type_tag<const_string>);

template<typename Source>
inline const_string lexical_cast(const boost::intrusive_ptr<Source> & src,type_tag<const_string>)
{
    throw error(NO_CAST);
}

// Lexical casting involving any type

template<typename Target>
inline Target lexical_cast_from_any(const any & arg,type_tag<Target>)
{
    if(any_is_string(arg))
        return lexical_cast(any_cast<const_string>(arg),type_tag<Target>());
    else
        return any_cast<Target>(arg);
}

const_string lexical_cast_from_any(const any & arg,type_tag<const_string>);
std::string lexical_cast_from_any(const any & arg,type_tag<std::string>);

template<typename Target>
inline Target lexical_cast(const any & src,type_tag<Target>)
{
    try
    {
        return lexical_cast_detail::lexical_cast_from_any(src,type_tag<Target>());
    }
    catch(bad_any_cast)
    {
        throw error(BAD_CAST);
    }
    catch(boost::numeric::positive_overflow)
    {
        throw error(INTEGER_OVERFLOW);
    }
    catch(boost::numeric::negative_overflow)
    {
        throw error(INTEGER_UNDERFLOW);
    }
    catch(boost::numeric::bad_numeric_cast)
    {
        assert(false);
        throw error(BAD_CAST);
    }
}

template<typename CharIterator>
std::basic_string<typename CharIterator::value_type> write_string_literal(CharIterator start,CharIterator end)
{
    std::basic_string<typename CharIterator::value_type> out;
    out.reserve(end - start);
    for(CharIterator it = start; it != end; it++)
    {
        switch(*it)
        {
            case '\"': out.append("\\\""); break;
            case '\\': out.append("\\");   break;
            case '\r': out.append("\\r");  break;
            case '\n': out.append("\\n");  break;
            case '\f': out.append("\\f");  break;
            case '\b': out.append("\\b");  break;
            default: out.push_back(*it);
        }
    }
    return out;
}

// printing array stuff (must be parsable by the functions in parse_array.hpp)

std::string write_sequence_element(const std::string & value);

template<typename T>
std::string write_sequence_element(const T & value)
{
    return lexical_cast_detail::lexical_cast(value, type_tag<const_string>()).copy();
}

template<typename ForwardContainer>
std::string write_sequence(const ForwardContainer & container)
{
    std::string output;
    for(typename ForwardContainer::const_iterator it = container.begin();
        it != container.end(); 
        ++it)
    {
        output.append(it == container.begin() ? "" : " ");
        output.append(write_sequence_element(*it));
    }
    return output;
}

template<typename Source>
inline const_string lexical_cast(const std::vector<Source,std::allocator<Source> > & src,type_tag<const_string>)
{
    return const_string(write_sequence(src));
}

template<typename Source>
inline const_string lexical_cast(const std::deque<Source,std::allocator<Source> > & src,type_tag<const_string>)
{
    return const_string(write_sequence(src));
}

} //namespace lexical_cast_detail

template<typename Target,typename Source>
inline Target lexical_cast(const Source & arg)
{
    return lexical_cast_detail::lexical_cast(arg, type_tag<Target>());
}

} //namespace script
} //namespace fungu

#endif
