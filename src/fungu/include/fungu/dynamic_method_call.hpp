/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_DYNAMIC_METHOD_CALL_HPP
#define FUNGU_DYNAMIC_METHOD_CALL_HPP

#include "type_tag.hpp"
#include "member_function_traits.hpp"
#include "dynamic_typedefs.hpp"
#include <exception>
#include <boost/type_traits.hpp>

namespace fungu{
namespace detail{

#define DYNAMIC_METHOD_CALL_FUNCTION(nargs) \
    template<typename FunctionTraits,typename Functor,typename ArgumentsContainer,typename Serializer> \
    inline typename ArgumentsContainer::value_type dynamic_method_call(Functor function, typename FunctionTraits::class_type * object, ArgumentsContainer & args, \
    Serializer & serializer, type_tag<typename FunctionTraits::result_type>, arity_tag<nargs>)

#define DYNAMIC_METHOD_CALL_VOID_FUNCTION(nargs) \
    template<typename FunctionTraits,typename Functor,typename ArgumentsContainer,typename Serializer> \
    inline typename ArgumentsContainer::value_type dynamic_call(Functor function, typename FunctionTraits::class_type * object, ArgumentsContainer & args, \
    Serializer & serializer, type_tag<void>, arity_tag<nargs>)

#define DYNAMIC_METHOD_CALL_ARGUMENT(name) \
    typename arg_holder<typename FunctionTraits::name##_type>::type name = serializer.deserialize(args.front(), type_tag<typename arg_holder<typename FunctionTraits::name##_type>::type>()); \
    args.pop_front();

#define DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(n) \
    if(args.size() < n) throw missing_args(args.size(), n);

DYNAMIC_METHOD_CALL_FUNCTION(0)
{
    return serializer.serialize((object->*function)());
}

DYNAMIC_METHOD_CALL_VOID_FUNCTION(0)
{
    (object->*function)();
    return serializer.get_void_value();
}

DYNAMIC_METHOD_CALL_FUNCTION(1)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    return serializer.serialize((object->*function)(arg1));
}

DYNAMIC_METHOD_CALL_VOID_FUNCTION(1)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    (object->*function)(arg1);
    return serializer.get_void_value();
}

DYNAMIC_METHOD_CALL_FUNCTION(2)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(2);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg2);
    return serializer.serialize((object->*function)(arg1, arg2));
}

DYNAMIC_METHOD_CALL_VOID_FUNCTION(2)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(2);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg2);
    (object->*function)(arg1, arg2);
    return serializer.get_void_value();
}

DYNAMIC_METHOD_CALL_FUNCTION(3)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(3);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg2);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg3);
    return serializer.serialize((object->*function)(arg1, arg2, arg3));
}

DYNAMIC_METHOD_CALL_VOID_FUNCTION(3)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(3);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg2);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg3);
    (object->*function)(arg1, arg2, arg3);
    return serializer.get_void_value();
}

DYNAMIC_METHOD_CALL_FUNCTION(4)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(4);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg2);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg3);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg4);
    return serializer.serialize((object->*function)(arg1, arg2, arg3, arg4));
}

DYNAMIC_METHOD_CALL_VOID_FUNCTION(4)
{
    DYNAMIC_METHOD_CALL_ARGS_SIZE_CHECK(4);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg1);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg2);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg3);
    DYNAMIC_METHOD_CALL_ARGUMENT(arg4);
    (object->*function)(arg1, arg2, arg3, arg4);
    return serializer.get_void_value();
}

} //namespace detail

template<typename Signature,typename Functor,typename ArgumentsContainer,typename Serializer>
inline typename ArgumentsContainer::value_type 
dynamic_method_call(
    Functor function,
    typename member_function_traits<Signature>::class_type * object,
    ArgumentsContainer & args,
    Serializer & serializer)
{
    typedef member_function_traits<Signature> FunctionTraits;
    return detail::dynamic_method_call<FunctionTraits>(
        function,
        object,
        args,
        serializer,
        type_tag<typename FunctionTraits::result_type>(),
        detail::arity_tag<FunctionTraits::arity>());
}

} //namespace fungu

#endif
