/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_CONVERT_HPP
#define FUNGU_CONVERT_HPP

#include "string.hpp"
#include <cmath>
#include <typeinfo>
#include <limits>

namespace fungu{

template<typename IntType>
IntType to_int(const const_string & input)
{
    const char * m_firstc = input.begin();
    const char * m_lastc = input.end() - 1;
    
    //TODO replace with a pow metafunction, if that's possible
    static IntType max_placeval = static_cast<IntType>(pow(10,std::numeric_limits<IntType>::digits10));
    static IntType highest_digit = std::numeric_limits<IntType>::max() / max_placeval;
    
    IntType result = 0;
    IntType placeval = 1;
    
    bool signed_ = std::numeric_limits<IntType>::is_signed;
    bool minus = signed_ && *m_firstc == '-';
    const_string::const_iterator firstc = m_firstc + minus;
    
    for(const_string::const_iterator i = m_lastc; i >= firstc; --i)
    {
        char c = i[0];
        bool last = i == firstc;
        
        if(c < '0' || c > '9') throw std::bad_cast();
        
        IntType digit = c - '0';
        if(placeval == max_placeval && digit > highest_digit) throw std::bad_cast();
        IntType tmp = placeval * digit;
        
        if( tmp > std::numeric_limits<IntType>::max() - result ) 
            throw std::bad_cast();
        result += tmp;
        
        if(placeval == max_placeval && !last) throw std::bad_cast();
        placeval *= 10;
    }
    
    if(minus) result = -result;
    
    return result;
}

template<typename IntType>
const_string from_int(IntType i)
{
    int index = std::numeric_limits<IntType>::digits10 + 1;
    bool negative = std::numeric_limits<IntType>::is_signed && static_cast<typename std::make_signed<IntType>::type>(i) < 0;
    if(negative)
    {
        if(i == std::numeric_limits<IntType>::min()) throw std::bad_cast();
        i = -i;
    }
    
    char *tmp = new char[index + 2];
    tmp[index + 1] = '\0';
    
    do
    {
        tmp[index--] = '0' + (i % 10);
        i /= 10;
    }while(i);
    
    if(negative) tmp[index] = '-';
    else ++index;
    
    const_string str = const_string(std::string(tmp + index));
    delete[] tmp;

    return str;
}

} //namespace fungu

#endif
