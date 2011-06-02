/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_DYNAMIC_CALL_HPP
#define FUNGU_DYNAMIC_CALL_HPP

#include "type_tag.hpp"
#include "dynamic_typedefs.hpp"
#include <exception>
#include <boost/type_traits.hpp>

namespace fungu{
namespace detail{

#define DYNAMIC_CALL_FUNCTION(nargs) \
    template<typename FunctionTraits,typename Functor,typename ArgumentsContainer,typename Serializer> \
    inline typename ArgumentsContainer::value_type dynamic_call(Functor function, ArgumentsContainer & args, \
    Serializer & serializer, type_tag2<typename FunctionTraits::result_type, FunctionTraits>, arity_tag<nargs>)

#define DYNAMIC_CALL_VOID_FUNCTION(nargs) \
    template<typename FunctionTraits,typename Functor,typename ArgumentsContainer,typename Serializer> \
    inline typename ArgumentsContainer::value_type dynamic_call(Functor function, ArgumentsContainer & args, \
    Serializer & serializer, type_tag2<void, FunctionTraits>, arity_tag<nargs>)

#define DYNAMIC_CALL_ARGUMENT(name) \
    typename arg_holder<typename FunctionTraits::name##_type>::type name = serializer.deserialize(args.front(), type_tag<typename arg_holder<typename FunctionTraits::name##_type>::type>()); \
    args.pop_front();

#define DYNAMIC_CALL_ARGS_SIZE_CHECK(n) \
    if(args.size() < n) throw missing_args(args.size(), n);

DYNAMIC_CALL_FUNCTION(0)
{
    return serializer.serialize(function());
}

DYNAMIC_CALL_VOID_FUNCTION(0)
{
    function();
    return serializer.get_void_value();
}

DYNAMIC_CALL_FUNCTION(1)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(1);
    DYNAMIC_CALL_ARGUMENT(arg1);
    return serializer.serialize(function(arg1));
}

DYNAMIC_CALL_VOID_FUNCTION(1)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(1);
    DYNAMIC_CALL_ARGUMENT(arg1);
    function(arg1);
    return serializer.get_void_value();
}

DYNAMIC_CALL_FUNCTION(2)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(2);
    DYNAMIC_CALL_ARGUMENT(arg1);
    DYNAMIC_CALL_ARGUMENT(arg2);
    return serializer.serialize(function(arg1,arg2));
}

DYNAMIC_CALL_VOID_FUNCTION(2)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(2);
    DYNAMIC_CALL_ARGUMENT(arg1);
    DYNAMIC_CALL_ARGUMENT(arg2);
    function(arg1,arg2);
    return serializer.get_void_value();
}

DYNAMIC_CALL_FUNCTION(3)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(3);
    DYNAMIC_CALL_ARGUMENT(arg1);
    DYNAMIC_CALL_ARGUMENT(arg2);
    DYNAMIC_CALL_ARGUMENT(arg3);
    return serializer.serialize(function(arg1,arg2,arg3));
}

DYNAMIC_CALL_VOID_FUNCTION(3)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(3);
    DYNAMIC_CALL_ARGUMENT(arg1);
    DYNAMIC_CALL_ARGUMENT(arg2);
    DYNAMIC_CALL_ARGUMENT(arg3);
    function(arg1,arg2,arg3);
    return serializer.get_void_value();
}

DYNAMIC_CALL_FUNCTION(4)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(4);
    DYNAMIC_CALL_ARGUMENT(arg1);
    DYNAMIC_CALL_ARGUMENT(arg2);
    DYNAMIC_CALL_ARGUMENT(arg3);
    DYNAMIC_CALL_ARGUMENT(arg4);
    return serializer.serialize(function(arg1,arg2,arg3,arg4));
}

DYNAMIC_CALL_VOID_FUNCTION(4)
{
    DYNAMIC_CALL_ARGS_SIZE_CHECK(4);
    DYNAMIC_CALL_ARGUMENT(arg1);
    DYNAMIC_CALL_ARGUMENT(arg2);
    DYNAMIC_CALL_ARGUMENT(arg3);
    DYNAMIC_CALL_ARGUMENT(arg4);
    function(arg1,arg2,arg3,arg4);
    return serializer.get_void_value();
}

} //namespace detail

template<typename Signature,typename Functor,typename ArgumentsContainer,typename Serializer>
inline
typename ArgumentsContainer::value_type dynamic_call(
    Functor function, 
    ArgumentsContainer & args,
    Serializer & serializer)
{
    typedef boost::function_traits<Signature> FunctionTraits;
    return detail::dynamic_call<FunctionTraits>(function,args,serializer,type_tag2<typename FunctionTraits::result_type, FunctionTraits>(),detail::arity_tag<FunctionTraits::arity>());
}

} //namespace fungu

#endif
