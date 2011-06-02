/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_SCRIPT_CODE_BLOCK_HPP
#define FUNGU_SCRIPT_CODE_BLOCK_HPP

#include "script_fwd.hpp"
#include <boost/shared_ptr.hpp>

namespace fungu{
namespace script{
    
class code_block
{
public:
    class iterator
    {
    public:
        iterator(expression * expr);
        expression * operator->();
        expression & operator*();
        expression * get();
        iterator & operator++();
        bool operator==(const iterator & comparison)const;
        bool operator!=(const iterator & comparison)const;
    private:
        expression * m_expr;
    };
    
    //DO NOT USE - it's only here because a default constructor is required for the class to be used by the any class.
    code_block();
    code_block(const_string source,const source_context * src_ctx = NULL);
    code_block(const code_block & src);
    ~code_block();
    
    code_block & operator=(const code_block & src);
    
    static code_block temporary_source(const_string source,const source_context * src_ctx);
    
    /**
        @brief Compile the block of code.
    
        Compiling the block of code means parsing and storing the allocated
        expression objects in a container.
    */
    code_block & compile(env_frame * aScope);
    bool should_compile();
    void destroy_compilation();
    
    iterator begin();
    iterator end();
    
    any eval_each_expression(env_frame * aScope);

    any value()const;
    
    const source_context * get_source_context()const;
private:
    const_string m_source;
    boost::shared_ptr<expression> m_first_expression;
    source_context * m_source_ctx;
};

} //namespace script
} //namespace fungu

#endif
