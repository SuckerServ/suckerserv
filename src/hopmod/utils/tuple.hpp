#ifndef HOPMOD_UTILS_TUPLE_HPP
#define HOPMOD_UTILS_TUPLE_HPP

namespace utils
{
 template <int...> struct index_holder {};

 namespace detail
 {

     template <int IDX, class HOLDER> struct make_indexes_impl;
 
     template <int IDX, int... INDEXES> 
     struct make_indexes_impl<IDX, index_holder<INDEXES...>>
     {
         typedef typename make_indexes_impl<IDX-1, 
                                 index_holder<IDX, INDEXES...>>::type type;
     };
 
     template <int ...INDEXES>
     struct make_indexes_impl<0, index_holder<INDEXES...>>
     {
         typedef index_holder<0,INDEXES...> type;
     };

     template <class A, class...ARGS, int ...INDEXES>
     std::tuple<ARGS...> get_tail_helper(const std::tuple<A, ARGS...> &t,
                                         const index_holder<INDEXES...> &)
     {
         return std::tuple<ARGS...>(std::get<(INDEXES+1)>(t)...);
     }
 }

 template <int N>
 struct make_indexes
 {
     typedef typename detail::make_indexes_impl<N-1, index_holder<>>::type type;
 };
 
 template <>
 struct make_indexes<0>
 {
     typedef index_holder<> type;
 };
 
 template <class A, class...ARGS> 
 std::tuple<ARGS...> get_tail(const std::tuple<A, ARGS...> &t)
 {
     return detail::get_tail_helper(t, typename make_indexes<sizeof...(ARGS)>::type());
 }
}
#endif
