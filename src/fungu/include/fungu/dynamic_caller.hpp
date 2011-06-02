/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_DYNAMIC_CALLER_HPP
#define FUNGU_DYNAMIC_CALLER_HPP

#include "type_tag.hpp"
#include "dynamic_typedefs.hpp"

#include <exception>
#include <boost/function.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>
#include <boost/bind.hpp>

namespace fungu{

namespace detail{

#define FUNGU_DYNAMIC_CALLER_OPERATOR(n) \
    template<typename ArgumentContainer,typename Serializer> \
    typename ArgumentContainer::value_type operator()(ArgumentContainer & arguments,Serializer & serializer){ \
        if(arguments.size() < n ) throw missing_args(arguments.size(), n ); \
        return caller(arguments, serializer, type_tag<typename FunctionTraits::result_type>()); \
    }

#define FUNGU_DYNAMIC_CALLER_VOID_CALLER \
    template<typename ArgumentContainer,typename Serializer> \
    typename ArgumentContainer::value_type caller(ArgumentContainer & arguments,Serializer & serializer,type_tag<void>)

#define FUNGU_DYNAMIC_CALLER_NONVOID_CALLER \
    template<typename ArgumentContainer,typename Serializer,typename RetT> \
    typename ArgumentContainer::value_type caller(ArgumentContainer & arguments,Serializer & serializer,type_tag<RetT>)
    
template<typename FunctionTraits>
class dynamic_caller0
{
public:
    template<typename Functor>
    dynamic_caller0(Functor function):m_function(function){}
    template<typename ArgumentContainer,typename Serializer>
    typename ArgumentContainer::value_type operator()(ArgumentContainer & arguments,Serializer & serializer)
    {
        return caller(arguments,serializer,type_tag<typename FunctionTraits::result_type>());
    }
private:
    template<typename ArgumentContainer,typename Serializer,typename RetT>
    typename ArgumentContainer::value_type caller(ArgumentContainer & arguments,Serializer & serializer,type_tag<RetT>)
    {
        return serializer.serialize(m_function());
    }
    
    template<typename ArgumentContainer,typename Serializer>
    typename ArgumentContainer::value_type caller(ArgumentContainer & arguments,Serializer & serializer,type_tag<void>)
    {
        m_function();
        return serializer.get_void_value();
    }
    
    boost::function0<typename FunctionTraits::result_type> m_function;
};

template<typename T> 
struct argtmp{
    //typedef typename boost::remove_const<typename boost::remove_reference<ArgType>::type>::type type;
    typedef typename boost::mpl::if_<
        boost::mpl::bool_<boost::is_reference<T>::value>,
        typename boost::remove_const<typename boost::remove_reference<T>::type>::type,
        T
    >::type type;
};

#define FUNGU_DYNAMIC_CALL_ARGUMENT(name,arguments,serializer) \
    typedef typename argtmp<typename FunctionTraits::name##_type>::type name##_tmp_type; \
    name##_tmp_type name = serializer.deserialize(arguments.front(), type_tag<name##_tmp_type>()); \
    arguments.pop_front();

template<typename FunctionTraits>
class dynamic_caller1
{
public:
    template<typename Functor>
    dynamic_caller1(Functor function):m_function(function){}
    FUNGU_DYNAMIC_CALLER_OPERATOR(1);
private:
    FUNGU_DYNAMIC_CALLER_VOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        m_function(arg1);
        return serializer.get_void_value();
    }
    
    FUNGU_DYNAMIC_CALLER_NONVOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        return serializer.serialize(m_function(arg1));
    }
    
    boost::function1<typename FunctionTraits::result_type,
                     typename FunctionTraits::arg1_type > m_function;
};

template<typename FunctionTraits>
class dynamic_caller2
{
public:
    template<typename Functor>
    dynamic_caller2(Functor function):m_function(function){}
    FUNGU_DYNAMIC_CALLER_OPERATOR(2);
private:
    FUNGU_DYNAMIC_CALLER_VOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        m_function(arg1,arg2);
        return serializer.get_void_value();
    }
    
    FUNGU_DYNAMIC_CALLER_NONVOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        return serializer.serialize(m_function(arg1,arg2));
    }
    
    boost::function2<typename FunctionTraits::result_type,
                     typename FunctionTraits::arg1_type,
                     typename FunctionTraits::arg2_type > m_function;
};

template<typename FunctionTraits>
class dynamic_caller3
{
public:
    template<typename Functor>
    dynamic_caller3(Functor function):m_function(function){}
    FUNGU_DYNAMIC_CALLER_OPERATOR(3);
private:
    FUNGU_DYNAMIC_CALLER_VOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        m_function(arg1,arg2,arg3);
        return serializer.get_void_value();
    }
    
    FUNGU_DYNAMIC_CALLER_NONVOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        return serializer.serialize(m_function(arg1,arg2,arg3));
    }

    boost::function3<typename FunctionTraits::result_type,
                     typename FunctionTraits::arg1_type,
                     typename FunctionTraits::arg2_type,
                     typename FunctionTraits::arg3_type > m_function;
};

