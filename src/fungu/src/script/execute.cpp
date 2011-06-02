/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.h"
#endif

#include "fungu/stringutils.hpp"

#include "fungu/script/env.hpp"
#include "fungu/script/expression.hpp"
#include "fungu/script/execute.hpp"
#include "fungu/script/error.hpp"
#include "fungu/script/code_block.hpp"
#include "fungu/script/constant.hpp"

#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <boost/scope_exit.hpp>

#include "code_block.cpp"
#include "eval_stream.cpp"
#include "error.cpp"

namespace fungu{
namespace script{

namespace execute_detail{

typedef script::eval_stream file_eval_stream;

} //namespace execute_detail

void execute_file(const char * filename, env & environment)
{
    FILE * file_stream = fopen(filename,"r");
    if(!file_stream)
    {
        std::string description;
        description  = "unable to open file ";
        description += filename;
        description += ":";
        description += strerror(errno);
        
        throw error(OPERATION_ERROR, boost::make_tuple(description));
    }
    
    file_source_context file_context(filename);
    const source_context * prev_context = environment.get_source_context();
    environment.set_source_context(&file_context);
    
    BOOST_SCOPE_EXIT((&environment)(&prev_context)(&file_stream))
    {
        environment.set_source_context(prev_context); \
        fclose(file_stream);
    } BOOST_SCOPE_EXIT_END
    
    env_frame file_frame(&environment);

    constant<const char *> filename_const(filename);
    filename_const.set_temporary();
    file_frame.bind_object(&filename_const, FUNGU_OBJECT_ID("FILENAME"));
    
    execute_detail::file_eval_stream reader(&file_frame);
    
    char line[1024];
    int line_number = 1;
    
    while(!file_frame.has_expired() && !feof(file_stream) && !ferror(file_stream))
    {
        if(!fgets(line, sizeof(line), file_stream)) continue;
        reader.feed(line, strlen(line) + feof(file_stream));
        file_context.set_line_number(++line_number);
    }
    
    if(reader.is_parsing_expression()) throw error(UNEXPECTED_EOF);
    
    if(ferror(file_stream))
    {
        std::string description;
        description  = "error reading from file ";
        description += filename;
        description += ":";
        description += strerror(errno);
        
        throw error(OPERATION_ERROR, boost::make_tuple(description));
    }
}

any execute_text(const_string code,env_frame * parent_scope)
{
    return code_block::temporary_source(code,NULL)
        .compile(parent_scope).eval_each_expression(parent_scope);
}

} //namespace script
} //namespace fungu
