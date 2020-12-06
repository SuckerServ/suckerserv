#ifndef HOPMOD_LUA_NET_READ_STREAM_BUFFER_HPP
#define HOPMOD_LUA_NET_READ_STREAM_BUFFER_HPP

#include "utils/protect_wrapper.hpp"
#include <asio/error.hpp>
#include <asio/read.hpp>
#include <asio/read_until.hpp>

#include <iostream>

template<typename StreamBufferClass, typename PodType>
class read_stream_buffer
{
public:
    read_stream_buffer(StreamBufferClass & buffer)
    :m_buffer(buffer), m_consume(0), m_read_lock(false)
    {
        
    }
    
    template<typename AsyncReadStream, typename ReadHandler>
    void async_read(AsyncReadStream & stream, std::size_t read_size, ReadHandler handler)
    {
        if(!lock_read(stream, handler)) return;
        
        if(read_size < m_buffer.size())
        {
            post(stream.get_executor(), std::bind(&read_stream_buffer<StreamBufferClass, PodType>::template read_complete<ReadHandler>,
               this, read_size, read_size, std::error_code(), 0, handler));
            return;
        }
        
        asio::async_read(stream, m_buffer, asio::transfer_at_least(
            read_size - m_buffer.size()), 
            std::bind(&read_stream_buffer::template read_complete<ReadHandler>, this, 
            m_buffer.size(), read_size, std::placeholders::_1, std::placeholders::_2, handler));
    }
    
    template<typename AsyncReadStream, typename ReadHandler>
    void async_read_until(AsyncReadStream & stream, const std::string & delim, ReadHandler handler)
    {
        if(!lock_read(stream, handler)) return;
        asio::async_read_until(stream, m_buffer, delim, std::bind(
            &read_stream_buffer::template read_complete<ReadHandler>, this, 0, -1, std::placeholders::_1, std::placeholders::_2, handler));
    }
    
    void unlock_read()
    {
        m_buffer.consume(m_consume);
        m_read_lock = false;
        m_consume = 0;
    }
private:
    template<typename IoObject, typename ReadHandler>
    bool lock_read(IoObject & object, ReadHandler handler)
    {
        if(m_read_lock)
        {
            post(object.get_executor(), std::bind(handler, asio::error::already_started, static_cast<const char *>(NULL), 0));
            return false;
        }
        m_read_lock = true;
        return true;
    }
    
    template<typename ReadHandler>
    void read_complete(std::size_t bytes_buffered, std::size_t max_consume,
        std::error_code ec, std::size_t bytes_transferred, ReadHandler handler)
    {
        m_consume = std::min(bytes_buffered + bytes_transferred, max_consume);
        handler(ec, asio::buffer_cast<const PodType *>(
            *m_buffer.data().begin()), static_cast<std::size_t>(m_consume/sizeof(PodType)));
        if (!m_consume) return; // not the best solution, but doesn't "crash" the server at least.
    }
    
    StreamBufferClass & m_buffer;
    std::size_t m_consume;
    bool m_read_lock;
};

#endif

