#ifndef LUA_PCALL_HPP
#define LUA_PCALL_HPP

namespace cubescript{
namespace lua{

// A lua_pcall function wrapper that uses the function set by set_error_handler() as the error function
int pcall(lua_State * L, int nargs, int nresults);

void set_error_handler(lua_State * L, lua_CFunction);
void unset_error_handler(lua_State * L);
bool get_error_handler(lua_State * L);
lua_CFunction get_pcall_error_function();

int save_error_info(lua_State *);
int push_error_info(lua_State *);

int save_traceback(lua_State * L);
int push_traceback(lua_State * L);

bool is_callable(lua_State *, int);

} //namespace lua
} //namespace cubescript

#endif

