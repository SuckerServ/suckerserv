#include <lua.hpp>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace lua{

int getcwd(lua_State * L)
{
    char buffer[512];
    if(!::getcwd(buffer, sizeof(buffer)))
        luaL_error(L, "getcwd() failed with errno %i", errno);
    lua_pushstring(L, buffer);
    return 1;
}

int mkfifo(lua_State * L)
{
    const char * filename = luaL_checkstring(L, 1);
    int mode = luaL_checkint(L, 2);
    int fd = ::mkfifo(filename, static_cast<mode_t>(mode));
    lua_pushinteger(L, fd);
    return 1;
}

int open_fifo(lua_State * L)
{
    const char * filename = luaL_checkstring(L, 1);
    
    int unlink_status = unlink(filename);
    
    if(unlink_status == -1 && errno != ENOENT)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to unlink file %s (errno %d)", filename, errno);
        return 2;
    }
    
    if(::mkfifo(filename, S_IRWXU | S_IRWXG | S_IRWXO) != 0)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to create fifo file %s (errno %d)", filename, errno);
        return 2;
    }
    
    int fd = open(filename, O_RDWR | O_NONBLOCK);
    if(fd == -1)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "failed to open fifo file %s (errno %d)", filename, errno);
        return 2;
    }
    else
    {
        lua_pushinteger(L, fd);
        return 1;
    }
}

int usleep(lua_State * L)
{
    int usecs = luaL_checkint(L, 1);
    ::usleep(usecs);
    return 0;
}

} //namespace lua

