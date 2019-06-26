/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

callargs::callargs(std::vector<value_type> & args)
 :m_arguments(args),m_front(m_arguments.begin())
{
    
}

void callargs::push_back(const value_type & value)
{
    int index = m_front - m_arguments.begin();
    m_arguments.push_back(value);
    m_front = m_arguments.begin() + index;
}

callargs::value_type & callargs::front()
{
    return *m_front;
}

callargs::value_type & callargs::back()
{
    return m_arguments.back();
}

any & callargs::front_reference()
{
    return *m_front;
}

callargs::value_type & callargs::safe_front()
{
    if(empty()) throw error(NOT_ENOUGH_ARGUMENTS);
    return *m_front;
}

void callargs::pop_front()
{
    assert(m_front < m_arguments.end());
    m_front++;
}

std::size_t callargs::size()const
{
    return m_arguments.size() - (m_front - m_arguments.begin());
}

bool callargs::empty()const
{
    return !size();
}

std::vector<any> callargs_serializer::dummyvector;
callargs callargs_serializer::dummycallargs(callargs_serializer::dummyvector);

callargs_serializer::callargs_serializer()
 :m_argv(dummycallargs), m_frame(NULL)
{
}

callargs_serializer::callargs_serializer(callargs & argv,env_frame * frame)
 :m_argv(argv),m_frame(frame)
{
    
}

any callargs_serializer::serialize(any value)
{
    return value;
}

void callargs_serializer::deserialize_return_value(const serialized_argument_type &, type_tag<void>)
{
    
}

callargs_serializer::serialized_argument_type & callargs_serializer::deserialize(callargs_serializer::serialized_argument_type & value, type_tag<serialized_argument_type>)
{
    return value;
}

const char * callargs_serializer::deserialize(const serialized_argument_type & value, type_tag<const char *>)
{
    m_string_list.push_back(value.to_string().copy());
    return m_string_list.back().begin();
}

code_block callargs_serializer::deserialize(const serialized_argument_type &, type_tag<code_block>)
{
    any & arg = m_argv.front_reference();
    if(arg.get_type() != typeid(code_block))
    {
        arg = code_block(arg.to_string(), m_frame->get_env()->get_source_context());
        any_cast<code_block>(&arg)->compile(m_frame);
    }
    return any_cast<code_block>(arg);
}

callargs_serializer::return_type callargs_serializer::get_void_value()
{
    return any::null_value();
}

void callargs_serializer::clear()
{
    m_string_list.clear();
}

} //namespace script
} //namespace fungu
