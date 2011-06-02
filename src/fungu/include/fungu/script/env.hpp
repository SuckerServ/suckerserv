/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_ENV_HPP
#define FUNGU_SCRIPT_ENV_HPP

#include "../string.hpp"
#include "env_symbol.hpp"
#include "env_frame.hpp"
#include "env_object.hpp"

#include <boost/unordered_map.hpp>
#include <boost/function.hpp>

#ifdef FUNGU_WITH_LUA
extern "C"{
#include <lua.h>
}
#endif

#define FUNGU_OBJECT_ID(id) fungu::const_string(FUNGU_LITERAL_STRING(id))

namespace fungu{
namespace script{

class source_context;

/**
    @brief Global environment class.
    
    Stores a symbol table and other global state for a scripting environment.
*/
class env
{
public:
    static const int recursion_limit = 1000;
    
    typedef boost::function2<void, const_string, env_object *> observer_function;
    
    env();
    
    /**
        Symbol objects referenced in the symbol table are deleted.
    */
    virtual ~env();

    /**
        @brief Search for a symbol in the symbol table.
        
        @param id symbol name
        @return pointer to a symbol object
        
        Returns NULL if a symbol is not found.
    */
    env_symbol * lookup_symbol(const_string id)const;
    
    /**
        @brief Create a symbol object and register the name with the symbol
        table.
        
        @param id symbol name
        @return pointer to symbol object.
        
        If the symbol entry already exists then creation of a new symbol object
        is bypassed and a pointer to the existing symbol object is returned.
    */
    env_symbol * create_symbol(const_string id);
    
    /**
        @brief Create a global symbol binding.
        
        @param obj pointer of the object to bind
        @param id symbol name
        
        The operation is always successful; an existing symbol binding with the
        same name is overridden.
    */
    void bind_global_object(env_object * obj, const_string id);
    
    /**
    
        
    */
    void unbind_global_object(const_string id);
    
    /**
        @brief Search for a global symbol binding.
        
        @param id symbol name
        @return pointer to the bound object
        
        Returns NULL if the symbol is not found.
    */
    env_object * lookup_global_object(const_string id)const;
    
    /**
        @brief Set associated source_context object.
    */
    void set_source_context(const source_context * source_ctx);
    
    /**
        @brief Get associated source_context object.
        
        Get information about the source code location of the current
        expression being evaluated.
    */
    const source_context * get_source_context()const;
    
    /**
        @brief Set a callback function to observe the creation of global symbol
        bindings.
        
        @param new_function Observer function; something than can be called as new_function(id, object).
        @return The old observer function as a boost::function2<void, const_string, env_object *> object.
        
        The observer function is called from the bind_global_object method.
    */
    template<typename ObserverFunction>
    observer_function set_bind_observer(ObserverFunction new_function)
    {
        observer_function old_function = m_bind_observer;
        m_bind_observer = new_function;
        return old_function;
    }
    
    template<typename ObserverFunction>
    observer_function set_unbind_observer(ObserverFunction new_function)
    {
        observer_function old_function = m_unbind_observer;
        m_unbind_observer = new_function;
        return old_function;
    }
    
    /**
        @brief Unset observer function set by set_bind_observer method.
        @return The observer function that was set, returned as a boost::function2<void, const_string, object *> object.
    */
    observer_function unset_bind_observer();
    
    observer_function unset_unbind_observer();
    
    #ifdef FUNGU_WITH_LUA
    /**
        @brief Get associated lua state.
    */
    lua_State * get_lua_state()const;
    
    /**
        @brief Set associated lua state.
    */
    void set_lua_state(lua_State * state);
    #endif
private:
    boost::unordered_map<const_string, env_symbol *> m_symbol;
    
    const source_context * m_source_ctx;
    int m_recursion_limit;
    
    observer_function m_bind_observer;
    observer_function m_unbind_observer;
    
    #ifdef FUNGU_WITH_LUA
    lua_State * m_lua_state;
    #endif
};

void initialize_library();

} //namespace script
} //namespace fungu

#endif
