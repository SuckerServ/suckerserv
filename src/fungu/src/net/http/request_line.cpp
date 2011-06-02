/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/net/http/request_line.hpp"
#include <strings.h>

static inline bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

namespace fungu{
namespace http{
    
request_line::request_line()
 :m_uri(NULL), m_query(NULL)
{
    
}

/**
    @param cursor
    @return status code
    
    Pre-conditions:
        * String terminated by \r\n\0

    Post-conditions:
        * String buffer within the request line range remain allocated and immutable.
        
*/
request_line::parse_status request_line::parse(cursor_type cursor)
{
    m_method = parse_method(cursor);
    if(m_method == UNKNOWN) return PARSE_UNKNOWN_METHOD;
    
    char * input = *cursor;

    if(*input++ != ' ') return PARSE_MISSING_URI;
    
    m_uri = input;
    for(; *input != ' ' && *input !='?' && *input; input++);
    if(!(input - m_uri)) return PARSE_MISSING_URI;
    
    char * end_of_uri = *cursor + (input - *cursor);
    char * end_of_query = end_of_uri;
    
    if(*end_of_uri == '?')
    {
        m_query = end_of_uri + 1;
        for(; *input != ' ' && *input; input++);
        if(!(input - m_uri)) return PARSE_MISSING_URI;
        end_of_query = *cursor + (input - *cursor);
    }
    
    bool valid_version = input[0] == ' ' && 
        input[1] == 'H' && input[2] == 'T' && input[3] == 'T' && input[4] == 'P' && input[5] == '/' &&
        is_digit(input[6]) && input[7] == '.' && is_digit(input[8]);
    
    if(!valid_version || input[9] != '\r' || input[10] != '\n') return PARSE_MALFORMED_VERSION;
    
    m_version = (input[6] - '0') * 10 + (input[8] - '0');
    
    *cursor = input + 11;
    *end_of_uri = '\0';
    *end_of_query = '\0';
    
    return PARSE_OK;
}

method_code request_line::method()const
{
    return m_method;
}

const char *  request_line::uri()const
{
    return m_uri;
}

const char * request_line::query()const
{
    return m_query;
}

int request_line::version_major()const
{
    return m_version / 10;
}

int request_line::version_minor()const
{
    return m_version % 10;
}

bool request_line::is_version_1_0()const
{
    return version_major() == 1 && version_minor() == 0;
}

bool request_line::is_version_1_1()const
{
    return m_version % 10 == 1 && m_version / 10 == 1;
}

method_code request_line::parse_method(cursor_type cursor)
{
    char * input = *cursor;
    
    const char * wanted_method = NULL;
    method_code wanted_method_code;
    int wanted_method_len = 0;
    
    switch(toupper(*input))
    {
        case 'O':
            wanted_method = "OPTIONS";
            wanted_method_len = 7;
            wanted_method_code = OPTIONS;
            break;
        
        case 'G':
            wanted_method = "GET";
            wanted_method_len = 3;
            wanted_method_code = GET;
            break;
        
        case 'H':
            wanted_method = "HEAD";
            wanted_method_len = 4;
            wanted_method_code = HEAD;
            break;
        
        case 'P':
            
            switch(toupper(*(input+1)))
            {
                case 'O':
                    wanted_method = "POST";
                    wanted_method_len = 4;
                    wanted_method_code = POST;
                    break;
                
                case 'U':
                    wanted_method = "PUT";
                    wanted_method_len = 3;
                    wanted_method_code = PUT;
                    break;
                
                default: break;
            }
            
            break;
        
        case 'D':
            wanted_method = "DELETE";
            wanted_method_len = 6;
            wanted_method_code = DELETE;
            break;
        
        case 'T':
            wanted_method = "TRACE";
            wanted_method_len = 5;
            wanted_method_code = TRACE;
            break;

        case 'C':
            wanted_method = "CONNECT";
            wanted_method_len = 7;
            wanted_method_code = CONNECT;
            break;
        
        default: break;
    }
    
    if(wanted_method && strncasecmp(input, wanted_method, wanted_method_len) == 0)
    {
        *cursor = input + wanted_method_len;
        return wanted_method_code;
    }
    else return UNKNOWN;
}

static int hex_digit(char c)
{
    if(c >= '0' && c <='9') return c - '0';
    else if(c >= 'A' && c <= 'F') return (c - 'A') + 10;
    return -1;
}

static bool is_unreserved(char c)
{
    if( c >= 'a' && c <= 'z') return true;
    if( c >= 'A' && c <= 'Z') return true;
    if( c >= '0' && c <= '9') return true;
    switch(c)
    {
        case '-':
        case '.':
        case '_':
        case '~':
            return true;
        default: break;
    }
    return false;
}

char * pct_decode(const char * input, const char * input_end, char * output, std::size_t * maxlen)
{
    (*maxlen)--; //reserve one char for null-term
    
    char * start_of_output = output;
    char * output_limit = output + *maxlen;
    
    for(; input < input_end && output < output_limit; input++, output++)
    {
        if(*input != '%') *output = *input;
        else
        {
            if(!*(input + 1) || !*(input + 2)) continue;
            int a = hex_digit(toupper(*(input + 1)));
            int b = hex_digit(toupper(*(input + 2)));
            if(a == -1 || b == -1) continue;
            *output = static_cast<char>(a * 16 + b);
            input += 2;
        }
    }
    
    *output = '\0';
    *maxlen = output - start_of_output;
    
    return start_of_output;
}

char * pct_encode(const char * input, const char * input_end, char * output, std::size_t * maxlen)
{
    (*maxlen)--;
    
    char * start_of_output = output;
    char * output_limit = output + *maxlen;
    
    for(; input < input_end && output < output_limit; input++, output++)
    {
        if(is_unreserved(*input)) *output = *input;
        else
        {
            if(output + 3 > output_limit) break;
            
            *output = '%';
            output++;
            
            static const char * digits = "0123456789ABCDEF";
            
            *output = digits[*input / 16];
            output++;
            
            *output = digits[*input % 16];
        }
    }
    
    *output = '\0';
    *maxlen = output - start_of_output;
    
    return start_of_output;
}

} //namespace http
} //namespace fungu
