/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_TUPLE_SETTER_HPP
#define FUNGU_SCRIPT_TUPLE_SETTER_HPP

#include "env.hpp"
#include <boost/tuple/tuple.hpp>

namespace fungu{
namespace script{
    
template<typename Tuple>
class tuple_setter:public env_object
{
public:
    tuple_setter()
      :m_set(false)
    {
        
    }
    
    any call(call_arguments & args,env_frame * aFrame)
    {
        read_next_element(args, m_tuple);
        return any::null_value();
    }
    
    any value()
    {
        return get_shared_ptr();
    }
    
    void reset()
    {
        m_set = false;
        m_tuple = Tuple();
    }
    
    bool is_set()const{return m_set;}
    const Tuple & get_tuple()const{return m_tuple;}
private:
    
    template<typename HT>
    void read_next_element(call_arguments & args, boost::tuples::cons<HT,boost::tuples::null_type> & t)
    {
        t.head = args.safe_casted_front<typename boost::tuples::cons<HT,boost::tuples::null_type>::head_type>();
        args.pop_front();
        m_set = true;
    }
    
    template<typename HT,typename TT>
    void read_next_element(call_arguments & args, boost::tuples::cons<HT,TT> & t)
    {
        t.head = args.safe_casted_front<typename boost::tuples::cons<HT,boost::tuples::null_type>::head_type>();
        args.pop_front();
        read_next_element(args, t.tail);
    }
    
    bool m_set;
    Tuple m_tuple;
};

} //namespace script
} //namespace fungu

#endif
