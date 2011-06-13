#ifndef HOPMOD_HANDLE_RESOLVER_HPP
#define HOPMOD_HANDLE_RESOLVER_HPP

#include <map>
#include <limits>

template<typename T>
class handle_resolver
{
public:
    typedef int handle_type;
    typedef std::map<handle_type, T> handle_map;
    
    static const handle_type INVALID_HANDLE = -1;
    
    handle_resolver(const T& default_value = T())
     :m_next_handle(-1), m_default_value(default_value)
    {
        
    }
    
    T resolve(handle_type handle)const
    {
        typename handle_map::const_iterator iter = m_handles.find(handle);
        if(iter == m_handles.end()) return m_default_value;
        return iter->second;
    }
    
    handle_type assign(const T& object)
    {
        if(m_next_handle == std::numeric_limits<handle_type>::max()) return INVALID_HANDLE;
        m_handles[++m_next_handle] = object;
        return m_next_handle;
    }
    
    void free(handle_type handle)
    {
        m_handles.erase(handle);
    }
private:
    handle_type m_next_handle;
    handle_map m_handles;
    T m_default_value;
};

#endif
