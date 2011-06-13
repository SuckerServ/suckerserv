/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/net/http/status_line.hpp"

namespace fungu{
namespace http{

static inline bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

bool status_line::parse(cursor_type cursor)
{
    char * input = *cursor;
    
    bool valid_version = input[0] == 'H' && input[1] == 'T' && input[2] == 'T' && input[3] == 'P'  && 
        input[4] == '/' && is_digit(input[5]) && input[6] == '.' && is_digit(input[7]);
    if(!valid_version) return false;
    
    m_version_major = input[5];
    m_version_minor = input[7];
    
    bool valid_code = input[8] == ' ' && is_digit(input[9]) && is_digit(input[10]) && is_digit(input[11]);
    if(!valid_code) return false;
    
    m_status_code = static_cast<status>(((input[9] - '0') * 100) + ((input[10] - '0') * 10) + (input[11] - '0'));
    
    bool valid_reason_phrase = input[12] == ' ';
    if(!valid_reason_phrase) return false;
    
    m_reason_phrase = &input[13];
    
    input = &input[13];
    for(; *input != '\r'; input++);
    if(input[1] != '\n') return false;
    
    input[0] = '\0';
    
    input += 2;
    *cursor = input;
    
    return true;
}

bool status_line::is_version_1_0()const
{
    return m_version_major == '1' && m_version_minor == '0';
}

bool status_line::is_version_1_1()const
{
    return m_version_major == '1' && m_version_minor == '1';
}

status status_line::get_status_code()const
{
    return m_status_code;
}

const char * status_line::get_reason_phrase()const
{
    return m_reason_phrase;
}

} //namespace http
} //namespace fungu
