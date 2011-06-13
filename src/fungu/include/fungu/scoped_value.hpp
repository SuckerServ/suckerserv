/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCOPED_VALUE_HPP
#define FUNGU_SCOPED_VALUE_HPP

namespace fungu{

template<typename T>
class scoped_value
{
public:
    scoped_value(T & var, const T & new_value)
     :m_reset_value(var),
      m_ref(var)
    {
        var = new_value;
    }
    
    scoped_value(T & var, const T & new_value, const T & reset_value)
    :m_reset_value(reset_value),
     m_ref(var)
    {
        var = new_value;
    }
    
    ~scoped_value()
    {
        m_ref = m_reset_value;
    }
private:
    T m_reset_value;
    T & m_ref;
};
    
} //namespace fungu

#endif
