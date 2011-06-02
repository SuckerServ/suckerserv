/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace lexical_cast_detail{

const_string lexical_cast(const std::string & src,type_tag<const_string>)
{
    return const_string(src);
}

std::string lexical_cast(const const_string & src,type_tag<std::string>)
{
    return src.copy();
}

int lexical_cast(const const_string & src, type_tag<int>)
{
    try
    {
        return to_int<int>(src);
    }
    catch(std::bad_cast)
    {
        throw error(BAD_CAST);
    }
}

unsigned int lexical_cast(const const_string & src, type_tag<unsigned int>)
{
    try
    {
        return to_int<unsigned int>(src);
    }
    catch(std::bad_cast)
    {
        throw error(BAD_CAST);
    }
}

const_string lexical_cast(int src, type_tag<const_string>)
{
    try
    {
        return from_int<int>(src);
    }
    catch(std::bad_cast)
    {
        throw error(BAD_CAST);
    }
}

int lexical_cast(const std::string & src, type_tag<int>)
{
    const_string str(src);
    return lexical_cast(str,type_tag<int>());
}

std::string lexical_cast(int src, type_tag<std::string>)
{
    return lexical_cast(src,type_tag<const_string>()).copy();
}

const_string lexical_cast(bool src, type_tag<const_string>)
{
    return lexical_cast(static_cast<int>(src),type_tag<const_string>());
}

bool lexical_cast(const const_string & src, type_tag<bool>)
{
    return lexical_cast(src, type_tag<int>());
}

char lexical_cast(const const_string & src, type_tag<char>)
{
    if(!src.length()) throw error(BAD_CAST);
    return *src.begin();
}

const_string lexical_cast(char src, type_tag<const_string>)
{
    return const_string(std::string(1,src));
}

float lexical_cast(const const_string & src,type_tag<float>)
{
    return boost::lexical_cast<float>(src);
}

const_string lexical_cast(float src,type_tag<const_string>)
{
    return const_string(boost::lexical_cast<std::string>(src));
}

unsigned short lexical_cast(const const_string & src,type_tag<unsigned short>)
{
    try
    {
        return to_int<unsigned short>(src);
    }
    catch(std::bad_cast)
    {
        throw error(BAD_CAST);
    }
}

const_string lexical_cast(unsigned short src, type_tag<const_string>)
{
    try
    {
        return from_int<unsigned short>(src);
    }
    catch(std::bad_cast)
    {
        throw error(BAD_CAST);
    }
}

const_string lexical_cast(unsigned int src, type_tag<const_string>)
{
    try
    {
        return from_int<unsigned int>(src);
    }
    catch(std::bad_cast)
    {
        throw error(BAD_CAST);
    }
}

const_string lexical_cast_from_any(const any & arg,type_tag<const_string>)
{
    return arg.to_string();
}

std::string lexical_cast_from_any(const any & arg,type_tag<std::string>)
{
    return arg.to_string().copy();
}

const_string lexical_cast(const char * src,type_tag<const_string>)
{
    return const_string(src);
}

const_string lexical_cast(const any_detail::empty &, type_tag<const_string>)
{
    return const_string();
}

#if 0
const_string lexical_cast(const json::object * src, type_tag<const_string>)
{
    std::stringstream output;
    output<<"{";
    for(json::object::const_iterator it = src->begin();
        it != src->end(); ++it)
    {
        output<<(it == src->begin() ? "" : ",")<<"\""<<it->first<<"\":"<<write_json_value(it->second.get());
    }
    output<<"}";
    return output.str();
}

const_string lexical_cast(boost::shared_ptr<json::object> src,type_tag<const_string>)
{
    return lexical_cast((const json::object *)src.get(),type_tag<const_string>());
}

const_string lexical_cast(const json::object & src,type_tag<const_string>)
{
    return lexical_cast((const json::object *)&src,type_tag<const_string>());
}
#endif

std::string write_sequence_element(const std::string & value)
{
    std::stringstream output;
    output<<"\""<<write_string_literal(value.begin(), value.end())<<"\"";
    return output.str();
}

} //namespace lexical_cast_detail
} //namespace script
} //namespace fungu
