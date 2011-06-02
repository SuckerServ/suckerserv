/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_CLIENT_HPP
#define FUNGU_NET_HTTP_CLIENT_HPP

#include <string>
#include <boost/asio.hpp>

namespace fungu{
namespace http{

class client
{
public:
    typedef boost::system::error_code error_code;
    
    error_code connect(const std::string &, unsigned short);
    void disconnect();
    
    
    
private:
    boost::asio::ip::tcp::socket m_socket;
};

} //namespace http
} //namespace fungu

#endif
