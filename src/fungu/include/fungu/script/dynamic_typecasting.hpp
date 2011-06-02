/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_DYNAMIC_TYPECASTING_HPP
#define FUNGU_SCRIPT_DYNAMIC_TYPECASTING_HPP

#include "error.hpp"
#include "../type_tag.hpp"

#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/logical.hpp>

namespace fungu{
namespace script{

template<typename T,typename S>
struct good_numeric_conversion_traits
{
    T numeric_cast(S value){return boost::numeric_cast<T>(value);}
};

template<typename T,typename S>
struct bad_numeric_conversion_traits
{
    T numeric_cast(S value){throw boost::numeric::bad_numeric_cast();}
};

template<typename T,typename S>
struct get_numeric_conversion_traits
{
    typedef typename boost::mpl::if_<
          boost::mpl::and_< boost::mpl::bool_<boost::is_arithmetic<T>::value>
            , boost::mpl::bool_<boost::is_arithmetic<S>::value> >
        , good_numeric_conversion_traits<T,S>
        , bad_numeric_conversion_traits<T,S> >::type type;
};

class dynamic_typecaster
{
public:
    virtual ~ dynamic_typecaster(){}
    
    template<typename T> T      numeric_cast(type_tag<T>)const{throw boost::numeric::bad_numeric_cast();}
    virtual signed char         numeric_cast(type_tag<signed char>)const = 0;
    virtual signed short        numeric_cast(type_tag<signed short>)const = 0;
    virtual signed int          numeric_cast(type_tag<signed int>)const = 0;
    virtual signed long int     numeric_cast(type_tag<signed long int>)const = 0;
    virtual unsigned char       numeric_cast(type_tag<unsigned char>)const = 0;
    virtual unsigned short      numeric_cast(type_tag<unsigned short>)const = 0;
    virtual unsigned int        numeric_cast(type_tag<unsigned int>)const = 0;
    virtual unsigned long int   numeric_cast(type_tag<unsigned long int>)const = 0;
    virtual float               numeric_cast(type_tag<float>)const = 0;
    virtual double              numeric_cast(type_tag<double>)const = 0;
    virtual long double         numeric_cast(type_tag<long double>)const = 0;
    virtual bool                numeric_cast(type_tag<bool>)const = 0;
    virtual char                numeric_cast(type_tag<char>)const = 0;
    virtual wchar_t             numeric_cast(type_tag<wchar_t>)const = 0;
};

template<typename SourceType>
class derived_dynamic_typecaster:public dynamic_typecaster
{
public:
    derived_dynamic_typecaster():m_value(NULL){}//TODO remove
    derived_dynamic_typecaster(const SourceType * value):m_value(value){}
    
    signed char numeric_cast(type_tag<signed char>) const{return typename get_numeric_conversion_traits<signed char,SourceType>::type().numeric_cast(*m_value);}
    signed short numeric_cast(type_tag<signed short>) const{return typename get_numeric_conversion_traits<signed short,SourceType>::type().numeric_cast(*m_value);}
    signed int numeric_cast(type_tag<signed int>) const{return typename get_numeric_conversion_traits<signed int,SourceType>::type().numeric_cast(*m_value);}
    signed long int numeric_cast(type_tag<signed long int>) const{return typename get_numeric_conversion_traits<signed long int,SourceType>::type().numeric_cast(*m_value);}
    
    unsigned char numeric_cast(type_tag<unsigned char>) const{return typename get_numeric_conversion_traits<unsigned char,SourceType>::type().numeric_cast(*m_value);}
    unsigned short numeric_cast(type_tag<unsigned short>) const{return typename get_numeric_conversion_traits<unsigned short,SourceType>::type().numeric_cast(*m_value);}
    unsigned int numeric_cast(type_tag<unsigned int>) const{return typename get_numeric_conversion_traits<unsigned int,SourceType>::type().numeric_cast(*m_value);}
    unsigned long int numeric_cast(type_tag<unsigned long int>) const{return typename get_numeric_conversion_traits<unsigned long int,SourceType>::type().numeric_cast(*m_value);}
    
    float numeric_cast(type_tag<float>) const{return typename get_numeric_conversion_traits<float,SourceType>::type().numeric_cast(*m_value);}
    double numeric_cast(type_tag<double>) const{return typename get_numeric_conversion_traits<double,SourceType>::type().numeric_cast(*m_value);}
    long double numeric_cast(type_tag<long double>) const{return typename get_numeric_conversion_traits<long double,SourceType>::type().numeric_cast(*m_value);}
    
    bool numeric_cast(type_tag<bool>) const{return typename get_numeric_conversion_traits<bool,SourceType>::type().numeric_cast(*m_value);}
    char numeric_cast(type_tag<char>) const{return typename get_numeric_conversion_traits<char,SourceType>::type().numeric_cast(*m_value);}
    wchar_t numeric_cast(type_tag<wchar_t>) const{return typename get_numeric_conversion_traits<wchar_t,SourceType>::type().numeric_cast(*m_value);}
private:
    const SourceType * m_value;
};

} //namespace script
} //namespace fungu

#endif
