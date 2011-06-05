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
#ifndef CUBESCRIPT_LUA_COMMAND_STACK_HPP
#define CUBESCRIPT_LUA_COMMAND_STACK_HPP

#include <lua.hpp>
#include "cubescript.hpp"

namespace cubescript{

/**
    A command stack implementation for Lua.
*/
class lua_command_stack:public command_stack
{
public:
    /**
        @param table_index Index value to a table on the given Lua stack to be
               used as the environment for this command stack.
    */
    lua_command_stack(lua_State * L, int table_index);
    
    std::size_t push_command();
    
    /**
        Lookup a variable name in the table and then push the variable's value 
        to the top of the stack. Names with sub keys are supported with the 
        Lua-like syntax, "name.subname.subname".
    */
    void push_argument_symbol(const char * id, std::size_t id_length);
    
    void push_argument();
    void push_argument(bool);
    void push_argument(int);
    void push_argument(float);
    void push_argument(const char *, std::size_t);
    std::string pop_string();
    void call(std::size_t);
private:
    lua_State * m_state;
    int m_table_index;
};

namespace lua{

/**
    A lua wrapper function for eval() (declared in cubescript.hpp).
*/
int eval(lua_State * L);

/**
    A lua wrapper function for is_complete_code() (declared in cubescript.hpp)
*/
int is_complete_code(lua_State * L);

/**
    For implementing command stacks in Lua code
    
    Used by the Cubescript runtime library for translating Cubescript code to 
    Lua code.
*/
class proxy_command_stack:public command_stack
{
public:
    static const char * CLASS_NAME;
    static int register_metatable(lua_State * L);
    static int create(lua_State *);
    
    std::size_t push_command();
    void push_argument_symbol(const char *, std::size_t);
    void push_argument();
    void push_argument(bool);
    void push_argument(int);
    void push_argument(float);
    void push_argument(const char *, std::size_t);
    std::string pop_string();
    void call(std::size_t);
private:
    proxy_command_stack(lua_State *);
    ~proxy_command_stack();
    static int __gc(lua_State * L);
    
    void setup_push_argument_call();
    void call_push_argument(int nargs, int nresults);
    
    lua_State * m_state;
    int m_push_command;
    int m_push_argument_symbol;
    int m_push_argument;
    int m_pop_string;
    int m_call;
};

} //namespace lua
} //namespace cubescript

#endif

