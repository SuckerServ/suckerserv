/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_SCRIPT_FWD_HPP
#define FUNGU_SCRIPT_SCRIPT_FWD_HPP

#if FUNGU_WITH_LUA
struct lua_State;
#endif

namespace fungu{
namespace script{

struct any;

class env;
class env_frame;
class env_symbol;
class env_symbol_local;
class env_object;

class callargs;
    
class error;
class error_trace;
class source_context;

template<typename Target,typename Source> Target lexical_cast(const Source &);

class construct;
class expression;
class base_expression;

} //namespace script
} //namespace fungu

#endif
