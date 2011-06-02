/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

class table_iterator:public env_object::member_iterator
{
    typedef std::map<std::string,env_object::shared_ptr>::const_iterator internal_iterator;
public:
    table_iterator(internal_iterator it, internal_iterator end)
     :m_it(it),m_end(end)
    {
        
    }
    
    const_string get_name()const
    {
        return m_it->first;
    }
    
    env_object * get_object()const
    {
        return m_it->second.get();
    }
    
    bool next()
    {
        assert(m_it != m_end);
        ++m_it;
        return m_it != m_end;
    }
private:
    internal_iterator m_it;
    internal_iterator m_end;
};

table::table()
{
    //m_members[".this"] = this->get_shared_ptr();
}

table::~table()
{
    //m_members[".this"].reset();
}

env_object::shared_ptr table::create()
{
    table * t = new table();
    t->set_adopted();
    return t->get_shared_ptr();
}

any table::call(call_arguments & args,frame *)
{
    env_object::shared_ptr obj = this;
    while(!args.empty())
    {
        const_string member_id = args.casted_front<const_string>();
        obj = obj->lookup_member(member_id);
        args.pop_front();
        if(!obj) throw error(UNKNOWN_SYMBOL,boost::make_tuple(member_id.copy()));
    }
    return obj->get_shared_ptr();
}

env_object::shared_ptr table::assign(const std::string & name,const any & data)
{
    any_variable * anyvar = new any_variable;
    anyvar->set_adopted();
    anyvar->assign(data);
    m_members[name] = anyvar->get_shared_ptr();
    return anyvar->get_shared_ptr();
}

void table::assign(const any & source)
{
    if(source.get_type() != typeid(shared_ptr)) throw error(NO_CAST);
    const table * source_table = dynamic_cast<const table *>(any_cast<shared_ptr>(source).get());
    if(!source_table) throw error(NO_CAST);
    m_members = source_table->m_members;
}

env_object * table::lookup_member(const_string id)
{
    map::iterator it = m_members.find(id.copy());
    if(it == m_members.end()) return NULL;
    return it->second.get();
}

bool table::erase_member(const std::string & name)
{
    return m_members.erase(name);
}

env_object::member_iterator * table::first_member()const
{
    if(m_members.empty()) return NULL;
    return new table_iterator(m_members.begin(), m_members.end());
}

} //namespace script
} //namespace fungu
