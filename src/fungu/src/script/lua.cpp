/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#include "fungu/script/lua/arguments.hpp"
#include "fungu/script/lua/lua_function.hpp"
#include "fungu/script/lua/object_wrapper.hpp"
#include "fungu/script/callargs.hpp"
#include "fungu/script/any_variable.hpp"

#include <limits>

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "lua/arguments.cpp"
#include "lua/lua_function.cpp"
#include "lua/object_wrapper.cpp"
#include "lua/push_value.cpp"
