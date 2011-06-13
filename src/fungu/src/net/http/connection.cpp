/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#include "fungu/net/http/connection.hpp"
#include <cmath>
#include <cstring>
using namespace boost::asio;

static int strpos(const char * body, char c)
{
    int count = 0;
    for(; *body && *body != c; body++, count++);
    return count;
}

static bool from_hex(const char * start, const char * end, std::size_t * output)
{
    static const char * digitset = "0123456789ABCDEF";
    std::size_t result = 0;
    std::size_t place_value = static_cast<std::size_t>(pow(16, end - start));
    
    for(const char * c = start; c <= end; c++, place_value/=16)
    {
        int digit = strpos(digitset, toupper(*c));
        if(digit > 15) return false;
        result += place_value * digit;
    }
    
    *output = result;
    return true;
}

namespace fungu{
namespace http{

connection::connection(boost::asio::ip::tcp::socket & socket)
 :m_socket(socket)
{

}

void connection::close()
{
    m_socket.close();
}

connection::port_type connection::remote_port()const
{
    return m_socket.remote_endpoint().port();
}

std::string connection::remote_ip_string()const
{
    return m_socket.remote_endpoint().address().to_string();
}

unsigned long connection::remote_ip_v4_ulong()const
{
    assert(m_socket.remote_endpoint().address().is_v4());
    return m_socket.remote_endpoint().address().to_v4().to_ulong();
}

bool connection::parse_chunk_size(const char * start, const char * end, std::size_t * output)
{
    const char * size_end;
    for(size_end = start; size_end < end && *size_end !=';'; size_end++);
    size_end--;
    return from_hex(start, size_end, output);
}

} //namespace http
} //namespace fungu
