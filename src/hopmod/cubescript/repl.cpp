#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <string>
#include "cubescript.hpp"
#include "lua_command_stack.hpp"
#include "lua/pcall.hpp"

static int env_table_ref = LUA_NOREF;
static int print_function_ref = LUA_NOREF;
static int debug_traceback_function_ref = LUA_NOREF;

static int set_env_table(lua_State * L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_unref(L, LUA_REGISTRYINDEX, env_table_ref);
    lua_pushvalue(L, 1);
    env_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    return 0;
}

int main(int, char**)
{
    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    
    lua_getglobal(L, "print");
    if(lua_type(L, -1) != LUA_TFUNCTION)
    {
        std::cerr<<"Startup error: _G['print'] is not a function"<<std::endl;
        return 1;
    }
    print_function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    debug_traceback_function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    cubescript::lua::proxy_command_stack::register_metatable(L);
    
    luaL_Reg cubescript_functions[] = {
        {"eval", cubescript::lua::eval},
        {"command_stack", &cubescript::lua::proxy_command_stack::create},
        {"is_complete_expression", &cubescript::lua::is_complete_code},
        {NULL, NULL}
    };
    luaL_register(L, "cubescript", cubescript_functions);
    
    lua_pushcfunction(L, set_env_table);
    lua_setglobal(L, "set_env_table");
    
    if(luaL_dofile(L, "./init.lua") != 0)
        std::cerr<<lua_tostring(L, -1)<<std::endl;
    
    std::string code;
    const char * line;
    while((line = readline(code.length() ? ">> " : "> ")))
    {
        if(!line[0]) continue;
        
        code += line;
        code += "\n";
        
        const char * code_c_str = code.c_str();
        const char * code_c_str_end = code_c_str + code.length();
        
        if(!cubescript::is_complete_code(code_c_str, code_c_str_end)) continue;
        
        if(env_table_ref != LUA_NOREF)
            lua_rawgeti(L, LUA_REGISTRYINDEX, env_table_ref);
        else lua_pushvalue(L, LUA_GLOBALSINDEX);
        
        int bottom = lua_gettop(L);
        
        cubescript::lua_command_stack lua_command(L, bottom);
        
        int print_function = lua_command.push_command();
        lua_rawgeti(L, LUA_REGISTRYINDEX, print_function_ref);
        
        
        bool discard_stack = false;
        
        try
        {
            cubescript::eval(&code_c_str, code_c_str_end, lua_command);
        }
        catch(const cubescript::parse_error & error)
        {
            std::cout<<"Parse error: "<<error.what()<<std::endl;
            discard_stack = true;
        }
        catch(const cubescript::eval_error & error)
        {
            std::cout<<"Command error: "<<error.what()<<std::endl;
            discard_stack = true;
        }
        
        int stack_size = lua_gettop(L);
        
        if(stack_size > bottom + 1 && !discard_stack)
            lua_command.call(print_function);
        else lua_pop(L, stack_size - bottom);
        
        lua_pop(L, 1); // env_table_ref
        
        code.clear();
        add_history(line);
    }
    
    std::cout<<std::endl;
    return 0;
}

