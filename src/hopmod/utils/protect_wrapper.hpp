#ifndef HOPMOD_UTILS_PROTECT_WRAPPER_HPP
#define HOPMOD_UTILS_PROTECT_WRAPPER_HPP

#include <functional>

template<typename T>
struct protect_wrapper : T
{
    protect_wrapper(const T& t) : T(t)
    {

    }

    protect_wrapper(T&& t) : T(std::move(t))
    {

    }
};

template<typename T>
typename std::enable_if< !std::is_bind_expression< typename std::decay<T>::type >::value,
                T&& >::type
protect(T&& t)
{
    return std::forward<T>(t);
}

template<typename T>
typename std::enable_if< std::is_bind_expression< typename std::decay<T>::type >::value,
                protect_wrapper<typename std::decay<T>::type > >::type
protect(T&& t)
{
    return protect_wrapper<typename std::decay<T>::type >(std::forward<T>(t));
}

#endif
