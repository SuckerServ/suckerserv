/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_ERROR_HPP
#define FUNGU_SCRIPT_ERROR_HPP

#include "../string.hpp"

#include <boost/any.hpp>
#include <boost/tuple/tuple.hpp>

namespace fungu{
namespace script{

class source_context
{
public:
    source_context();
    virtual ~source_context();
    virtual source_context * clone() const = 0;
    virtual const char * get_uri_scheme() const = 0;
    virtual std::string get_location() const = 0;
    int get_line_number()const;
    void set_line_number(int linenum);
private:
    int m_linenum;
};

class file_source_context:public source_context
{
public:
    file_source_context(const std::string & filename);
    file_source_context(const std::string & filename, int);
    source_context * clone()const;
    const char * get_uri_scheme()const;
    std::string get_location()const;
private:
    std::string m_filename;
};

class local_source_context:public source_context
{
public:
    local_source_context();
    source_context * clone()const;
    const char * get_uri_scheme()const;
    std::string get_location()const;
};

class string_source_context:public source_context
{
public:
    string_source_context(const std::string &);
    string_source_context(const std::string &,int);
    source_context * clone()const;
    const char * get_uri_scheme()const;
    std::string get_location()const;
private:
    std::string m_source;
};

enum error_code
{
    EXPECTED_SYMBOL = 1,        // missing syntax symbol
    UNEXPECTED_SYMBOL,          // unexpected syntax symbol used
    UNEXPECTED_EOF,             // parser unexpectedly reached end of the file
    EXTERNAL_PARSE_ERROR,       // parse error in external syntax (e.g. malformed syntax in JSON)
    NOT_ENOUGH_ARGUMENTS,       // operation requires more arguments
    TOO_MANY_ARGUMENTS,         // operation requires less arguments
    MAXARGS,                    // exceeded the maximum argument list size
    BAD_CAST,                   // type cast failure
    NO_CAST,                    // no casting operation supported between Target and Source type
    INVALID_TYPE,               // an object of the wrong type was given
    INVALID_VALUE,              // invalid value
    INTEGER_UNDERFLOW,          // numeric_cast failure
    INTEGER_OVERFLOW,           // numeric cast failure
    DIVIDE_BY_ZERO,             // div op failure
    NO_VALUE,                   // object has no value
    UNKNOWN_SYMBOL,             // symbol lookup failed
    NO_BIND,                    // cannot bind object to symbol
    NO_WRITE,                   // cannot write value
    NO_READ,                    // cannot read value
    PERMISSION_DENIED,          // operation was rejected
    OPERATION_ERROR,            // internal function error
    UNSUPPORTED,                // whatever was attempted to be done is unsupported
    HIT_RECURSION_LIMIT,        // a function exceeded the environment's recursion limit
    MEMBER_ACCESS_CHAIN_LIMIT,  // too many members (e.g $a.b.c.d.e ...)
    SCRIPT_THROW,               // a user-defined exception thrown from script land
    LUA_ERROR                   // error in lua script
};

class error_exception{};

/**
    
*/
class error:public error_exception
{
public:
    template<typename ArgumentsTuple> 
     error(error_code code,ArgumentsTuple args):m_code(code), m_args(args){}
    error(error_code code);
    error_code get_error_code()const;
    const boost::any & get_error_arguments()const;
    std::string get_error_message()const;
private:
    error_code m_code;
    boost::any m_args;
};

/**

*/
class error_trace:public error_exception
{
public:
    explicit error_trace(const error & key,const_string arg,source_context * src_ctx);
    explicit error_trace(error_trace * head_info,const_string arg,source_context * src_ctx);
    ~error_trace();
    const error & get_error()const;
    const error_trace * get_parent_info()const;
    const error_trace * get_root_info()const;
    const source_context * get_source_context()const;
    const_string get_arg()const;
private:
    error m_key;
    error_trace * m_head_info;
    const_string m_arg;
    source_context * m_source_ctx;
};

} //namespace script
} //namespace fungu

#endif
