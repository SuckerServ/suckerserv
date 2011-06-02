/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{

code_block::iterator::iterator(expression * expr)
 :m_expr(expr)
{

}
    
expression * code_block::iterator::operator->()
{
    return m_expr;
}

expression & code_block::iterator::operator*()
{
    return *m_expr;
}

expression * code_block::iterator::get()
{
    return m_expr;
}

code_block::iterator & code_block::iterator::operator++()
{
    m_expr = static_cast<expression *>(m_expr -> get_next_sibling());
    return *this;
}

bool code_block::iterator::operator==(const code_block::iterator & comparison)const
{
    return m_expr == comparison.m_expr;
}

bool code_block::iterator::operator!=(const code_block::iterator & comparison)const
{
    return m_expr != comparison.m_expr;
}

code_block::code_block()
 :m_source_ctx(NULL)
{
    assert(false);
}

code_block::code_block(const_string source,const source_context * src_ctx)
 :m_source(source.copy()),
  m_source_ctx(src_ctx ? src_ctx->clone() : new local_source_context)
{
    
}

code_block::code_block(const code_block & src)
 :m_source(src.m_source),
  m_first_expression(src.m_first_expression),
  m_source_ctx(src.m_source_ctx->clone())
{
    
}

code_block::~code_block()
{
    delete m_source_ctx;
}

code_block & code_block::operator=(const code_block & src)
{
    delete m_source_ctx;
    
    m_source = src.m_source;
    m_first_expression = src.m_first_expression;
    m_source_ctx = src.m_source_ctx->clone();
    
    return *this;
}

code_block code_block::temporary_source(const_string source,const source_context * src_ctx)
{
    code_block tmp(const_string(),NULL);
    tmp.m_source = source;
    //tmp.m_source_ctx = src_ctx ? src_ctx->clone() : new local_source_context;
    return tmp;
}

code_block & code_block::compile(env_frame * frm)
{
    const source_context * last_context = frm->get_env()->get_source_context();
    frm->get_env()->set_source_context(m_source_ctx);
    int first_line = m_source_ctx->get_line_number();
    
    expression * current = new base_expression;
    expression * before_current = NULL;
    
    const_string::const_iterator readptr = m_source.begin();
    
    BOOST_SCOPE_EXIT((&frm)(&last_context)(&m_source_ctx)(&first_line)(&current))
    {
        frm->get_env()->set_source_context(last_context);
        m_source_ctx->set_line_number(first_line);
        delete current;
    } BOOST_SCOPE_EXIT_END
    
    while(readptr != m_source.end())
    {
        if(current->parse(&readptr, m_source.end()-1, frm) != PARSE_COMPLETE)
        {
            const char * lf = "\n";
            if(current->parse(&lf,lf,frm) != PARSE_COMPLETE) throw error(UNEXPECTED_EOF);
        }
        
        if(!current->is_empty_expression())
        {
            if(before_current) before_current->set_next_sibling(current);
            else m_first_expression = boost::shared_ptr<expression>(current);
            before_current = current;
        }
        else delete current;
        
        if(scan_newline(&(--readptr)))
            m_source_ctx->set_line_number(m_source_ctx->get_line_number()+1);
        else readptr++;
        
        current = new base_expression;
    }
    
    return *this;
}

bool code_block::should_compile()
{
    return !m_first_expression;
}

void code_block::destroy_compilation()
{
    m_first_expression = boost::shared_ptr<expression>();
}

code_block::iterator code_block::begin()
{
    return iterator(m_first_expression.get());
}

code_block::iterator code_block::end()
{
    return iterator(NULL);
}

any code_block::eval_each_expression(env_frame * frm)
{
    any last_result;
    for(construct * e = m_first_expression.get(); e; e = e->get_next_sibling() )
    {
        last_result = e->eval(frm);
        if(frm->has_expired()) break;
    }
    if(!frm->get_result_value().empty()) return frm->get_result_value();
    else return last_result;
}

any code_block::value()const
{
    return m_source;
}

const source_context * code_block::get_source_context()const
{
    return m_source_ctx;
}

} //namespace script
} //namespace fungu