template<typename FunctionTraits>
class dynamic_caller4
{
public:
    template<typename Functor>
    dynamic_caller4(Functor function):m_function(function){}
    FUNGU_DYNAMIC_CALLER_OPERATOR(4);
private:
    FUNGU_DYNAMIC_CALLER_VOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg4, arguments, serializer);
        m_function(arg1,arg2,arg3,arg4);
        return serializer.get_void_value();
    }
    
    FUNGU_DYNAMIC_CALLER_NONVOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg4, arguments, serializer);
        return serializer.serialize(m_function(arg1,arg2,arg3,arg4));
    }
    
    boost::function4<typename FunctionTraits::result_type,
                     typename FunctionTraits::arg1_type,
                     typename FunctionTraits::arg2_type,
                     typename FunctionTraits::arg3_type,
                     typename FunctionTraits::arg4_type > m_function;
};

template<typename FunctionTraits>
class dynamic_caller5
{
public:
    template<typename Functor>
    dynamic_caller5(Functor function):m_function(function){}
    FUNGU_DYNAMIC_CALLER_OPERATOR(5);
private:
    FUNGU_DYNAMIC_CALLER_VOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg4, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg5, arguments, serializer);
        m_function(arg1,arg2,arg3,arg4,arg5);
        return serializer.get_void_value();
    }
    
    FUNGU_DYNAMIC_CALLER_NONVOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg4, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg5, arguments, serializer);
        return serializer.serialize(m_function(arg1,arg2,arg3,arg4,arg5));
    }
    
    boost::function5<typename FunctionTraits::result_type,
                     typename FunctionTraits::arg1_type,
                     typename FunctionTraits::arg2_type,
                     typename FunctionTraits::arg3_type,
                     typename FunctionTraits::arg4_type,
                     typename FunctionTraits::arg5_type> m_function;
};

template<typename FunctionTraits>
class dynamic_caller6
{
public:
    template<typename Functor>
    dynamic_caller6(Functor function):m_function(function){}
    FUNGU_DYNAMIC_CALLER_OPERATOR(6);
private:
    FUNGU_DYNAMIC_CALLER_VOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg4, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg5, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg6, arguments, serializer);
        m_function(arg1,arg2,arg3,arg4,arg5,arg6);
        return serializer.get_void_value();
    }
    
    FUNGU_DYNAMIC_CALLER_NONVOID_CALLER
    {
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg1, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg2, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg3, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg4, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg5, arguments, serializer);
        FUNGU_DYNAMIC_CALL_ARGUMENT(arg6, arguments, serializer);
        return serializer.serialize(m_function(arg1,arg2,arg3,arg4,arg5,arg6));
    }

    boost::function6<typename FunctionTraits::result_type,
                     typename FunctionTraits::arg1_type,
                     typename FunctionTraits::arg2_type,
                     typename FunctionTraits::arg3_type,
                     typename FunctionTraits::arg4_type,
                     typename FunctionTraits::arg5_type,
                     typename FunctionTraits::arg6_type> m_function;
};

#define FUNGU_DYNAMIC_CALLER_CLASS_SELECTION \
boost::mpl::at<boost::mpl::vector< \
        dynamic_caller0<boost::function_traits<Signature> >, \
        dynamic_caller1<boost::function_traits<Signature> >, \
        dynamic_caller2<boost::function_traits<Signature> >, \
        dynamic_caller3<boost::function_traits<Signature> >, \
        dynamic_caller4<boost::function_traits<Signature> >, \
        dynamic_caller5<boost::function_traits<Signature> >, \
        dynamic_caller6<boost::function_traits<Signature> > >, \
    boost::mpl::int_<boost::function_traits<Signature>::arity> >::type \

template<typename Signature>
class get_dynamic_caller_class:public FUNGU_DYNAMIC_CALLER_CLASS_SELECTION
{
public:
    template<typename Functor>
    get_dynamic_caller_class(Functor function)
     :FUNGU_DYNAMIC_CALLER_CLASS_SELECTION(function)
    {
        
    }
};

} //end of namespace detail

/**
    @brief Function object wrapper with a uniform call interface.
    
    The uniform call interface is a template function-call operator that takes
    an argument container and a Serializer object as arguments and calls
    the bound function. For each parameter of the bound function, an element
    from the argument container is deserialized and passed by value. If there 
    are not enough elements are in the argument container then a missing_args
    exception is thrown. The value returned by a function call is serialized 
    and returned by the function call operator.
    
    dynamic_call is useful where you want to expose C++ free/member functions
    to a distributed system or scripting interface without needing to use
    something like a code generator to generate upcall skeletons.

    Currently, dynamic_call can support functions with up to 6 arguments.
    
    Example of the syntax:
    @code
    //the function to be exposed
    int add(int a,int b){return a + b;}
    
    dynamic_caller<int (int,int)> dc_add(add);
    
    std::deque<std::string> args;
    args.push_back("1");
    args.push_back("2");
    
    //call the add function passing a reference to the arguments container
    //and a reference to a temporary serialization object.
    std::string answer = dc_add(args,serializer());
    @endcode
    
    @warning This class is now deprecated - use the dynamic_call function instead.
*/
template<typename Signature>
class dynamic_caller:public detail::get_dynamic_caller_class<Signature>
{
public:
    template<typename Functor>
    dynamic_caller(Functor function)
     :detail::get_dynamic_caller_class<Signature>(function)
    {
        
    }
};

} //end of namespace fungu

#endif
