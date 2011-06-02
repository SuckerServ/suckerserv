/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_SLOT_FACTORY_HPP
#define FUNGU_SCRIPT_SLOT_FACTORY_HPP

#include "../make_function_signature.hpp"
#include "script_function.hpp"
#include <boost/function.hpp>
#include <boost/signal.hpp>
#include <map>
#include <vector>
#include <list>

namespace fungu{
namespace script{
    
class slot_factory
{
public:
    typedef any (* error_handler_function)(error_trace *);
    
    slot_factory()
      :m_next_slot_id(0)
    {
        
    }

    ~slot_factory()
    {
        clear();
        deallocate_destroyed_slots();
    }
    
    void clear()
    {
        while(m_slots.size()) destroy_slot(m_slots.begin()->first);
        m_slots.clear();
    }
    
    template<typename SignalType>
    void register_signal(SignalType & sig, const_string name, error_handler_function error_handler, boost::signals::connect_position cp = boost::signals::at_back)
    {
        m_signal_connectors[name] = boost::bind(&slot_factory::connect_slot<SignalType>, this, boost::ref(sig), error_handler, cp, _1, _2);
    }
    
    int create_slot(const_string name, env_object::shared_ptr obj, env * environment)
    {
        std::map<const_string,slot_connect_function>::iterator it = m_signal_connectors.find(name);
        if(it == m_signal_connectors.end()) return -1;
        return it->second(obj, environment);
    }
    
    bool destroy_slot(int handle)
    {
        slot_map::iterator it = m_slots.find(handle);
        if(it == m_slots.end()) return false;
        m_destroyed.push_back(it->second.first);
        it->second.second.disconnect();
        m_slots.erase(it);
        return true;
    }
    
    void deallocate_destroyed_slots()
    {
        while(!m_destroyed.empty())
        {
            delete m_destroyed.front();
            m_destroyed.pop_front();
        }
    }
    
    int skip_slot_id()
    {
        return m_next_slot_id;
    }
private:
    template<typename SignalType>
    int connect_slot(SignalType & sig,
        error_handler_function error_handler,
        boost::signals::connect_position cp,
        env_object::shared_ptr obj,
        env * environment)
    {
        typedef typename make_function_signature<typename SignalType::slot_function_type>::type SignalSignature;
        
        std::pair<base_script_function *,boost::signals::connection> newSlot;
        script_function<SignalSignature> * newSlotFunction = new script_function<SignalSignature>(obj, environment, error_handler);
        
        newSlot.first = newSlotFunction;
        newSlot.second = sig.connect(boost::ref(*newSlotFunction), cp);
        
        int slot_id = m_next_slot_id;
        m_next_slot_id++;
        
        m_slots[slot_id] = newSlot;
        
        return slot_id;
    }
    
    typedef boost::function<int (env_object::shared_ptr,env *)> slot_connect_function;
    std::map<const_string,slot_connect_function> m_signal_connectors;
    
    typedef detail::base_script_function<std::vector<any>, callargs_serializer, error> base_script_function;
    typedef std::vector<std::pair<base_script_function *,boost::signals::connection> > slot_vector;
    typedef std::map<int, std::pair<base_script_function *, boost::signals::connection> > slot_map;
    slot_map m_slots;
    int m_next_slot_id;
    
    std::list<base_script_function *> m_destroyed;
};

} //namespace script
} //namespace fungu

#endif
