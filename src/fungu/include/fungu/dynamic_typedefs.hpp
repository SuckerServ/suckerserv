/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_DYNAMIC_TYPEDEFS_HPP
#define FUNGU_DYNAMIC_TYPEDEFS_HPP

#include <boost/type_traits.hpp>

namespace fungu{

class missing_args:public std::exception
{
public:
    missing_args(int given,int needed)throw()
     :m_given(given),m_needed(needed){}
    ~missing_args()throw(){}
    const char * what()const throw(){return "";}
private:
    int m_given;
    int m_needed;
};    

namespace detail{

template<int N> struct arity_tag{};

template<typename T> 
struct arg_holder{
    typedef typename boost::mpl::if_<
        boost::mpl::bool_<boost::is_reference<T>::value>,
        typename boost::remove_const<typename boost::remove_reference<T>::type>::type,
        T
    >::type type;
};

} //namespace detail
} //namespace fungu

#endif
