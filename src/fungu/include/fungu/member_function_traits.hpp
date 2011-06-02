/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_MEMBER_FUNCTION_HPP
#define FUNGU_MEMBER_FUNCTION_HPP

#include <boost/type_traits/function_traits.hpp>

namespace fungu{

template<typename> struct member_function_traits;
template<typename F,typename Class>
struct member_function_traits<F Class::*> : boost::function_traits<F>
{
    typedef Class class_type;
    typedef F function_type;
};

} //end of namespace fungu

#endif
