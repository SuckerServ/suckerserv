#ifndef HOPMOD_LUA_NET_TCP_SOCKET_HPP
#define HOPMOD_LUA_NET_TCP_SOCKET_HPP

#include <lua.hpp>
#include <asio.hpp>
#include "read_stream_buffer.hpp"

namespace lua{

struct tcp_socket
{
    tcp_socket(asio::io_service &);
    asio::ip::tcp::socket socket;
    asio::streambuf read_buffer_buffer;
    read_stream_buffer<asio::streambuf, char> read_buffer;
};

class managed_tcp_socket
{
public:
    typedef std::shared_ptr<tcp_socket> target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int open(lua_State * L);
    static int close(lua_State * L);
    static int cancel(lua_State * L);
    static int shutdown(lua_State * L);
    static int shutdown_send(lua_State * L);
    static int shutdown_receive(lua_State * L);
    static int local_endpoint(lua_State * L);
    static int remote_endpoint(lua_State * L);
    static int get_option(lua_State * L);
    static int set_option(lua_State * L);
    static int async_read(lua_State * L);
    static int async_read_until(lua_State * L);
    static int async_send(lua_State * L);
    static int async_connect(lua_State * L);
    static int bind(lua_State * L);
};

} //namespace lua

#endif

