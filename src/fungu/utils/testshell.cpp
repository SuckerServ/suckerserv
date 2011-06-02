
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

#include <fungu/script.hpp>
#include <boost/bind.hpp>

using namespace fungu;
static script::env environment;

void print_script_error(const script::error_trace * errinfo)
{
    const script::source_context * source = errinfo->get_root_info()->get_source_context();
    script::error error = errinfo->get_error();
    error.get_error_message();
    std::cerr<<"Error";
    if(source) std::cerr<<" "<<source->get_location()<<":"<<source->get_line_number();
    std::cerr<<": "<<error.get_error_message();
    std::cerr<<std::endl;
}

void evaluator(script::expression * expr,script::env::frame * frame, void *)
{
    if(!expr->is_empty_expression())
    {
        const_string result = expr->eval(frame).to_string();
        if(result.length()) std::cout<<result<<std::endl;
    }
}

void echo(const char * text)
{
    std::cout<<text<<std::endl;
}

int main(int argc,char ** argv)
{
    script::load_corelib(environment);
    
    script::function<void (const char *)> echo_func(echo);
    environment.bind_global_object(&echo_func,FUNGU_OBJECT_ID("echo"));
    
    const char * line = NULL;
    
    script::env::frame frame(&environment);
    script::eval_stream reader(&frame, &evaluator, NULL);
    
    script::file_source_context source("stdin");
    source.set_line_number(1);
    environment.set_source_context(&source);
    
    do
    {
        line = readline(!reader.is_parsing_expression() ? "fungu> " : ">> ");
        if(line)
        {
            if(line[0]!='\0') add_history(line);
            
            try
            {
                reader.feed(line,strlen(line));
                reader.feed("\n",1);
            }
            catch(script::error_trace * errinfo)
            {
                print_script_error(errinfo);
                delete errinfo;
            }
        }
    }while(line && !frame.has_expired());
    
    std::cout<<"\nBye."<<std::endl;
    
    return 0;
}
