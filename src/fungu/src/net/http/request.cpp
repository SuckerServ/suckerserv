/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/net/http/request.hpp"
#include "fungu/net/http/response.hpp"
#include <iostream>

namespace fungu{
namespace http{
namespace server{

static inline void delete_request(request * ptr)
{
    delete ptr;
}

static bool strncaseeq(const std::pair<const char*,const char*> & s1, const char * s2, std::size_t s2len)
{
    return strncasecmp(s1.first, s2, std::min(static_cast<std::size_t>(s1.second - s1.first + 1), s2len));
}

request * request::create(connection & conn, resource & handler_dispatcher)
{
    request * r = new request(conn, handler_dispatcher);
    return r;
}

void request::destroy(request & req)
{
    req.m_connection.io_service().post(boost::bind(delete_request, &req));
}

request::request(connection & conn, resource & root_resource)
 :m_connection(conn), 
  m_root_resource(root_resource),
  m_raw_headers(NULL),
  m_host(NULL),
  m_close_connection(false),
  m_expect_continue(false),
  m_client_has_message(false),
  m_transfer_encoding(IDENTITY),
  m_content_length(0)
{
    m_connection.async_read_header(boost::bind(&request::process_header, this, _1, _2, _3));
}

request::~request()
{
    delete [] m_raw_headers;
    if(m_close_connection || m_connection.has_connection_error() || !m_connection.is_open())
    {
        m_connection.close();
        m_finished_callback();
    }
    else request::create(m_connection, m_root_resource, m_finished_callback);
}

connection & request::get_connection()
{
    return m_connection;
}

bool request::expect_continue()const
{
    return m_expect_continue;
}

bool request::support_transfer_encoding(transfer_encoding_type encoding_type)const
{
    switch(encoding_type)
    {
        case IDENTITY:
            return true;
        case CHUNKED:
            return m_request_line.version_minor() > 0;
        default:
            return false;
    }
}

bool request::has_header_field(const char * name)const
{
    return m_headers.find(name) != m_headers.end();
}

header_field request::get_header_field(const char * name)const
{
    std::map<const char *, header_field, ltstr>::const_iterator it = m_headers.find(name);
    assert(it != m_headers.end());
    return it->second;
}

bool request::is_last_request()const
{
    return m_close_connection;
}

bool request::has_content()const
{
    return m_client_has_message;
}

std::size_t request::get_content_length()const
{
    return m_content_length;
}

content_type request::get_content_type()const
{
    return m_content_type;
}

const char * request::get_uri()const
{
    return m_request_line.uri();
}

const char * request::get_uri_query()const
{
    return m_request_line.query();
}

const char * request::get_host()const
{
    return m_host;
}

void request::process_header(const char * header, std::size_t header_len, const connection::error & error)
{
    if(error)
    {
        m_close_connection = true;
        request::destroy(*this);
        return;
    }
    
    m_raw_headers = new char[header_len + 1];
    strncpy(m_raw_headers, header, header_len);
    m_raw_headers[header_len] = '\0';
    
    char * cursor = m_raw_headers;
    
    switch(m_request_line.parse(&cursor))
    {
        case request_line::PARSE_UNKNOWN_METHOD:
            send_response(NOT_IMPLEMENTED, "unknown method");
            return;
        case request_line::PARSE_MISSING_URI:
        case request_line::PARSE_MALFORMED_VERSION:
            send_response(BAD_REQUEST);
            return;
        default:;
    }
    
    if(m_request_line.version_major() != 1 || m_request_line.version_minor() > 1)
    {
        send_response(HTTP_VERSION_NOT_SUPPORTED);
        return;
    }
    
    // HTTP/1.0 uses connection bursting as default behaviour
    m_close_connection = m_request_line.is_version_1_0();
    
    headers_buffer headers(cursor);
    
    header_field field;
    while(headers.next_header_field(field))
    {
        standard_headers field_code = resolve_header_field(field.get_name());
        
        switch(field_code)
        {
            case ACCEPT:
                if(!parse_accept(field, m_accept))
                {
                    send_response(BAD_REQUEST, "malformed Accept header field\n");
                    return;
                }
                break;
            case HOST:
            {
                field_value_token host;
                if(!field.next_value_token(host) || host.get_type() != field_value_token::atom)
                {
                    send_response(BAD_REQUEST, "malformed host header field\n");
                    return;
                }
                m_host = field.get_value();
                break;
            }
            case CONNECTION:
            {
                m_close_connection = strcasecmp(field.get_value(), "close") == 0;
                // in case of Keep-Alive value: value != "close" so m_close_connection would be assigned false value
                break;
            }
            case CONTENT_TYPE:
                if(!parse_content_type(field, m_content_type))
                {
                    send_response(BAD_REQUEST, "malformed Content-Type header field\n");
                    return;
                }
                break;
            case CONTENT_LENGTH:
                if(!parse_content_length(field, &m_content_length))
                {
                    send_response(BAD_REQUEST, "malformed Content-Length header field\n");
                    return;
                }
                m_client_has_message = true;
                break;
            case TRANSFER_ENCODING:
                if(!parse_transfer_encoding(field, m_transfer_encoding))
                {
                    send_response(BAD_REQUEST, "malformed Transfer-Encoding header field\n");
                    return;
                }
                m_client_has_message = true;
                break;
            case EXPECT:
            {
                field_value_token token;
                if(!field.next_value_token(token) || token.get_type() != field_value_token::atom)
                {
                    send_response(BAD_REQUEST, "malformed Expect header field\n");
                    return;
                }
                
                m_expect_continue = strncaseeq(token.get_content(), "100-continue", 12);
                
                if(!m_expect_continue)
                {
                    m_close_connection = true;
                    send_response(EXPECTATION_FAILED);
                    return;
                }
                
                break;
            }
            default:
                break;
        }
        
        m_headers[field.get_name()] = field;
    }
    
    if(m_client_has_message)
    {
        method_code m = m_request_line.method();
        if(m != POST && m != PUT)
        {
            m_close_connection = true;
            send_response(NOT_IMPLEMENTED, "will only accept message body for POST and PUT requests\n");
            return;
        }
    }

    if(!m_host)
    {
        send_response(BAD_REQUEST, "missing host field\n");
        return;
    }
    
    resource * resource_handler = resolve_resource(m_request_line.uri());
    
    if(!resource_handler)
    {
        send_response(NOT_FOUND, "resource not found\n");
        return;
    }
    
    dispatch_method_handler(resource_handler);
}

resource * request::resolve_resource(const char * uri)
{
    assert(*uri=='/');
    
    resource * parent = &m_root_resource;
    const char * start = uri + 1;
    
    for(const char * c = start; *c; c++)
    {
        if(*c == '/' || !*c || !*(c+1))
        {
            const char * last = (*c=='/' ? c-1 : c);
            if(*start == '.' && *last == '.') // ignore /./ and /../
            {
                start = (start == last ? last + 1 : last + 2);
                continue;
            }
            
            char normalized_filename[255];
            std::size_t normalized_filename_len = sizeof(normalized_filename);
            pct_decode(start, last + 1, normalized_filename, &normalized_filename_len);
            
            parent = parent->resolve(const_string(normalized_filename, normalized_filename + normalized_filename_len - 1));
            if(!parent) return NULL;
            
            start = last + 2;
        }
    }
    
    return parent;
}

void request::dispatch_method_handler(resource * resource_handler)
{
    switch(m_request_line.method())
    {
        case GET:
            resource_handler->get_method(*this);
            break;
        case PUT:
            resource_handler->put_method(*this);
            break;
        case POST:
            resource_handler->post_method(*this);
            break;
        case DELETE:
            resource_handler->delete_method(*this);
            break;
        default:
        {
            m_close_connection = true;
            
            response * res = new response(*this, METHOD_NOT_ALLOWED);
            res->add_header_field("Allow", "GET, PUT, POST, DELETE");
            res->set_content_length(0);
            res->send_header();
        }
    }
}

void request::send_response(status status_code, const char * content)
{
    response * res = new response(*this, status_code);
    res->add_header_field("Content-Type", "text/plain");
    
    std::size_t content_length = strlen(content);
    res->set_content_length(content_length);
    
    res->send_header();
    res->send_body(content, content_length);
}

void request::send_response(status status_code)
{
    response * res = new response(*this, status_code);
    res->set_content_length(0);
    res->send_header();
}

} //namespace server
} //namespace http
} //namespace fungu
