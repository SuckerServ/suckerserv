#include "../cubescript/cubescript.hpp"
#include "../cubescript/lua_command_stack.hpp"

namespace lua{
namespace module{

void open_cubescript(lua_State * L)
{
    cubescript::lua::proxy_command_stack::register_metatable(L);
    
    luaL_Reg cubescript_functions[] = {
        {"eval", cubescript::lua::eval},
        {"command_stack", &cubescript::lua::proxy_command_stack::create},
        {"is_complete_expression", &cubescript::lua::is_complete_code},
        {NULL, NULL}
    };
    
    luaL_register(L, "cubescript", cubescript_functions);
    
    lua_pop(L, 1);
}

} //namespace module
} //namespace lua

