#ifndef FUNGU_SCRIPT_CORE_CONSTRUCTS_HPP
#define FUNGU_SCRIPT_CORE_CONSTRUCTS_HPP

#include "construct.hpp"
#include "expression.hpp"
#include "any.hpp"
#include "env.hpp"
#include "env_object.hpp" //env_object used by symbol class
#include "env_frame.hpp"

namespace fungu{
namespace script{

class subexpression:public construct
{
public:
    subexpression();
    ~subexpression();
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame);
    any eval(env_frame * frame);
    bool is_string_constant()const;
    std::string form_source()const;
private:
    expression * m_parsing;
    expression * m_first_expr;
};

template<typename ExitTerminals>
class word:public construct
{
public:
    word()
     :m_first(const_string::null_const_iterator())
    {
        
    }
    
    word(const_string w)
     :m_first(w.begin()),
      m_last(w.end()-1)
    {
        
    }
    
    parse_state parse(source_iterator * first,source_iterator last,env_frame *)
    {
        if(m_first == const_string::null_const_iterator())
        m_first = *first;
        
        for(; *first <= last; ++(*first) )
        {
            if(ExitTerminals::is_member(**first)) return PARSE_COMPLETE;
            m_last = (*first); // the reason for assigning m_last on each
            //iteration, instead of assigning once when parsing is complete, is
            //to allow exist terminal symbols to be stored outside of the
            //source buffer.
        }
        
        return PARSE_PARSING;    
    }
    
    any eval(env_frame * frame)
    {
        assert(m_first != const_string::null_const_iterator());
        return const_string(m_first,m_last);    
    }
    
    bool is_string_constant()const
    {
        return true;
    }
    
    std::string form_source()const
    {
        assert(m_first != const_string::null_const_iterator());
        return const_string(m_first,m_last).copy();
    }
private:
    source_iterator m_first;
    source_iterator m_last;
};

template<typename ExitTerminals>
class symbol:public construct
{
public:
    symbol()
     : m_first(const_string::null_const_iterator()),
       m_symbol(NULL)
    {
        m_members[0] = NULL;
        m_members[1] = NULL;
    }
    
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame)
    {
        if(m_first == const_string::null_const_iterator())
        {
            if(**first == '.' || ExitTerminals::is_member(**first)) 
                throw error(UNEXPECTED_SYMBOL,boost::make_tuple(**first));
            
            m_first = *first;
            m_members[0] = m_first;
        }
        
        for(; *first <= last; ++(*first) )
        {
            if(ExitTerminals::is_member(**first))
            {
                if(*m_last == '.') throw error(UNEXPECTED_SYMBOL,boost::make_tuple('.'));
                m_symbol = frame->get_env()->lookup_symbol(get_member(0));
                return PARSE_COMPLETE;
            }
            else if(**first == '.') *get_memv_tail() = (*first) + 1;
            
            m_last = (*first);
        }
        
        return PARSE_PARSING;
    }
    
    any eval(env_frame * frame)
    {
        return resolve_symbol(frame)->value();
    }
    
    bool is_string_constant()const
    {
         return false;
    }
    
    std::string form_source()const
    {
        assert_parsed();
        return (std::string(1,'$') + get_full_id().copy());
    }
protected:
    void assert_parsed()const
    {
        assert(m_first != const_string::null_const_iterator());
    }
    
    const_string get_full_id()const
    {
        return const_string(m_first,m_last);
    }
    
    env_object * resolve_symbol(env_frame * frame)
    {
        assert_parsed();
        
        const_string id = get_member(0);
        
        m_symbol = (!m_symbol ? frame->get_env()->lookup_symbol(id) : m_symbol);
        if(!m_symbol) throw error(UNKNOWN_SYMBOL,boost::make_tuple(id.copy()));
        
        env_object * obj = m_symbol->lookup_object(frame);
        if(!obj) throw error(UNKNOWN_SYMBOL, boost::make_tuple(id.copy()));
        
        int members = get_memv_size();
        for(int i = 1; i < members; i++)
        {
            obj = obj->lookup_member(get_member(i));
            if(!obj) throw error(UNKNOWN_SYMBOL,boost::make_tuple(get_member(i).copy()));
        }
        
        return obj;
    }
private:
    source_iterator * get_memv_tail()
    {
        int size = sizeof(m_members) / sizeof(source_iterator);
        
        for(int i = 0; i < size; i++)
        {
            if(!m_members[i])
            {
                if(i+1 < size) m_members[i+1] = NULL;
                return &m_members[i];
            }
        }
        
        throw error(MEMBER_ACCESS_CHAIN_LIMIT,boost::make_tuple(size));
    }
    
    int get_memv_size()const
    {
        int count = 0;
        int size = sizeof(m_members) / sizeof(source_iterator);
        for(int i = 0; i < size; i++) if(m_members[i]) count++; else break;
        return count;
    }
    
    const_string get_member(int i)const
    {
        assert(i < get_memv_size());
        int size = sizeof(m_members) / sizeof(source_iterator);
        return const_string(m_members[i],(i+1 == size-1 || !m_members[i+1] ? m_last : m_members[i+1] - 2));
    }
    
    source_iterator m_first;
    source_iterator m_last;
    source_iterator m_members[6];
    env_symbol * m_symbol;
};

template<typename ExitTerminals>
class reference:public symbol<ExitTerminals>
{
public:
    any eval(env_frame * frame)
    {
        return symbol<ExitTerminals>::resolve_symbol(frame)->get_shared_ptr();
    }
    
    std::string form_source()const
    {
        symbol<ExitTerminals>::assert_parsed();
        return (std::string(1,'@') + symbol<ExitTerminals>::get_full_id().copy());
    }
};

class quote:public construct
{
public:
    quote();
    parse_state parse(source_iterator * first,source_iterator last,env_frame *);
    any eval(env_frame *);
    bool is_string_constant()const;
    std::string form_source()const;
private:
    std::string m_string;
    bool m_escaped;
};

class macro:public construct
{
public:
    macro();
    ~macro();
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame);
    any eval(env_frame * frame);
    int get_evaluation_level()const;
    bool is_string_constant()const;
    std::string form_source()const;
private:
    int m_escape;
    construct * m_con;
};

class block:public construct
{
public:
    block();
    ~block();
    parse_state parse(source_iterator * first,source_iterator last,env_frame * frame);
    any eval(env_frame *);
    bool is_string_constant()const;
    std::string form_source()const;
private:
    source_iterator m_first;
    source_iterator m_last;
    std::vector<boost::tuple<source_iterator,source_iterator,macro *> > m_macros;
    int m_nested;
};

class comment:public construct
{
public:
    comment();
    parse_state parse(source_iterator * first,source_iterator last,env_frame *);
    any eval(env_frame *);
    bool is_eval_supported()const;
    bool is_string_constant()const;
    std::string form_source()const;
private:
    static inline bool is_terminator(const_string::value_type c);
private:
    source_iterator m_first;
    source_iterator m_last;
};

} //namespace script
} //namespace fungu

#endif
