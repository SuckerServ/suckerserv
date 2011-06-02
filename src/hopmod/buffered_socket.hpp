#ifndef HOPMOD_BUFFERED_SOCKET_HPP
#define HOPMOD_BUFFERED_SOCKET_HPP

#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace boost{
namespace asio{

template<typename SocketClass>
class buffered_reading
{
public:
    buffered_reading(SocketClass & socket)
     :m_socket(socket)
    {
        
    }
    
    template<typename ReadHandler>
    void async_read_until(char delim, ReadHandler handler)
    {
        boost::asio::async_read_until(m_socket, m_buffer, delim, handler);
    }
    
    template<typename ReadHandler>
    void async_read_until(const std::string & delim, ReadHandler handler)
    {
        boost::asio::async_read_until(m_socket, m_buffer, delim, handler);
    }
    
    template<typename ReadHandler>
    void async_read(const std::size_t readSize, ReadHandler handler)
    {
        if(readSize < m_buffer.size())
        {
            m_socket.get_io_service().post(boost::bind(handler, boost::system::error_code(), readSize));
        }
        else
        {
            boost::asio::async_read(m_socket, m_buffer, boost::asio::transfer_at_least(readSize - m_buffer.size()), handler);
        }
    }
    
    const boost::asio::streambuf & read_buffer()const
    {
        return m_buffer;
    }
    
    boost::asio::streambuf & read_buffer()
    {
        return m_buffer;
    }
    
private:
    SocketClass & m_socket;
    boost::asio::streambuf m_buffer;
};

} //namespace asio
} //namespace boost

#endif
