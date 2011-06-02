/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_FRAME_HPP
#define FUNGU_SCRIPT_FRAME_HPP

#include "any.hpp"

namespace fungu{
namespace script{

class env;
class env_object;
class env_symbol;
class env_symbol_local;

class env_frame
{
public:
    /**
        @brief Start a new scope
    */
    env_frame(env * envir);

    /**
        @brief Continue scope of parent frame - construct an inner frame.
    */
    env_frame(env_frame * outer_frame);

    /**
        Local bindings are deleted at frame object destruction.
    */
    ~env_frame();
    
    /**
        @brief Create local symbol binding.        
    */
    void bind_object(env_object *, const_string);

    /**
        @brief Create local symbol binding.
    */
    void bind_object(env_object *, env_symbol *);
    
    /**
        @brief Get the object bound to a local symbol.
        
        Returns NULL if the symbol is not found.
    */
    env_object * lookup_object(const_string id)const;
    
    /**
        @brief Get the object bound to a local symbol.
        
        Returns NULL if the symbol is not found.
    */
    env_object * lookup_object(const env_symbol *)const;

    /**
        @brief Get the object bound to a local symbol.
        
        Throws error(UNKNOWN_SYMBOL) exception if the symbol is not found.
    */
    env_object * lookup_required_object(const_string id)const;
    
    /**
        @brief Get the object bound to a local symbol.
        
        Throws error(UNKNOWN_SYMBOL) exception if the symbol is not found.
    */
    env_object * lookup_required_object(const env_symbol *)const;
    
    /**
        @brief Tell the evaluator not to evaluate anymore expressions.
    */
    void signal_return();
    
    /**
        @brief Tell the evaluator not to evaluate anymore expressions.
        @param value set the explicit result value
    */
    void signal_return(any value);
    
    /**
        @brief Undo a signal_return() call.
    */
    void unset_return();
    
    /**
        @brief Get the explicit result value.
    */
    any get_result_value();
    
    /**
        @brief Has signal_return() been called?
    */
    bool has_expired()const;
    
    /**
        @brief Return a pointer of the associated environment object.
    */
    env * get_env()const;
    
    /**
        @brief Get the root frame of this scope.
    */
    env_frame * get_scope_frame()const;
    
    /**
        @brief Get the latest symbol_local object created.
    */
    env_symbol_local * get_last_bind()const;
    
    /**
        @brief Register frame-related runtime functions with a global environment.
    */
    static void register_functions(env &);
private:
    env_frame(const env_frame &);

    /**
        @brief Reattach local symbol bindings to environment.
    */
    void attach_locals();
    
    /**
        @brief Detach local symbol bindings from environment.
        
        Once detached, the local symbols will remain allocated but will be
        invisible to symbol lookups.
    */
    void detach_locals();

    /**
        @brief Are local symbols currently detached from the environment?
    */
    bool is_detached_from_env()const;

    any signal_return_scriptfun(callargs & args);
    any set_result_scriptfun(callargs & args);
    any bind_myvar_scriptfun(callargs & args);
    any bind_localvar_scriptfun(callargs & args);
    any bind_globalvar_scriptfun(callargs & args);
    any bind_var_scriptfun(callargs & args,int scope);
    any getvar_scriptfun(callargs & args);
    any setvar_scriptfun(callargs & args);
    any issymbol_scriptfun(callargs & args);
    any isnull_scriptfun(callargs & args);
    any retnull_scriptfun(callargs & args);
    any isprocedure_scriptfun(callargs & args);
    
    env * m_env;
    env_frame * m_scope;
    
    env_symbol_local * m_last_binding;
    
    bool m_expired;
    any m_result;
    
    bool m_detached;
};

} //namespace script
} //namespace fungu

#endif
