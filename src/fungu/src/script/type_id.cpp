/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

type_id::type_id(base_id base, sub_id sub)
 :m_base(base),m_sub(sub)
{
    
}

type_id::type_id()
 :m_base(FOREIGN), m_sub(SIGNED_INT)
{
    
}

type_id type_id::get(type_tag<signed char>){return type_id(NUMBER, SIGNED_CHAR);}
type_id type_id::get(type_tag<signed int>){return type_id(NUMBER, SIGNED_INT);}
type_id type_id::get(type_tag<signed short int>){return type_id(NUMBER, SIGNED_SHORT_INT);}
type_id type_id::get(type_tag<signed long int>){return type_id(NUMBER, SIGNED_LONG_INT);}
type_id type_id::get(type_tag<unsigned char>){return type_id(NUMBER, UNSIGNED_CHAR);}
type_id type_id::get(type_tag<unsigned int>){return type_id(NUMBER, UNSIGNED_INT);}
type_id type_id::get(type_tag<unsigned short int>){return type_id(NUMBER, UNSIGNED_SHORT_INT);}
type_id type_id::get(type_tag<unsigned long int>){return type_id(NUMBER, UNSIGNED_LONG_INT);}
type_id type_id::get(type_tag<float>){return type_id(NUMBER, FLOAT);}
type_id type_id::get(type_tag<double>){return type_id(NUMBER, DOUBLE);}
type_id type_id::get(type_tag<bool>){return type_id(BOOLEAN, UNSIGNED_CHAR);}
type_id type_id::get(type_tag<const_string>){return type_id(STRING, CONST_STRING);}
type_id type_id::get(type_tag<const char *>){return type_id(STRING, C_STRING);}
type_id type_id::get(type_tag<std::string>){return type_id(STRING, STD_STRING);}
type_id type_id::get(type_tag<any_detail::empty>){return type_id(NIL, NIL_INT);}

bool type_id::is_nil()const{return m_base == NIL;}
bool type_id::is_boolean()const{return m_base == BOOLEAN;}
bool type_id::is_number()const{return m_base == NUMBER;}
bool type_id::is_string()const{return m_base == STRING;}
bool type_id::is_object()const{return m_base == OBJECT;}

type_id::base_id type_id::base()const{return m_base;}
type_id::sub_id type_id::sub()const{return m_sub;}

} //namespace script
} //namespace fungu
