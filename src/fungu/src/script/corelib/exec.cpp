/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace corelib{

namespace execlib{

inline any exec_cubescript(env_object::call_arguments & args,env_frame * aFrame)
{
    const_string filename = args.safe_casted_front<const_string>();
    args.pop_front();
    execute_file(std::string(filename.copy()).c_str(),*aFrame->get_env());
    return any::null_value();
}

#ifdef FUNGU_WITH_LUA
inline any exec_lua(env_object::call_arguments & args,env_frame * aFrame)
{
    const_string filename = args.safe_casted_front<const_string>();
    args.pop_front();
    
    lua_State *L = aFrame->get_env()->get_lua_state();
    int loadError = luaL_loadfile(L, filename.copy().c_str());
    
    if(loadError)
    {
        if(loadError == LUA_ERRSYNTAX)
        {
            throw error(LUA_ERROR, boost::make_tuple(L));
        }
        else
        {
            lua_pop(L, 1);
            throw error(OPERATION_ERROR, boost::make_tuple(std::string("lua memory allocation error")));
        }
    }
    
    lua::lua_function func(L, -1);
    lua_pop(L, 1);
    
    func.call(args, aFrame);
    
    return any::null_value();
}
#endif

inline const_string get_filename_extension(const_string filename)
{
    for(const_string::const_iterator it = filename.end() - 1; it != filename.begin(); it--)
        if(*it=='.') return filename.substring(it+1,filename.end()-1);
    return const_string();
}

inline any exec_script(env_object::call_arguments & args,env_frame * aFrame)
{
    const_string filename = args.safe_casted_front<const_string>();
    const_string ext = get_filename_extension(filename);
    
    #ifdef FUNGU_WITH_LUA
    if(ext == fungu::const_string(FUNGU_LITERAL_STRING("lua")))
    {
        return exec_lua(args, aFrame);
    }
    else
    #endif
    {
        return exec_cubescript(args, aFrame);
    }
}

} //namespace detail

void register_exec_functions(env & environment)
{
    static function<raw_function_type> exec_func(execlib::exec_script);
    environment.bind_global_object(&exec_func, FUNGU_OBJECT_ID("exec"));
    
    static function<raw_function_type> exec_cubescript_func(execlib::exec_cubescript);
    environment.bind_global_object(&exec_cubescript_func, FUNGU_OBJECT_ID("exec_cubescript"));
    
    #ifdef FUNGU_WITH_LUA
    static function<raw_function_type> exec_lua_func(execlib::exec_lua);
    environment.bind_global_object(&exec_lua_func, FUNGU_OBJECT_ID("exec_lua"));
    #endif
}

} //namespace corelib
} //namespace script
} //namespace fungu
