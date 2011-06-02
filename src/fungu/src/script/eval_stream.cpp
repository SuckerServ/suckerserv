/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

static void default_evaluator(expression * expr, env_frame * frm, void *)
{
    expr->eval(frm);
}

eval_stream::eval_stream(env_frame * frm, evaluation_function evaluator, void * evaluator_closure)
 :m_evaluator(evaluator),
  m_evaluator_closure(evaluator_closure),
  m_frame(frm),
  m_parsing(false),
  m_buffer_use(&m_first_buffer[0]),
  m_dynbuf_size(0)
{
    if(evaluator == NULL)
    {
        m_evaluator = default_evaluator;
        m_evaluator_closure = NULL;
    }
    
    reset_buffer();
}

eval_stream::~eval_stream()
{
    reset_expression();
    reset_buffer();
}

void eval_stream::feed(const void * data, std::size_t bytesLength)
{
    if(would_overflow_buffer(bytesLength)) expand_buffer(bytesLength);
    
    if(!m_parsing && bytesLength) m_parsing = true;
    
    memcpy(m_buffer_write, data, bytesLength);
    m_buffer_write += bytesLength;
    
    const char * start_of_read = m_buffer_read;
    bool completed = false;
    
    try
    {
        if(m_expression.parse(&m_buffer_read, m_buffer_write - 1, m_frame) == PARSE_COMPLETE)
        {
            if(m_expression.is_empty_expression() == false) 
                m_evaluator(&m_expression, m_frame, m_evaluator_closure);
            
            completed = true;
        }
    }
    catch(...)
    {
        reset();
        throw;
    }
    
    if(completed)
    {
        std::size_t consumed = m_buffer_read - start_of_read;
        std::size_t remaining = bytesLength - consumed;
        
        reset();
        
        if(remaining && !m_frame->has_expired())
            return feed(static_cast<const char *>(data) + consumed, remaining);
    }
}

bool eval_stream::is_parsing_expression()const
{
    return m_parsing;
}

bool eval_stream::using_dynamic_buffer()const
{
    return m_dynbuf_size > 0;
}

bool eval_stream::would_overflow_buffer(std::size_t bytesLength)const
{
    const char * futureWrite = m_buffer_write + bytesLength;
    
    if(using_dynamic_buffer())
        return futureWrite > m_buffer_use + (m_dynbuf_size - 1);
    else
        return futureWrite > &m_first_buffer[InitialBufSize - 1];
}

void eval_stream::reset_buffer()
{
    if(using_dynamic_buffer())
    {
        free(m_buffer_use);
        m_dynbuf_size = 0;
    }
    
    m_buffer_use = &m_first_buffer[0];
    m_buffer_read = m_buffer_use;
    m_buffer_write = m_buffer_use;
}

void eval_stream::expand_buffer(std::size_t length)
{
    bool copy_first_buffer = false;
    
    std::size_t read_offset = m_buffer_read - m_buffer_use;
    std::size_t write_offset = m_buffer_write - m_buffer_use;
    
    if(!using_dynamic_buffer())
    {
        m_buffer_use = NULL;
        copy_first_buffer = true;
    }
    
    m_dynbuf_size += length + InitialBufSize;
    m_buffer_use = (char *)realloc(m_buffer_use, m_dynbuf_size);
    
    if(copy_first_buffer) memcpy(m_buffer_use, &m_first_buffer[0], InitialBufSize);
    
    m_buffer_read = m_buffer_use + read_offset;
    m_buffer_write = m_buffer_use + write_offset;
    
    /* The existing expression object contains dangling pointers to the old
       buffer allocation; we must re-parse everything again with a new
       expression object.
    */
    reset_expression();
    const char * reread = m_buffer_use;
    m_expression.parse(&reread, m_buffer_read - 1, m_frame);
    assert(reread == m_buffer_read);
}

void eval_stream::reset_expression()
{
    m_expression.~base_expression();
    new (&m_expression) base_expression;
    m_parsing = false;
}

void eval_stream::reset()
{
    reset_expression();
    reset_buffer();
}

} //namespace script
} //namespace fungu
