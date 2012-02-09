#ifndef HOPMOD_PREFIX_TREE_HPP
#define HOPMOD_PREFIX_TREE_HPP

#include "address_prefix.hpp"

namespace hopmod{
namespace ip{

template<typename T, T default_value = T()>
class address_prefix_tree
{
public:
    address_prefix_tree()
      :m_value(default_value)
    {
        m_child[0] = NULL;
        m_child[1] = NULL;
    }
    
    ~address_prefix_tree()
    {
        delete m_child[0];
        delete m_child[1];
    }
    
    void insert(const address_prefix & prefix, T value)
    {
        address_prefix common_prefix = m_prefix.common_prefix(prefix);
        
        if(common_prefix.mask().value() < m_prefix.mask().value())
        {
            address_prefix_tree<T, default_value> * successor = new address_prefix_tree<T, default_value>;
            
            successor->m_prefix = m_prefix;
            successor->m_prefix <<= common_prefix.mask();
            
            successor->m_value = m_value;
            successor->m_child[0] = m_child[0];
            successor->m_child[1] = m_child[1];
            
            m_child[0] = NULL;
            m_child[1] = NULL;
            m_value = default_value;
            
            m_child[successor->m_prefix.value().first_bit()] = successor;
        }
        
        m_prefix = common_prefix;
        
        address_prefix child_prefix = prefix;
        child_prefix <<= common_prefix.mask();
        
        if(!child_prefix.mask().value())
        {
            m_value = value;
            return;
        }
        
        bool first_bit = static_cast<bool>(child_prefix.value().first_bit());
        if(m_child[first_bit]) m_child[first_bit]->insert(child_prefix, value);
        else
        {
            m_child[first_bit] = new address_prefix_tree<T, default_value>;
            m_child[first_bit]->m_prefix = child_prefix;
            m_child[first_bit]->m_value = value;
        }
    }
    
    void erase(const address_prefix & src_prefix)
    {
        address_prefix prefix = src_prefix;
        
        address_prefix_tree<T, default_value> * predecessor = NULL;
        address_prefix_tree<T, default_value> * current = next_match(&prefix, &predecessor);
        address_prefix_tree<T, default_value> * last = NULL;
        
        while(current)
        {
            last = current;
            current = current->next_match(&prefix, &predecessor);
        }
        if(!last) return;
        
        last->m_value = default_value;
        
        bool first_child = static_cast<bool>(last->m_child[0]);
        bool second_child = static_cast<bool>(last->m_child[1]);
        
        if(first_child || second_child)
        {
            address_prefix_tree<T, default_value> * tmp = (first_child ? last->m_child[0] : last->m_child[1]);
            
            last->m_child[0] = tmp->m_child[0];
            last->m_child[1] = tmp->m_child[1];
            last->m_value = tmp->m_value;
            
            address::integral_type combined_address = last->m_prefix.value().value();
            combined_address |= (tmp->m_prefix.value() >> last->m_prefix.mask().bits()).value();
            
            last->m_prefix = address_prefix(combined_address, address_mask(last->m_prefix.mask().bits() + tmp->m_prefix.mask().bits()));
            
            tmp->m_child[0] = NULL;
            tmp->m_child[1] = NULL;
            delete tmp;
        }
        else if(!first_child && !second_child)
        {
            predecessor->m_child[(predecessor->m_child[0] == last ? 0 : 1)] = NULL;
            delete last;
        }
    }
    
    address_prefix_tree<T, default_value> * next_match(address_prefix * prefix, address_prefix_tree<T, default_value> ** predecessor = NULL)
    {
        address_prefix_tree<T, default_value> * parent = this;
        address_prefix_tree<T, default_value> * child = this;
        
        while( (child = child->m_child[prefix->value().first_bit()]) )
        {
            address_prefix common_prefix = child->m_prefix.common_prefix(*prefix);
            
            if(common_prefix.mask() == child->m_prefix.mask())
            {
                (*prefix) <<= common_prefix.mask();
                
                if(child->m_value != default_value)
                {
                    if(predecessor) *predecessor = parent;
                    return child;
                }
            }
            else return NULL;
            
            parent = child;
        }
        
        return NULL;
    }
    
    T value()const{return m_value;}
private:
    address_prefix_tree<T, default_value> * m_child[2];
    T m_value;
    address_prefix m_prefix;
};

} //namespace ip
} //namespace hopmod

#endif
