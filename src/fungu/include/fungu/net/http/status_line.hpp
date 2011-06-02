/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_STATUS_LINE_HPP
#define FUNGU_NET_HTTP_STATUS_LINE_HPP

#include "status.hpp"

namespace fungu{
namespace http{

class status_line
{
public:
    typedef char ** cursor_type;
    
    bool parse(cursor_type);
    
    bool is_version_1_0()const;
    bool is_version_1_1()const;
    
    const char * get_reason_phrase()const;
    status get_status_code()const;
private:
    status m_status_code;
    const char * m_reason_phrase;
    char m_version_major;
    char m_version_minor;
};

} //namespace http
} //namespace fungu

#endif
