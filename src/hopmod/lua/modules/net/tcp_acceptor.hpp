#ifndef HOPMOD_LUA_NET_TCP_ACCEPTOR_HPP
#define HOPMOD_LUA_NET_TCP_ACCEPTOR_HPP

#include <luajit-2.0/lua.hpp>
#include <asio.hpp>

namespace lua{

class tcp_acceptor
{
public:
    typedef std::shared_ptr<asio::ip::tcp::acceptor> target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int listen(lua_State * L);
    static int close(lua_State * L);
    static int async_accept(lua_State * L);
    static int set_option(lua_State * L);
    static int get_option(lua_State * L);
};

} //namespace lua

#endif

