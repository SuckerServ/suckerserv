#include "file_stream.hpp"
#include "weak_ref.hpp"
#include "module.hpp"
#include "../../register_class.hpp"
#include "../../to.hpp"
#include "../../create_object.hpp"
#include "../../pcall.hpp"
#include "../../error_handler.hpp"
#include <asio.hpp>
#include <functional>
using namespace asio;

namespace lua{

const char * file_stream::CLASS_NAME = "file_stream";

int file_stream::register_class(lua_State * L){
    static luaL_Reg member_functions[] = {
        {"__gc", &file_stream::__gc},
        {"async_read_some", &file_stream::async_read_some},
        {"cancel", &file_stream::cancel},
        {"close", &file_stream::close},
        {NULL, NULL}
    };
    lua::register_class(L, CLASS_NAME, member_functions);
    return 0;
}

int file_stream::create_object(lua_State * L)
{
    int fd = luaL_checkint(L, 1);
    try
    {
        lua::create_object<file_stream>(L, std::shared_ptr<posix::stream_descriptor>(
            new posix::stream_descriptor(get_main_io_service(L), fd)));
    }
    catch(const std::system_error & error)
    {
        return luaL_error(L, "%s", error.code().message().c_str());
    }
    return 1;
}

int file_stream::__gc(lua_State * L)
{
    file_stream::target_type self = *lua::to<file_stream>(L, 1);
    std::error_code ec;
    self->close(ec);
    self.~target_type();
    return 0;
}

static void async_read_some_handler(lua_State * L, lua::weak_ref callback, char * char_buffer, 
    const std::error_code & ec, std::size_t bytes_transferred)
{
    if(callback.is_expired())
    {   
        delete [] char_buffer;
        return;
    }
    
    lua::get_error_handler(L);
    int error_function = lua_gettop(L);
    
    callback.get(L);
    
    int args = 0;
    
    if(!ec)
    {
        lua_pushlstring(L, char_buffer, bytes_transferred);
        args = 1;
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, ec.message().c_str());
        args = 2;
    }
    
    if(lua::pcall(L, args, 0, error_function) != 0)
        log_error(L, "async_read_some");
    
    lua_pop(L, 1);
    
    callback.unref(L);
    delete [] char_buffer;
}

int file_stream::async_read_some(lua_State * L)
{
    target_type self = *lua::to<file_stream>(L, 1);
    
    int max_length = luaL_checkint(L, 2);
    
    luaL_checktype(L, 3, LUA_TFUNCTION);
    lua_pushvalue(L, 3);
    lua::weak_ref callback_ref = lua::weak_ref::create(L);

    char * char_buffer;
    try
    {
        char_buffer = new char[max_length];
    }
    catch(std::bad_alloc)
    {
        luaL_error(L, "memory allocation error");
        return 0;
    }
    
    self->async_read_some(buffer(char_buffer, max_length), 
        std::bind(async_read_some_handler, L, callback_ref, char_buffer, std::placeholders::_1, std::placeholders::_2));
    
    return 0;
}

int file_stream::cancel(lua_State * L)
{
    target_type self = *lua::to<file_stream>(L, 1);
    std::error_code ec;
    self->cancel(ec);
    if(ec) return luaL_error(L, ec.message().c_str());
    return 0;
}

int file_stream::close(lua_State * L)
{
    target_type self = *lua::to<file_stream>(L, 1);
    std::error_code ec;
    self->close(ec);
    if(ec) return luaL_error(L, ec.message().c_str());
    return 0;
}

} //namespace lua

