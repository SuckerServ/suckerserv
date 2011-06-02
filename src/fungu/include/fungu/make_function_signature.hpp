/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_MAKE_FUNCTION_SIGNATURE_HPP
#define FUNGU_MAKE_FUNCTION_SIGNATURE_HPP

#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>

namespace fungu{

namespace detail{
    
template<typename FT>
struct make_function0_signature{typedef typename FT::result_type (type)();};

template<typename FT>
struct make_function1_signature{typedef typename FT::result_type (type)(typename FT::arg1_type);};

template<typename FT>
struct make_function2_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type);};

template<typename FT>
struct make_function3_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type);};

template<typename FT>
struct make_function4_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type,typename FT::arg4_type);};

template<typename FT>
struct make_function5_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type,typename FT::arg4_type,typename FT::arg5_type);};

template<typename FT>
struct make_function6_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type,typename FT::arg4_type,typename FT::arg5_type,typename FT::arg6_type);};

} //namespace detail

template<typename FT>
struct make_function_signature : 
    boost::mpl::at<boost::mpl::vector<
            detail::make_function0_signature<FT>,
            detail::make_function1_signature<FT>,
            detail::make_function2_signature<FT>,
            detail::make_function3_signature<FT>,
            detail::make_function4_signature<FT>,
            detail::make_function5_signature<FT>,
            detail::make_function6_signature<FT> >,
        boost::mpl::int_<FT::arity> >::type {};

} //namespace fungu

#endif
