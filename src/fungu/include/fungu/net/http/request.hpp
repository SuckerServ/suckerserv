/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_REQUEST_HPP
#define FUNGU_NET_HTTP_REQUEST_HPP

#include "../../string.hpp"
#include "connection.hpp"
#include "request_line.hpp"
#include "header.hpp"
#include "info.hpp"
#include "status.hpp"
#include <map>
#include <functional>

struct ltstr
{
  bool operator()(const char* s1, const char* s2)const
  {
    return strcmp(s1, s2) < 0;
  }
};

namespace fungu{
namespace http{
namespace server{

class request;
class resource_resolver;

class resource
{
public:
    virtual resource * resolve(const const_string &)=0;
    
    virtual void get_method(request &)=0;
    virtual void put_method(request &)=0;
    virtual void post_method(request &)=0;
    virtual void delete_method(request &)=0;
};

class request
{
public:
    template<typename FinishedCallbackFunction>
    static request * create(connection & con, resource & res, FinishedCallbackFunction callback)
    {
        request * req = create(con, res);
        req->m_finished_callback = callback;
        return req;
    }
    
    static void destroy(request &);
    ~request();
    connection & get_connection();
    bool expect_continue()const;
    bool support_transfer_encoding(transfer_encoding_type)const;
    
    bool has_header_field(const char *)const;
    header_field get_header_field(const char *)const;
    
    bool is_last_request()const;
    
    bool has_content()const;
    std::size_t get_content_length()const;
    content_type get_content_type()const;
    
    const char * get_uri()const;
    const char * get_uri_query()const;
    const char * get_host()const;

    template<typename CompletionHandler>
    void async_read_content(stream::sink & output, CompletionHandler handler)
    {
        if(m_transfer_encoding == CHUNKED) m_connection.async_read_chunked_body(output, handler);
        else m_connection.async_read_body(m_content_length, output, handler);
    }
private:
    static request * create(connection &, resource &);
    
    request(connection &, resource &);    
    void process_header(const char *, std::size_t, const connection::error &);
    resource * resolve_resource(const char *);
    void dispatch_method_handler(resource *);
    void send_response(status, const char *);
    void send_response(status);
    
    connection & m_connection;
    resource & m_root_resource;
    
    request_line m_request_line;

    char * m_raw_headers;
    std::map<const char *, header_field, ltstr> m_headers;

    std::vector<content_type> m_accept;
    const char * m_host;
    
    bool m_close_connection;
    bool m_expect_continue;
    bool m_client_has_message;
    
    transfer_encoding_type m_transfer_encoding;
    std::size_t m_content_length;
    content_type m_content_type;
    
    std::function<void()> m_finished_callback;
};

} //namespace server
} //namespace http
} //namespace fungu

#endif
