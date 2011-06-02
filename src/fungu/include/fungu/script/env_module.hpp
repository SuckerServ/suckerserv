/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_ENV_MODULE_HPP
#define FUNGU_SCRIPT_ENV_MODULE_HPP

namespace fungu{
namespace script{

template<class Derived>
class env_module
{
public:
    env_module(env * e)
    :m_env(e)
    {
        
    }
    
    static int get_module_id()
    {
        static int id = env::generate_module_id();
        return id;
    }
protected:
    env * get_env()const{return m_env;}
private:
    env * m_env;
};

} //namespace script
} //namespace fungu

#endif
