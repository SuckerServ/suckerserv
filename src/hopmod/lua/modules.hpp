#ifndef HOPMOD_LUA_MODULES_HPP
#define HOPMOD_LUA_MODULES_HPP

namespace lua{
namespace module{

int open_net2(lua_State *L);
int open_crypto(lua_State *L);
int open_mmdb(lua_State *L);
int open_filesystem(lua_State *L);
int open_http_server(lua_State *L);

} //namespace module
} //namespace lua

#endif

