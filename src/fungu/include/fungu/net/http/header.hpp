/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_HEADER_HPP
#define FUNGU_NET_HTTP_HEADER_HPP

#include <string>
#include <vector>
#include <utility>
#include <stack>

namespace fungu{
namespace http{

class field_value_token;
class header_field;

class headers_buffer
{
friend class field_value_token;
friend class header_field;
public:
    headers_buffer(char *);
    bool next_header_field(header_field &);
private:
    char * m_headers;
    char * m_next_field;
};

typedef std::pair<const char *, const char *> field_value_string;

class field_value_token
{
public:
    //token types
    static const char atom = '\0';
    static const char linear_whitespace = ' ';
    static const char quoted_string = '\"';
    static const char comment = '(';
    static const char domain_literal = '[';
    static const char message_id = '<';
    field_value_token();
    bool parse(const char **);
    char get_type()const;
    bool is_separator_type()const;
    field_value_string get_content()const;
private:
    bool parse_atom(const char **);
    char m_type;
    const char * m_start;
    const char * m_last;
};

class header_field
{
public:
    enum parse_error
    {
        OK = 0,
        EMPTY,
        MALFORMED
    };
    parse_error parse(headers_buffer &);
    const char * get_name()const;
    const char * get_value()const;
    bool next_value_token(field_value_token &);
private:
    const char * m_name;
    const char * m_value;
    const char * m_next_value_token;
};

class field_value_token_stack
{
public:
    field_value_token_stack(header_field & field);
    const field_value_token * next_value_token();
    void pop();
private:
    header_field & m_field;
    std::stack<field_value_token> m_tokens;
};

class header_parameter
{
public:
    header_parameter(const field_value_token &, const field_value_token &);
    field_value_string name()const;
    field_value_string value()const;
private:
    field_value_token m_key;
    field_value_token m_value;
};

class content_type
{
public:
    bool parse(field_value_token_stack &);
    field_value_string type()const;
    field_value_string subtype()const;
    const std::vector<header_parameter> & parameters()const;
private:
    field_value_token m_type;
    field_value_token m_subtype;
    std::vector<header_parameter> m_parameters;
};

bool parse_host(header_field &, std::string *, std::string *);
bool parse_content_length(header_field &, std::size_t *);
bool parse_content_type(header_field &, content_type &);
bool parse_accept(header_field &, std::vector<content_type> &);

enum transfer_encoding_type
{
    CHUNKED = 0,
    IDENTITY,
    GZIP,
    COMPRESS,
    DEFLATE
};

bool parse_transfer_encoding(header_field &, transfer_encoding_type &);

time_t parse_date(const char *);

} //namespace http
} //namespace fungu

#endif
