/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.h"
#endif

#include "fungu/script/any_variable.hpp"
#include "fungu/script/env_object.hpp"
#include "fungu/script/callargs.hpp"

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "any_variable.cpp"
