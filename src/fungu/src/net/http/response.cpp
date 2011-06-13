/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/net/http/response.hpp"
#include "fungu/convert.hpp"
#include <sstream>
#include <ctime>
#include <iostream>

namespace fungu{
namespace http{
namespace server{

static const char * datetime_now(char * buffer, std::size_t buffersize)
{
    time_t timestamp = time(NULL);
    const tm * current_gmtime = gmtime(&timestamp);
    
    if(!current_gmtime)
    {
        buffer[0] = '\0';
        return buffer;
    }
    
    strftime(buffer, buffersize, "%a, %d %b %Y %T GMT", current_gmtime);
    return buffer;
}

response::response(request & req, status code, bool keep_request)
 :m_request(req),
  m_keep_request(keep_request),          // to allow for multiple responses (in case of sending a continue status response)
  m_connection(req.get_connection()), 
  m_status(code),
  m_using_chunked_encoding(false),
  m_content_length(0)
#ifndef NDEBUG
  , m_bytes_sent(0)
  , m_sent_header(true) //set to false when send_header() is called
#endif
{
    m_header  = "HTTP/1.1 ";
    m_header += to_string(m_status);
    m_header += " ";
    m_header += get_reason_phrase(m_status);
    m_header += "\r\n";
    
    char date_buffer[32];
    add_header_field("Date", datetime_now(date_buffer, sizeof(date_buffer)));
}

response::~response()
{
    assert(m_sent_header);
    if(!m_keep_request) request::destroy(m_request);
}

void response::add_header_field(const char * name, const char * value)
{
    std::size_t name_start = m_header.length();
    
    m_header.append(name);
    m_header.append(": ");
    m_header.append(value);
    m_header.append("\r\n");
    
    m_header[name_start] = toupper(m_header[name_start]);
}

void response::use_chunked_encoding()
{
    m_using_chunked_encoding = true;
    add_header_field("Transfer-Encoding", "chunked");
}

void response::set_content_length(std::size_t bytes_length)
{
    m_content_length = bytes_length;
    add_header_field("Content-Length", from_int(bytes_length).begin());
}

std::size_t response::get_content_length()const
{
    return m_content_length;
}

static void empty_handler(const connection::error &){}

void response::send_header()
{
    if(m_request.is_last_request() && !m_keep_request) 
        add_header_field("Connection", "close");
    
    m_header += "\r\n";
    
#ifndef NDEBUG
    m_sent_header = false;
#endif
    
    m_connection.async_send(m_header, boost::bind(&response::send_header_complete, this, _1));
}

void response::send_header_complete(const connection::error &)
{
#ifndef NDEBUG
    m_sent_header = true;
#endif
    
    if(!m_content_length && !m_using_chunked_encoding) delete this;
}

void response::sent_body(response * res, char * data, const http::connection::error & err)
{
    delete [] data;
    delete res;
}

void response::send_body(const char * content, std::size_t content_length)
{
    char * content_copy = new char[content_length];
    memcpy(content_copy, content, content_length);
    async_send_body(content_copy, content_length, boost::bind(&response::sent_body, this, content_copy, _1));
}

void send_response(request & req, status status_code, const char * content)
{
    response * res = new response(req, status_code);
    res->add_header_field("Content-Type", "text/plain");
    
    std::size_t content_length = strlen(content);
    res->set_content_length(content_length);
    
    res->send_header();
    res->send_body(content, content_length);
}

void send_response(request & req, status status_code)
{
    response * res = new response(req, status_code);
    res->set_content_length(0);
    res->send_header();
}

} //namespace server
} //namespace http
} //namespace fungu
