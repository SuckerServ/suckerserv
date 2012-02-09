/*   
 *   The Fungu Network Library
 *   
 *   Copyright (c) 2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_NET_HTTP_CONNECTION_HPP
#define FUNGU_NET_HTTP_CONNECTION_HPP

#include "../../streams.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/bind/protect.hpp>
#include <boost/bind/make_adaptable.hpp>
#include <queue>
#include <cstdio>

namespace fungu{
namespace http{

class connection
{
public:
    typedef unsigned short port_type;
    
    class error
    {
    public:
        enum error_type
        {
            NONE = 0,
            BUFFER,
            NETWORK,
            PROTOCOL
        };
        error():m_value(NONE){}
        error(error_type value):m_value(value){}
        error(error_type value, const std::string & message):m_value(value), m_message(message){}
        error_type type()const{return m_value;}
        operator bool()const{return m_value != NONE;}
        const std::string & message()const{return m_message;}
    private:
        error_type m_value;
        std::string m_message;
    };
    
    connection(boost::asio::ip::tcp::socket &);

    template<typename ReadHandler>
    void async_read_header(ReadHandler handler)
    {
        boost::asio::socket_base::receive_buffer_size option;
        m_socket.get_option(option);
        m_receive_buffer_size = option.value();
        
        boost::asio::async_read_until(m_socket, m_read_buffer, "\r\n\r\n", bind_io_handler(&connection::read_header_complete<ReadHandler>, handler));
    }
    
    template<typename CompletionHandler>
    void async_read_body(std::size_t content_length, stream::sink & output, CompletionHandler handler)
    {
        consume_header();
        boost::system::error_code noerror;
        read_body_complete(0, content_length, output, boost::make_adaptable<void, const error &>(handler), noerror, 0);
    }
    
    template<typename CompletionHandler>
    void async_read_chunked_body(stream::sink & output, CompletionHandler handler)
    {
        consume_header();
        _async_read_chunked_body(output, boost::make_adaptable<void, const error &>(handler));
    }
    
    template<typename CompletionHandler>
    void async_send(const std::string & data, CompletionHandler handler)
    {
        boost::asio::async_write(m_socket, boost::asio::buffer(data), boost::asio::transfer_at_least(data.length()), bind_io_handler(&connection::send_complete<CompletionHandler>, handler));
    }
    
    template<typename CompletionHandler>
    void async_send(const void * data, std::size_t datalen, CompletionHandler handler)
    {
        boost::asio::async_write(m_socket, boost::asio::buffer(data, datalen), boost::asio::transfer_at_least(datalen), bind_io_handler(&connection::send_complete<CompletionHandler>, handler));
    }
    
    template<typename CompletionHandler>
    void async_send_chunk(const void * data, std::size_t datalen, CompletionHandler handler)
    {
        char buffer[32];
        std::sprintf(buffer, "%x\r\n", static_cast<unsigned int>(datalen));
        
        m_sent_chunk_headers.push(buffer);
        
        std::vector<boost::asio::const_buffer> chunk;
        chunk.push_back(boost::asio::buffer(m_sent_chunk_headers.back()));
        if(datalen) chunk.push_back(boost::asio::buffer(data, datalen));        
        chunk.push_back(boost::asio::buffer("\r\n", 2));
        
        m_socket.async_send(chunk, bind_io_handler(&connection::send_chunk_complete<CompletionHandler>, handler));
    }
    
    void close();
    
    port_type remote_port()const;
    std::string remote_ip_string()const;
    unsigned long remote_ip_v4_ulong()const;
    
    boost::asio::io_service & io_service()
    {
        return m_socket.get_io_service();
    }
    
    bool has_connection_error()const
    {
        return m_read_error || m_send_error;
    }
    
    bool is_open()const
    {
        return m_socket.is_open();   
    }
private:
    template<typename MemberFunction, typename CompletionHandler>
    class io_handler_binder
    {
    public:
        io_handler_binder(MemberFunction ih, connection * c, CompletionHandler eh)
         :m_this(c), m_internal_handler(ih), m_external_handler(eh){}
        void operator()(const boost::system::error_code & error_code, const std::size_t bytes_transferred)
        {
            (m_this->*m_internal_handler)(m_external_handler, error_code, bytes_transferred);
        }
    private:
        connection * m_this;
        MemberFunction m_internal_handler;
        CompletionHandler m_external_handler;
    };
    
    template<typename MemberFunction, typename CompletionHandler>
    io_handler_binder<MemberFunction, CompletionHandler> bind_io_handler(MemberFunction internal_handler, CompletionHandler external_handler)
    {
        return io_handler_binder<MemberFunction, CompletionHandler>(internal_handler, this, external_handler);
    }
    
    template<typename ReadHandler>
    void read_header_complete(ReadHandler handler, const boost::system::error_code error_code, const std::size_t readSize)
    {
        m_read_error = error_code;
        
        m_unconsumed_header_size = readSize;
        
        if(error_code)
        {
            handler(static_cast<const char * >(NULL), 0, error(error::NETWORK, error_code.message()));
        }
        else
        {            
            handler(boost::asio::buffer_cast<const char *>(*m_read_buffer.data().begin()), readSize, error());
        }
        
        consume_header();
    }
    
    template<typename CompletionHandler>
    void read_body_complete(std::size_t completed, std::size_t contentLength, stream::sink & output, CompletionHandler finishedHandler, const boost::system::error_code error_code, const std::size_t readSize)
    {
        if(error_code)
        {
            m_read_error = error_code;
            m_socket.get_io_service().post(boost::bind(finishedHandler, error(error::NETWORK, error_code.message())));
            return;
        }
        
        if(readSize)
        {
            output.write(boost::asio::buffer_cast<const char *>(*m_read_buffer.data().begin()), readSize);
            m_read_buffer.consume(readSize);
        }
        else if(m_read_buffer.size()) //read what was left over from when the header was read
        {
            read_body_complete(0, contentLength, output, finishedHandler, error_code, std::min(m_read_buffer.size(), contentLength));
            return;
        }
        
        completed += readSize;

        std::size_t remaining = contentLength - completed;
        if(!remaining)
        {
            m_socket.get_io_service().post(boost::bind(finishedHandler, error()));
            return;
        }

        boost::asio::async_read(m_socket, m_read_buffer, 
            boost::asio::transfer_at_least(std::min(remaining, m_receive_buffer_size)), 
            boost::bind(&connection::read_body_complete<CompletionHandler>, this, completed, contentLength, boost::ref(output), finishedHandler, _1, _2));
    }
    
    static bool parse_chunk_size(const char *, const char *, std::size_t *);
    
    template<typename CompletionHandler>
    void _async_read_chunked_body(stream::sink & output, CompletionHandler handler)
    {
        boost::asio::async_read_until(m_socket, m_read_buffer, "\r\n", boost::bind(&connection::read_chunk_header<CompletionHandler>, this, &output, handler, _1, _2));
    }
    
    template<typename CompletionHandler>
    void read_chunk_header(stream::sink * output, CompletionHandler handler, const boost::system::error_code & error_code, const std::size_t readSize)
    {
        if(error_code)
        {
            m_read_error = error_code;
            handler(error(error::NETWORK, error_code.message()));
            return;
        }
        
        const char * line = boost::asio::buffer_cast<const char *>(*m_read_buffer.data().begin());
        std::size_t chunk_size;
        
        if(!parse_chunk_size(line, line + readSize, &chunk_size))
        {
            handler(error(error::PROTOCOL));
            return;
        }
        
        m_read_buffer.consume(readSize);
        
        if(chunk_size)
        {
            boost::system::error_code noerror;
            read_chunk(output, handler, chunk_size, noerror, 0); 
        }
        else
        {
            boost::asio::async_read_until(m_socket, m_read_buffer, "\r\n\r\n", 
                boost::bind(&connection::read_trailer<CompletionHandler>, this, handler, _1, _2));
        }
    }
    
    template<typename CompletionHandler>
    void read_chunk(stream::sink * output, CompletionHandler handler, std::size_t remaining, const boost::system::error_code & error_code, const std::size_t readSize)
    {
        if(error_code)
        {
            m_read_error = error_code;
            handler(error(error::NETWORK, error_code.message()));
            return;
        }
        
        if(readSize)
        {
            const char * data = boost::asio::buffer_cast<const char *>(*m_read_buffer.data().begin());
            output->write(data, readSize);
            m_read_buffer.consume(readSize);
        }
        
        remaining -= readSize;
        
        if(remaining)
        {
            boost::asio::async_read(m_socket, m_read_buffer,
                boost::asio::transfer_at_least(std::min(remaining, m_receive_buffer_size)),
                boost::bind(&connection::read_chunk<CompletionHandler>, this, output, handler, remaining, _1, _2));
        }
        else
        {
            boost::asio::async_read_until(m_socket, m_read_buffer, "\r\n", 
                boost::bind(&connection::read_end_chunk_data<CompletionHandler>, this, output, handler, _1, _2));
        }
    }
    
    template<typename CompletionHandler>
    void read_end_chunk_data(stream::sink * output, CompletionHandler handler, const boost::system::error_code & error_code, const std::size_t readSize)
    {
        if(error_code)
        {
            m_read_error = error_code;
            handler(error(error::NETWORK, error_code.message()));
            return;
        }
        
        _async_read_chunked_body(*output, handler);
    }
    
    template<typename CompletionHandler>
    void read_trailer(CompletionHandler handler, const boost::system::error_code & error_code, const std::size_t readSize)
    {
        if(error_code)
        {
            m_read_error = error_code;
            handler(error(error::NETWORK, error_code.message()));
            return;
        }
        
        //ignore headers
        
        handler(error(error::NONE));
    }
    
    template<typename CompletionHandler>
    void send_complete(CompletionHandler handler, const boost::system::error_code & error_code, std::size_t bytes_sent)
    {
        if(error_code)
        {
            m_send_error = error_code;
            handler(error(error::NETWORK, error_code.message()));
            return;
        }
        
        handler(error(error::NONE));
    }
    
    template<typename CompletionHandler>
    void send_chunk_complete(CompletionHandler handler, const boost::system::error_code & error_code, std::size_t bytes_sent)
    {
        m_sent_chunk_headers.pop();
        
        if(error_code)
        {
            m_send_error = error_code;
            handler(error(error::NETWORK, error_code.message()));
            return;
        }
        
        handler(error(error::NONE));
    }
    
    void consume_header()
    {
        m_read_buffer.consume(m_unconsumed_header_size);
        m_unconsumed_header_size = 0;
    }
    
    boost::asio::streambuf m_read_buffer;
    std::size_t m_unconsumed_header_size;
    boost::system::error_code m_read_error;
    boost::system::error_code m_send_error;
    boost::asio::ip::tcp::socket & m_socket;
    std::size_t m_receive_buffer_size;
    std::size_t m_send_buffer_size;
    
    std::queue<std::string> m_sent_chunk_headers;
};

namespace server{
class client_connection:public boost::asio::ip::tcp::socket, public connection
{
public:
    client_connection(boost::asio::io_service & service)
     :boost::asio::ip::tcp::socket(service), connection(*static_cast<boost::asio::ip::tcp::socket *>(this)){}
};
} //namespace server

} //namespace http
} //namespace fungu

#endif
