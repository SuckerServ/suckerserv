/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CORELIB_HPP
#define FUNGU_SCRIPT_CORELIB_HPP

namespace fungu{
namespace script{

namespace corelib{

void register_alias_functions(env &);
void unload_alias_functions(env &);
void register_anonfunc_functions(env &);
void register_controlflow_functions(env &);
void register_datetime_functions(env &);
void register_exception_functions(env &);
void register_exec_functions(env &);
void register_math_functions(env &);
void register_string_functions(env &);
void register_vector_functions(env &);

} //namespace corelib

inline void load_corelib(env & environment)
{
    script::env_frame::register_functions(environment);
    
    script::corelib::register_math_functions(environment);
    script::corelib::register_controlflow_functions(environment);
    //script::corelib::register_alias_functions(environment);
    script::corelib::register_anonfunc_functions(environment);
    script::corelib::register_exec_functions(environment);
    script::corelib::register_string_functions(environment);
    script::corelib::register_vector_functions(environment);
    script::corelib::register_datetime_functions(environment);
    script::corelib::register_exception_functions(environment);
}

inline void unload_corelib(env & environment)
{
    script::corelib::unload_alias_functions(environment);
}

} //namespace script
} //namespace fungu

#endif
