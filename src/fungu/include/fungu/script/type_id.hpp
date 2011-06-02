/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_TYPE_ID_HPP
#define FUNGU_SCRIPT_TYPE_ID_HPP

#include "env_object.hpp"
#include "../type_tag.hpp"
#include "../string.hpp"

namespace fungu{
namespace script{

namespace any_detail{struct empty;}

class type_id
{
public:
    enum base_id
    {
        FOREIGN = 0,
        NIL,
        BOOLEAN,
        NUMBER,
        STRING,
        OBJECT
    }; static const std::size_t base_id_count = 6;
    
    enum sub_id
    {
        NIL_INT = 0,
        SIGNED_CHAR,
        SIGNED_INT,
        SIGNED_SHORT_INT,
        SIGNED_LONG_INT,
        UNSIGNED_CHAR,
        UNSIGNED_INT,
        UNSIGNED_SHORT_INT,
        UNSIGNED_LONG_INT,
        FLOAT,
        DOUBLE,
        LONG_DOUBLE,
        BOOL,
        WCHAR_T,
        C_STRING,
        STD_STRING,
        CONST_STRING,
        DATA_OBJECT,
        FUNCTION_OBJECT
    }; static const std::size_t sub_id_count = 19;
    
    template<typename T> 
    static type_id get(type_tag<T>){return type_id(FOREIGN, NIL_INT);}
    
    static type_id get(type_tag<signed char>);
    static type_id get(type_tag<signed int>);
    static type_id get(type_tag<signed short int>);
    static type_id get(type_tag<signed long int>);
    static type_id get(type_tag<unsigned char>);
    static type_id get(type_tag<unsigned int>);
    static type_id get(type_tag<unsigned short int>);
    static type_id get(type_tag<unsigned long int>);
    static type_id get(type_tag<float>);
    static type_id get(type_tag<double>);
    static type_id get(type_tag<bool>);
    static type_id get(type_tag<const_string>);
    static type_id get(type_tag<const char *>);
    static type_id get(type_tag<std::string>);
    static type_id get(type_tag<any_detail::empty>);
    
    type_id(base_id base, sub_id sub);
    type_id();
    
    bool is_nil()const;
    bool is_boolean()const;
    bool is_number()const;
    bool is_string()const;
    bool is_object()const;
    
    base_id base()const;
    sub_id sub()const;
private:
    base_id m_base;
    sub_id m_sub;
};

} //namespace script
} //namespace fungu

#endif
