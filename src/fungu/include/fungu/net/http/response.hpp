/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_RESPONSE_HPP
#define FUNGU_NET_HTTP_RESPONSE_HPP

#include "request.hpp"

namespace fungu{
namespace http{
namespace server{

class response
{
public:
    response(request &, status, bool keep_request = false);
    ~response();
    
    void add_header_field(const char *, const char *);

    void use_chunked_encoding();
    
    void set_content_length(std::size_t);
    std::size_t get_content_length()const;

    void send_header();
    
    template<typename CompletionHandler>
    void async_send_header(CompletionHandler handler)
    {
        m_connection.async_send(m_header, handler);
    }
    
    template<typename CompletionHandler>
    void async_send_body(const void * data, std::size_t datalen, CompletionHandler handler)
    {
        if(m_using_chunked_encoding)
        {
            m_connection.async_send_chunk(data, datalen, handler);
        }
        else
        {
            assert(m_bytes_sent <= m_content_length);
            m_connection.async_send(data, datalen, handler);
        #ifndef NDEBUG
            m_bytes_sent += datalen;
        #endif
        }
    }
    
    void send_body(const char * content, std::size_t content_length);
private:
    void send_header_complete(const connection::error &);
    static void sent_body(response * res, char * data, const http::connection::error & err);
    
    request & m_request;
    bool m_keep_request;
    
    connection & m_connection;
    status m_status;
    std::string m_header;

    bool m_using_chunked_encoding;
    std::size_t m_content_length;
    
#ifndef NBEBUG
    std::size_t m_bytes_sent;
    bool m_sent_header;
#endif
};

void send_response(request &, status, const char *);
void send_response(request &, status);

} //namespace server
} //namespace http
} //namespace fungu

#endif
