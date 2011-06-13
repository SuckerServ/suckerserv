#ifndef HOPMOD_LUA_NET_SSL_TCP_STREAM_HPP
#define HOPMOD_LUA_NET_SSL_TCP_STREAM_HPP

#include "tcp_socket.hpp"
#include "ssl_context.hpp"
#include <lua.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio/ssl.hpp>

namespace lua{

struct ssl_tcp_stream
{
    ssl_tcp_stream(boost::shared_ptr<tcp_socket>, boost::shared_ptr<boost::asio::ssl::context>);
    boost::shared_ptr<tcp_socket> socket;
    boost::shared_ptr<boost::asio::ssl::context> ssl_context;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket &> stream;
};

class managed_ssl_tcp_stream
{
public:
    typedef boost::shared_ptr<ssl_tcp_stream> target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
    static int push_constants(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int async_handshake(lua_State * L);
    static int async_read(lua_State * L);
    static int async_read_until(lua_State * L);
    static int async_send(lua_State * L);
    static int async_shutdown(lua_State * L);
};

} //namespace lua

#endif

