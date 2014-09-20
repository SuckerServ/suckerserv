#ifndef HOPMOD_LUA_NET_SSL_TCP_STREAM_HPP
#define HOPMOD_LUA_NET_SSL_TCP_STREAM_HPP

#include "tcp_socket.hpp"
#include "ssl_context.hpp"
#include <lua.hpp>
#include <asio/ssl.hpp>

namespace lua{

struct ssl_tcp_stream
{
    ssl_tcp_stream(std::shared_ptr<tcp_socket>, std::shared_ptr<asio::ssl::context>);
    std::shared_ptr<tcp_socket> socket;
    std::shared_ptr<asio::ssl::context> ssl_context;
    asio::ssl::stream<asio::ip::tcp::socket &> stream;
};

class managed_ssl_tcp_stream
{
public:
    typedef std::shared_ptr<ssl_tcp_stream> target_type;
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

