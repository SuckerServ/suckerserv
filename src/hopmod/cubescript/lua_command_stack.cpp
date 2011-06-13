/*
  Copyright (c) 2010 Graham Daws <graham.daws@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/
#include "lua_command_stack.hpp"
#include "lua/pcall.hpp"
#include <sstream>
#include <iostream>

namespace cubescript{

lua_command_stack::lua_command_stack(lua_State * state, int table_index)
 :m_state(state), m_table_index(table_index)
{
    
}

std::size_t lua_command_stack::push_command()
{
    return lua_gettop(m_state) + 1;
}

void lua_command_stack::push_argument_symbol(const char * value, 
                                             std::size_t length)
{
    const char * start = value;
    const char * end = start;
    const char * end_of_string = start + length;
    
    lua_pushvalue(m_state, m_table_index);
    
    while(end < end_of_string)
    {
        if(lua_type(m_state, -1) == LUA_TNIL)
        {
            lua_pop(m_state, 1);
            std::stringstream format;
            format<<"attempt to index '"<<std::string(value, start - 1)
                  <<"' (a nil value)";
                  
            throw command_error(format.str());
        }
        
        for(; end != end_of_string && *end !='.'; end++);
        
        if(end - start == 0)
            throw parse_error("invalid id given for lua index operator");
        
        lua_pushlstring(m_state, start, end - start);
        lua_gettable(m_state, -2);
        lua_replace(m_state, -2);
        
        start = end + 1;
        end = start;
    }
}

void lua_command_stack::push_argument()
{
    lua_pushnil(m_state);
}

void lua_command_stack::push_argument(bool value)
{
    lua_pushboolean(m_state, value);
}

void lua_command_stack::push_argument(int value)
{
    lua_pushinteger(m_state, value);
}

void lua_command_stack::push_argument(float value)
{
    lua_pushnumber(m_state, value);
}

void lua_command_stack::push_argument(const char * value, std::size_t length)
{
    lua_pushlstring(m_state, value, length);
}

std::string lua_command_stack::pop_string()
{
    std::string output;
    std::size_t length;
    const char * string = lua_tolstring(m_state, -1, &length);
    if(string) output = std::string(string, length);
    else
    {
        switch(lua_type(m_state, -1))
        {
            case LUA_TNIL:
                output = "nil";
                break;
            case LUA_TBOOLEAN:
                output = (lua_toboolean(m_state, -1) ? "true" : "false");
                break;
            default:
            {
                std::stringstream format;
                format<<"<"<<lua_typename(m_state, lua_type(m_state, -1))
                      <<": "<<std::hex<<lua_topointer(m_state, -1)
                      <<">"<<std::endl;
                
                output = format.str();
            }
        }
    }
    lua_pop(m_state, 1);
    return output;
}

static int on_runtime_error(lua_State * L)
{
    // Enclosing the error message in a table stops subsequent runtime error
    // functions from altering the error message argument. This is useful for
    // preserving an error message from the source nested pcall.

    if(lua_type(L, 1) != LUA_TTABLE)
    {
        lua_getglobal(L, "debug");
        if(lua_type(L, -1) != LUA_TTABLE)
        {
            lua_pop(L, 1);
            return 1;
        }
        
        lua_getfield(L, -1, "traceback");
        if(lua_type(L, -1) != LUA_TFUNCTION)
        {
            lua_pop(L, 1);
            return 1;
        }
        
        lua_pushvalue(L, 1);
        lua_pushinteger(L, 2);
        lua_pcall(L, 2, 1, 0);
        
        lua_newtable(L);
        lua_pushinteger(L, 1);
        lua_pushvalue(L, -3);
        lua_settable(L, -3);
        
        return 1;
    }
    
    lua_pushvalue(L, 1);
    return 1;
}

static void process_error_value(lua_State * L, int bottom)
{
    const char * error_message_c_str = NULL;
    
    if(lua_type(L, -1) == LUA_TTABLE)
    {
        lua_pushinteger(L, 1);
        lua_gettable(L, -2);
    }
    
    error_message_c_str = lua_tostring(L, -1);
    
    std::string error_message;
    if(error_message_c_str)
        error_message = error_message_c_str;
    
    lua_pop(L, lua_gettop(L) - bottom);
    
    throw command_error(error_message);
}

void lua_command_stack::call(std::size_t index)
{
    std::size_t top = lua_gettop(m_state);
    if(index > top)
    {
        lua_pushnil(m_state);
        return;
    }
    
    lua_pushcfunction(m_state, on_runtime_error);
    lua_insert(m_state, 1);
    
    int status = lua_pcall(m_state, top - index, LUA_MULTRET, 1);
    
    lua_remove(m_state, 1);
    
    if(status != 0) process_error_value(m_state, index);
}

namespace lua{

int eval(lua_State * L)
{
    std::size_t source_length;
    const char * source = luaL_checklstring(L, 1, &source_length);

    lua_command_stack lua_command(L, 2);
    command_stack * command = &lua_command;
    
    if(lua_type(L, 2) != LUA_TTABLE)
    {
        command = reinterpret_cast<proxy_command_stack *>(
            luaL_checkudata(L, 2, proxy_command_stack::CLASS_NAME));
    }
    
    int bottom = lua_gettop(L);
    lua_pushnil(L);
    
    try
    {
        eval(&source, source + source_length, *command);
    }
    catch(const eval_error & error)
    {
        lua_pushstring(L, error.what());
        lua_replace(L, bottom + 1);
    }

    return lua_gettop(L) - bottom;
}

int is_complete_code(lua_State * L)
{
    std::size_t code_length;
    const char * code = luaL_checklstring(L, 1, &code_length);
    bool complete = ::cubescript::is_complete_code(code, code + code_length);
    lua_pushboolean(L, complete);
    return 1;
}

proxy_command_stack::proxy_command_stack(lua_State * L)
 :m_state(L),
  m_push_command(LUA_NOREF),
  m_push_argument_symbol(LUA_NOREF),
  m_push_argument(LUA_NOREF),
  m_pop_string(LUA_NOREF),
  m_call(LUA_NOREF)
{
    
}

proxy_command_stack::~proxy_command_stack()
{
    luaL_unref(m_state, LUA_REGISTRYINDEX, m_push_command);
    luaL_unref(m_state, LUA_REGISTRYINDEX, m_push_argument_symbol);
    luaL_unref(m_state, LUA_REGISTRYINDEX, m_push_argument);
    luaL_unref(m_state, LUA_REGISTRYINDEX, m_pop_string);
    luaL_unref(m_state, LUA_REGISTRYINDEX, m_call);
}

int proxy_command_stack::__gc(lua_State * L)
{
    reinterpret_cast<proxy_command_stack *>(
        luaL_checkudata(L, 1, CLASS_NAME))->~proxy_command_stack();
    return 0;
}

std::size_t proxy_command_stack::push_command()
{
    if(m_push_command == LUA_NOREF)
        throw command_error("no function bound for push command");
    
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_push_command);

    if(::cubescript::lua::pcall(m_state, 0, 1) != 0)
        throw command_error("internal error in push command");
    
    return lua_tointeger(m_state, -1);
}

void proxy_command_stack::push_argument_symbol(
    const char * value, std::size_t value_length)
{
    if(m_push_argument_symbol == LUA_NOREF)
        throw command_error("no function bound for push argument symbol");
    
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_push_argument_symbol);
    lua_pushlstring(m_state, value, value_length);
    
    if(::cubescript::lua::pcall(m_state, 1, 1) != 0)
        throw command_error("internal error in push argument symbol");
}

void proxy_command_stack::setup_push_argument_call()
{
    if(m_push_argument == LUA_NOREF)
        throw command_error("no function bound for push argument");
    
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_push_argument);
}

void proxy_command_stack::call_push_argument(int nargs, int nresults)
{
    if(::cubescript::lua::pcall(m_state, nargs, nresults) != 0)
        throw command_error("internal error in push argument");
}

void proxy_command_stack::push_argument()
{
    setup_push_argument_call();
    call_push_argument(0, 0);
}

void proxy_command_stack::push_argument(bool value)
{
    setup_push_argument_call();
    lua_pushboolean(m_state, value);
    call_push_argument(1, 0);
}

void proxy_command_stack::push_argument(int value)
{
    setup_push_argument_call();
    lua_pushinteger(m_state, value);
    call_push_argument(1, 0);
}

void proxy_command_stack::push_argument(float value)
{
    setup_push_argument_call();
    lua_pushnumber(m_state, value);
    call_push_argument(1, 0);
}

void proxy_command_stack::push_argument(
    const char * value, std::size_t value_length)
{
    setup_push_argument_call();
    lua_pushlstring(m_state, value, value_length);
    call_push_argument(1, 0);
}

std::string proxy_command_stack::pop_string()
{
    if(m_pop_string == LUA_NOREF)
        throw command_error("no function bound for pop string");
    
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_pop_string);
    
    if(::cubescript::lua::pcall(m_state, 0, 1) != 0)
        throw command_error("internal error in push argument");

    return lua_tostring(m_state, -1);
}

void proxy_command_stack::call(std::size_t index)
{
    if(m_call == LUA_NOREF)
        throw command_error("no function bound for call command");
    
    lua_rawgeti(m_state, LUA_REGISTRYINDEX, m_call);
    lua_pushinteger(m_state, index);
    
    if(::cubescript::lua::pcall(m_state, 1, 0) != 0)
        process_error_value(m_state, lua_gettop(m_state));
}

const char * proxy_command_stack::CLASS_NAME = "command_stack";

int proxy_command_stack::register_metatable(lua_State * L)
{
    luaL_newmetatable(L, CLASS_NAME);
    luaL_Reg functions[] = {
        {"__gc", &proxy_command_stack::__gc},
        {NULL, NULL}
    };
    luaL_register(L, NULL, functions);
    lua_pop(L, 1);
    return 0;
}

int proxy_command_stack::create(lua_State * L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    
    proxy_command_stack * object = new (lua_newuserdata(L, 
        sizeof(proxy_command_stack))) proxy_command_stack(L);
    
    if(!object) return 0;
    
    lua_pushvalue(L, -2);
    
    lua_pushliteral(L, "push_command");
    lua_gettable(L, -2);
    if(lua_type(L, -1) == LUA_TFUNCTION)
        object->m_push_command = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_pop(L, 1);
    
    lua_pushliteral(L, "push_argument_symbol");
    lua_gettable(L, -2);
    if(lua_type(L, -1) == LUA_TFUNCTION)
        object->m_push_argument_symbol = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_pop(L, 1);
    
    lua_pushliteral(L, "push_argument");
    lua_gettable(L, -2);
    if(lua_type(L, -1) == LUA_TFUNCTION)
        object->m_push_argument = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_pop(L, 1);
    
    lua_pushliteral(L, "pop_string");
    lua_gettable(L, -2);
    if(lua_type(L, -1) == LUA_TFUNCTION)
        object->m_pop_string = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_pop(L, 1);
    
    lua_pushliteral(L, "call");
    lua_gettable(L, -2);
    if(lua_type(L, -1) == LUA_TFUNCTION)
        object->m_call = luaL_ref(L, LUA_REGISTRYINDEX);
    else lua_pop(L, 1);
    
    lua_pop(L, 1);
    
    luaL_getmetatable(L, CLASS_NAME);
    lua_setmetatable(L, -2);
    
    return 1;
}

} //namespace lua
} //namespace cubescript

