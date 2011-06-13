/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/net/http/header.hpp"
#include "fungu/string.hpp"
#include "fungu/convert.hpp"
#include <cstddef>  // std::size_t
#include <ctype.h>  // tolower()
#include <string.h> // strstr()

namespace fungu{
namespace http{

static void erase_chars(std::size_t n, char * input)
{
    char * output = input;
    for(input = input + n; *input; input++) *(output++) = *input;
    *output = '\0';
}

static char * normalize_field_name(char * input)
{
    for(; *input && *input != ':'; input++) *input = tolower(*input);
    return input++;
}

static char * normalize_field_value(char * input)
{
    for(; *input; input++)
    {
        if(*input == '\r' && *(input + 1) == '\n')
        {
            // Unfold line continuations
            if(*(input + 2) == ' ' || *(input + 2) == '\t')
            {
                int skip = 3;
                for(; *(input + skip) == ' ' || *(input + skip) == '\t'; skip ++);
                
                *input = ' ';
                erase_chars(skip - 1, input + 1);
            }
            else break;
        }
    }
    return input;
}

static void normalize_headers(char * input)
{
    while(*input)
    {
        input = normalize_field_name(input);
        input = normalize_field_value(input);
    }
}

headers_buffer::headers_buffer(char * buffer)
 :m_headers(buffer),
  m_next_field(buffer)
{
    normalize_headers(m_headers);
}

bool headers_buffer::next_header_field(header_field & output)
{
    return output.parse(*this) == header_field::OK;
}

field_value_token::field_value_token()
 :m_type(0),
  m_start(NULL),
  m_last(NULL)
{
    
}

char field_value_token::get_type()const
{
    return m_type;
}

enum symbol_type
{
    SYMBOL_INVALID = 0,
    SYMBOL_WHITESPACE = 1,
    SYMBOL_SEPARATOR = 2,
    SYMBOL_ATOMCHAR = 3,
    SYMBOL_QUOTE = 4
};
/*
    Symbol map inspired by a similar one found in
        <http://mxr.mozilla.org/mozilla-central/source/netwerk/protocol/http/src/nsHttp.cpp?raw=1>
*/
static const char symbols[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, //   0
    0, 1, 0, 0, 0, 0, 0, 0, //   8
    0, 0, 0, 0, 0, 0, 0, 0, //  16
    0, 0, 0, 0, 0, 0, 0, 0, //  24

    1, 3, 4, 3, 3, 3, 3, 3, //  32
    2, 2, 3, 3, 2, 3, 3, 2, //  40
    3, 3, 3, 3, 3, 3, 3, 3, //  48
    3, 3, 2, 2, 2, 2, 2, 2, //  56

    2, 3, 3, 3, 3, 3, 3, 3, //  64
    3, 3, 3, 3, 3, 3, 3, 3, //  72
    3, 3, 3, 3, 3, 3, 3, 3, //  80
    3, 3, 3, 2, 2, 2, 3, 3, //  88

    3, 3, 3, 3, 3, 3, 3, 3, //  96
    3, 3, 3, 3, 3, 3, 3, 3, // 104
    3, 3, 3, 3, 3, 3, 3, 3, // 112
    3, 3, 3, 0, 3, 0, 3, 0  // 120
};

bool field_value_token::is_separator_type()const
{
    int type = symbols[static_cast<unsigned char>(m_type)];
    return type == SYMBOL_SEPARATOR || type == SYMBOL_WHITESPACE;
}

bool field_value_token::parse(const char ** input_output)
{
    m_type = **input_output;
    
    switch(symbols[static_cast<unsigned char>(m_type)])
    {
        case SYMBOL_WHITESPACE:
            
            m_start = *input_output;
            for(; **input_output ==' ' || **input_output == '\t'; (*input_output)++) 
                m_last = *input_output;
            return true;
        
        case SYMBOL_SEPARATOR:
            
            m_start = *input_output;
            m_last = m_start;
            (*input_output)++;
            return true;
        
        case SYMBOL_ATOMCHAR:

            m_type = 0;
            return parse_atom(input_output);
        
        case SYMBOL_QUOTE:
            
            (*input_output)++;
            m_start = *input_output;
            
            for(; **input_output !='\r' && **input_output != '\"'; (*input_output)++)
            {
                if(**input_output == '\\') **input_output++;
                m_last = *input_output;
            }
            
            if(**input_output != '\"') return false;
            
            break;
            
        case SYMBOL_INVALID:
        default:
            return false;
    }
    
    return true;
}

bool field_value_token::parse_atom(const char ** input_output)
{
    m_start = *input_output;
    m_last = m_start;
    (*input_output)++;
    for(; symbols[static_cast<unsigned char>(**input_output)] == SYMBOL_ATOMCHAR; (*input_output)++) m_last = *input_output;
    return true;
}

std::pair<const char *, const char *> field_value_token::get_content()const
{
    return field_value_string(m_start, m_last);
}

header_field::parse_error header_field::parse(headers_buffer & input)
{
    if(*input.m_next_field == '\r') return EMPTY;
    
    char * name_end = strstr(input.m_next_field, ":");
    if(!name_end) return MALFORMED;
    *name_end = '\0';
    
    m_name = input.m_next_field;
    m_value = name_end + 1;
    
    // Skip leading linear white space in field value
    for(; *m_value == ' ' || *m_value == '\t'; m_value++);
    
    m_next_value_token = m_value;
    
    // Set the start position for the next field
    input.m_next_field += m_value - input.m_next_field;
    for(; *input.m_next_field != '\r'; input.m_next_field++);
    input.m_next_field += 2;
    
    *(input.m_next_field - 2) = '\0';
    
    return OK;
}

const char * header_field::get_name()const
{
    return m_name;
}

const char * header_field::get_value()const
{
    return m_value;
}

bool header_field::next_value_token(field_value_token & output)
{
    return output.parse(&m_next_value_token);
}

field_value_token_stack::field_value_token_stack(header_field & field)
 :m_field(field)
{
    
}

const field_value_token * field_value_token_stack::next_value_token()
{
    if(m_tokens.empty())
    {
        field_value_token token;
        if(!m_field.next_value_token(token)) return NULL;
        m_tokens.push(token);
    }
    return &m_tokens.top();
}

void field_value_token_stack::pop()
{
    m_tokens.pop();
}

static const_string to_const_string(const std::pair<const char *, const char *> & input)
{
    return const_string(input.first, input.second);
}

static void assign_to_string(const std::pair<const char *, const char *> & input, std::string * output)
{
    output->assign(input.first, input.second + 1);
}

header_parameter::header_parameter(const field_value_token & key, const field_value_token & value)
 :m_key(key), m_value(value)
{
    
}

field_value_string header_parameter::name()const
{
    return m_key.get_content();
}

field_value_string header_parameter::value()const
{
    return m_value.get_content();
}

static void parse_parameters(field_value_token_stack & field_tokens, std::vector<header_parameter> & output)
{
    while(1)
    {
        const field_value_token * token = field_tokens.next_value_token();
        if(!token || token->get_type()!=';') return;
        field_tokens.pop();
        
        token = field_tokens.next_value_token();
        if(!token || token->get_type() != field_value_token::atom) return;
        field_value_token name = *token;
        field_tokens.pop();
        
        token = field_tokens.next_value_token();
        if(!token || token->get_type()!='=') return;
        field_tokens.pop();
        
        token = field_tokens.next_value_token();
        if(!token || token->get_type() != field_value_token::atom) return;
        
        output.push_back(header_parameter(name, *token));
        
        field_tokens.pop();
    }
}

static void skip_whitespaces(field_value_token_stack & field_tokens)
{
    while(1)
    {
        const field_value_token * token = field_tokens.next_value_token();
        if(!token || token->get_type() != field_value_token::linear_whitespace) return;
        field_tokens.pop();
    }
}

bool content_type::parse(field_value_token_stack & field_tokens)
{
    const field_value_token * token = field_tokens.next_value_token();
    if(!token || token->get_type() != field_value_token::atom) return false;
    m_type = *token;
    field_tokens.pop();

    token = field_tokens.next_value_token();
    if(!token || token->get_type() != '/') return false;
    field_tokens.pop();
    
    token = field_tokens.next_value_token();
    if(!token || token->get_type() != field_value_token::atom) return false;
    m_subtype = *token;
    field_tokens.pop();
    
    parse_parameters(field_tokens, m_parameters);
    
    return true;
}

field_value_string content_type::type()const
{
    return m_type.get_content();
}

field_value_string content_type::subtype()const
{
    return m_subtype.get_content();
}

const std::vector<header_parameter> & content_type::parameters()const
{
    return m_parameters;
}

bool parse_host(header_field & field, std::string * host, std::string * port)
{
    field_value_token value;
    if(!field.next_value_token(value) || value.get_type() != field_value_token::atom)
        return false;
    assign_to_string(value.get_content(), host);
    if(field.next_value_token(value))
    {
        if(value.get_type() != ':' || 
            field.next_value_token(value) || value.get_type() != field_value_token::atom) return false;
        assign_to_string(value.get_content(), port);
    }
    return true;
}

bool parse_content_length(header_field & field, std::size_t * content_length)
{
    field_value_token value;
    if(!field.next_value_token(value) || value.get_type() != field_value_token::atom)
        return false;
    try
    {
        *content_length = to_int<std::size_t>(to_const_string(value.get_content()));
    }
    catch(std::bad_cast)
    {
        return false;
    }
    return true;
}

bool parse_content_type(header_field & field, content_type & output)
{
    field_value_token_stack field_tokens(field);
    return output.parse(field_tokens);
}

bool parse_accept(header_field & field, std::vector<content_type> & output)
{
    field_value_token_stack field_tokens(field);
    
    while(1)
    {
        content_type ct;
        bool succeeded = ct.parse(field_tokens);
        if(!succeeded) return false;
        
        output.push_back(ct);
        
        const field_value_token * token = field_tokens.next_value_token();
        if(!token) return true;
        if(token->get_type() != ',') return false;
        field_tokens.pop();
        
        skip_whitespaces(field_tokens);
    }
}

bool parse_transfer_encoding(header_field & field, transfer_encoding_type & output)
{
    field_value_token token;
    if(!field.next_value_token(token) || token.get_type() != field_value_token::atom) return false;
    
    field_value_string value = token.get_content();
    
    const char * expected_type_string;
    std::size_t expected_type_stringlen;
    transfer_encoding_type expected_type;
    
    char first_char = tolower(value.first[0]);
    switch(first_char)
    {
        case 'c':
            
            switch(tolower(value.first[1]))
            {
                case 'h':
                    expected_type_string = "chunked";
                    expected_type_stringlen = 7; 
                    expected_type = CHUNKED;
                    break;
                case 'o':
                    expected_type_string = "compress";
                    expected_type_stringlen = 8;
                    expected_type = COMPRESS;
                    break;
                default: return false;
            }
            
            break;
        case 'i':
            expected_type_string = "identity";
            expected_type_stringlen = 8;
            expected_type = IDENTITY;
            break;
        case 'g':
            expected_type_string = "gzip";
            expected_type_stringlen = 4;
            expected_type = GZIP;
            break;
        case 'd':
            expected_type_string = "deflate";
            expected_type_stringlen = 7;
            expected_type = DEFLATE;
            break;
        default: return false;
    }
    
    std::size_t value_len = value.second - value.first + 1;
    
    if(!strncasecmp(value.first, expected_type_string, std::min(expected_type_stringlen, value_len)))
    {
        output = expected_type;
        return true;
    }
    else return false;
}

static int weekday(const char * weekday_string)
{
    static const char * names[] = 
    {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    
    for(int i = 0; i < sizeof(names)/sizeof(const char *); i++)
        if(strncasecmp(weekday_string, names[i], 3) == 0) return i;
    
    return -1;
}

static int month(const char * month_string)
{
    static const char * names[] =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    for(int i = 0; i < sizeof(names)/sizeof(const char *); i++)
        if(strncasecmp(month_string, names[i], 3) == 0) return i;
    
    return -1;
}

static int parse_int(const char ** date_string, char delim)
{
    const char * start = *date_string;
    const char * end = *date_string;
    
    for(; *end && *end != delim; end++);
    if(*end != delim) return -1;
    
    *date_string = end + 1;
    
    try
    {
        return to_int<int>(const_string(start, end - 1));
    }
    catch(std::bad_cast)
    {
        return -1;
    }
}

static bool parse_rfc1123_date(const char * date_string, time_t & output)
{
    tm time_info;
    memset(&time_info, 0, sizeof(time_info));
    
    time_info.tm_wday = weekday(date_string);
    if(time_info.tm_wday == -1) return false;
    
    for(date_string += 3; *date_string && *date_string != ','; date_string++);
    if(*date_string !=',' || *(date_string+1) !=' ') return false;
    date_string += 2;
    
    time_info.tm_mday = parse_int(&date_string, ' ');
    if(time_info.tm_mday == -1) return false;
    
    time_info.tm_mon = month(date_string);
    if(time_info.tm_mon == -1) return false;
    
    date_string += 3;
    if(*date_string != ' ') return false;
    date_string++;
    
    time_info.tm_year = parse_int(&date_string, ' ') - 1900;
    if(time_info.tm_year == -1) return false;
    
    time_info.tm_hour = parse_int(&date_string, ':');
    if(time_info.tm_hour == -1) return false;
    
    time_info.tm_min = parse_int(&date_string, ':');
    if(time_info.tm_min == -1) return false;
    
    time_info.tm_sec = parse_int(&date_string, ' ');
    if(time_info.tm_sec  == -1) return false;
    
    //bool has_gmt = *(date_string++) == 'G' && *(date_string++) == 'M' && *(date_string++) == 'T';
    
    output = mktime(&time_info);
    
    return true;
    //return has_gmt;
}

static bool parse_rfc850_date(const char * date_string, time_t & output)
{
    return false;
}

static bool parse_asctime_date(const char * date_string, time_t & output)
{
    return false;
}

time_t parse_date(const char * date_string)
{
    time_t output;
    if( !parse_rfc1123_date(date_string, output) &&
        !parse_rfc850_date(date_string, output) &&
        !parse_asctime_date(date_string, output)) return -1;
    return output;
}

} //namespace http
} //namespace fungu
