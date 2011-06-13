#ifndef HOPMOD_LUA_NET_FILE_STREAM_HPP
#define HOPMOD_LUA_NET_FILE_STREAM_HPP

#include <lua.hpp>
#include <boost/asio.hpp>

namespace lua{

class file_stream
{
public:
    typedef boost::shared_ptr<boost::asio::posix::stream_descriptor> target_type;
    static const char * CLASS_NAME;
    static int register_class(lua_State * L);
    static int create_object(lua_State * L);
private:
    static int __gc(lua_State * L);
    static int async_read_some(lua_State * L);
    static int cancel(lua_State * L);
    static int close(lua_State * L);
};

} //namespace lua

#endif

