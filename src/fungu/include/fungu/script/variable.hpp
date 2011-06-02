/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_VARIABLE_HPP
#define FUNGU_SCRIPT_VARIABLE_HPP

#include "callargs.hpp"

#ifdef FUNGU_WITH_LUA
#include "lua/push_value.hpp"
#endif

#include <boost/function.hpp>
#include <sstream>

namespace fungu{
namespace script{

class env_frame;

/**
    @brief C++ variable wrapper.
*/
template<typename T>
class variable:public env_object
{
public:
    variable(T & var)
     :m_var(var)
    {
        
    }
    
    /**
        @brief Returns DATA_OBJECT value.
    */
    object_type get_object_type()const
    {
        return DATA_OBJECT;
    }
    
    /**
        @brief Assign new value.
    */
    void assign(const any & value)
    {
        m_var = lexical_cast<T>(value);
    }
    
    /**
        @brief Assign new value and return it.
    */
    any call(call_arguments & args, env_frame *)
    {
        m_var = args.safe_casted_front<T>();
        args.pop_front();
        try{return value();}
        catch(error){return any::null_value();}
    }
    
    /**
        @brief Get value.
    */
    any value()
    {
        return m_var;
    }
    
    #ifdef FUNGU_WITH_LUA
    void value(lua_State * L)
    {
        lua::push_value(L, m_var);
    }
    #endif
    
    /**
        @brief Return value as native type.
    */
    const T & get_value()
    {
        return m_var;
    }
protected:
    T & get_var(){return m_var;}
private:
    T & m_var;
};

/**
    @brief C++ variable wrapper with controlled access and hooking.
*/
template<typename T>
class managed_variable:public variable<T>
{
public:
    managed_variable(T & var)
     :variable<T>(var),
      m_perms(0)
    {
        
    }
    
    void assign(const any & value)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        if(m_write_hook_func)
        {
            T tmp = any_cast<T>(value);
            m_write_hook_func(tmp);
        }
        variable<T>::assign(value);
    }
    
    any call(env_object::call_arguments & args,env_frame * frame)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        if(m_write_hook_func)
        {
            T tmp = args.safe_casted_front<T>();
            m_write_hook_func(tmp);
        }
        return variable<T>::call(args,frame);
    }
    
    any value()
    {
        if(m_perms & DENY_READ) throw error(NO_READ,boost::make_tuple());
        return variable<T>::value();
    }
    
    managed_variable<T> & lock_read(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_READ : m_perms & ~DENY_READ);
        return *this;
    }
    
    managed_variable<T> & lock_write(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_WRITE : m_perms & ~DENY_WRITE);
        return *this;
    }
    
    template<typename WriteHookFunction>
    managed_variable<T> & set_write_hook(WriteHookFunction func)
    {
        m_write_hook_func = func;
        return *this;
    }
private:
    enum
    {
        DENY_READ = 1,
        DENY_WRITE = 2
    };
    char m_perms;
    
    boost::function1<void,const T &> m_write_hook_func;
};

namespace var_hooks{

template<typename T>
void inclusive_range(const T & min,const T & max, const T & value)
{
    if(value < min)
    {
        std::stringstream msg;
        msg<<"value too small ("<<min<<" minimum)";
        throw error(INVALID_VALUE,boost::make_tuple(msg.str()));
    }
    else if(value > max)
    {
        std::stringstream msg;
        msg<<"value too big ("<<max<< " maximum)";
        throw error(INVALID_VALUE,boost::make_tuple(msg.str()));
    }
}

} //namespace var_hooks

/**
    @brief C++ variable wrapper with ability to lock read or write access.
*/
template<typename T>
class lockable_variable:public variable<T>
{
public:
    lockable_variable(T & var)
     :variable<T>(var),
      m_perms(0)
    {
        
    }
    
    void assign(const any & value)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        variable<T>::assign(value);
    }
    
    any call(env_object::call_arguments & args,env_frame * frame)
    {
        if(m_perms & DENY_WRITE) throw error(NO_WRITE,boost::make_tuple());
        return variable<T>::call(args,frame);
    }
    
    any value()
    {
        if(m_perms & DENY_READ) throw error(NO_READ,boost::make_tuple());
        return variable<T>::value();
    }
    
    lockable_variable<T> & lock_read(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_READ : m_perms & ~DENY_READ);
        return *this;
    }
    
    lockable_variable<T> & lock_write(bool enable)
    {
        m_perms = (enable ? m_perms | DENY_WRITE : m_perms & ~DENY_WRITE);
        return *this;
    }
private:
    enum
    {
        DENY_READ = 1,
        DENY_WRITE = 2
    };
    char m_perms;
};

/**
    @brief Make wrapped C++ function appear as a read-only variable.
*/
template<typename T>
class function_variable:public env_object
{
public:
    template<typename Functor>
     function_variable(Functor func):m_func(func){}
    object_type get_object_type()const{return DATA_OBJECT;}
    void assign(const any & value){throw error(NO_WRITE);}
    any call(env_object::call_arguments & args,env_frame * frame){throw error(NO_WRITE);}
    any value(){return m_func();}
    #ifdef FUNGU_WITH_LUA
    void value(lua_State * L)
    {
        lua::push_value(L, m_func());
    }
    #endif
private:
    boost::function0<T> m_func;
};

} //namespace script
} //namespace fungu

#endif
