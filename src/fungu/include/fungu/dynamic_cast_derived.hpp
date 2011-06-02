/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_DYNAMIC_CAST_DERIVED_HPP
#define FUNGU_DYNAMIC_CAST_DERIVED_HPP

#include <typeinfo>
#include <boost/type_traits/remove_pointer.hpp>

namespace fungu{

template<typename DerivedType,typename BaseType>
DerivedType dynamic_cast_derived(BaseType * ptr)
{
    if(ptr && typeid(*ptr) == typeid(typename boost::remove_pointer<DerivedType>::type))
        return static_cast<DerivedType>(ptr);
    else return NULL;
}

} //namespace fungu

#endif
