#ifndef LUA_FUNCTION_HPP
#define LUA_FUNCTION_HPP

#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>

#include "push_function.hpp"

namespace lua{

struct call_signature_binds
{
    lua_State * L;
    int results;
    int error_function;
};

template<typename FT>
struct call0_signature:public call_signature_binds
{
    void operator()()
    {
         lua_pcall(L, 0, results, error_function);
    }
};

template<typename FT>
struct call1_signature{typedef typename FT::result_type (type)(typename FT::arg1_type);};

template<typename FT>
struct call2_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type);};

template<typename FT>
struct call3_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type);};

template<typename FT>
struct call4_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type,typename FT::arg4_type);};

template<typename FT>
struct call5_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type,typename FT::arg4_type,typename FT::arg5_type);};

template<typename FT>
struct call6_signature{typedef typename FT::result_type (type)(typename FT::arg1_type,typename FT::arg2_type,typename FT::arg3_type,typename FT::arg4_type,typename FT::arg5_type,typename FT::arg6_type);};

template<typename FT>
struct call_signature: 
    boost::mpl::at<
        boost::mpl::vector<
            detail::call0_signature<FT>,
            detail::call1_signature<FT>,
            detail::call2_signature<FT>,
            detail::call3_signature<FT>,
            detail::call4_signature<FT>,
            detail::call5_signature<FT>,
            detail::call6_signature<FT>
        >, boost::mpl::int_<FT::arity> >::type {};

} //namespace fungu
} //namespace lua

#endif

