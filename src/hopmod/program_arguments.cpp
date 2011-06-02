#include "program_arguments.hpp"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <sstream>
#include <iostream>

namespace program_arguments{

static std::string uppercase(const std::string & input)
{
    std::string output(input);
    std::transform(output.begin(), output.end(), output.begin(), ::toupper);
    return output;
}

static basic_binder * delete_basic_binder(basic_binder * binding)
{
    delete binding;
    return NULL;
}

parser::parser()
 :m_program_description(""),
  m_argument_repeat_last(0)
{
    options_group unnamed;
    unnamed.description = NULL;
    m_groups.push_back(unnamed);
}

parser::~parser()
{
    std::transform(m_binders.begin(), m_binders.end(), m_binders.begin(), 
        delete_basic_binder);
}

parser & parser::add_option(const char     name, 
                            const char *   verbose_name, 
                            const char *   value_name, 
                            const char *   description,
                            basic_binder * binding)
{
    option opt;
    opt.name = name;
    opt.verbose_name = verbose_name;
    opt.value_name = value_name;
    opt.description = description;
    opt.binding = binding;
    
    if(opt.name != '\0') m_names[opt.name] = opt;
    if(opt.verbose_name) m_verbose_names[opt.verbose_name] = opt;
    
    m_groups.back().options.push_back(opt);
    
    std::sort(m_groups.back().options.begin(), 
              m_groups.back().options.end(),
              &parser::compare_option);
    
    return *this;
}

parser & parser::add_option(const char     name, 
                            const char *   verbose_name, 
                            const char *   value_name, 
                            const char *   description,
                            std::string &  output)
{
    add_option(name, verbose_name, value_name, description, 
        new string_binder(output));
    return *this;
}

parser & parser::add_option(const char     name, 
                            const char *   verbose_name, 
                            const char *   value_name, 
                            const char *   description,
                            int &          output)
{
    add_option(name, verbose_name, value_name, description, 
        new int_binder(output));
    return *this;
}

parser & parser::add_option(const char     name, 
                            const char *   verbose_name, 
                            const char *   description,
                            bool &         output,
                            bool           value)
{
    add_option(name, verbose_name, NULL, description, 
        new bool_binder(output, value));
    return *this;
}

parser & parser::add_argument(const char * value_name, basic_binder * binding)
{
    if(m_argument_repeat_last) m_argument_repeat_last++;

    argument arg;
    arg.value_name = value_name;
    arg.binding = binding;
    m_arguments.push_back(arg);
    
    m_next_argument = m_arguments.begin();
    
    return *this;
}

parser & parser::add_argument(const char * value_name, std::string & output)
{
    add_argument(value_name, new string_binder(output));
    return *this;
}

parser & parser::add_argument(const char * value_name, int & output)
{
    add_argument(value_name, new int_binder(output));
    return *this;
}

parser & parser::add_argument(const char * value_name, 
    std::vector<std::string> & output)
{
    add_argument(value_name, new string_vector_binder(output));
    if(!m_argument_repeat_last) m_argument_repeat_last = 1;
    return *this;
}

void parser::set_program_description(const char * description)
{
    m_program_description = description;
}

void parser::set_options_group(const char * description)
{
    options_group group;
    group.description = description;
    m_groups.push_back(group);
}

void parser::set_argument_repeat_last(int value)
{
    m_argument_repeat_last = value;
}

void parser::parse(int argc, const char ** argv)
{
    bool print_help = false;
    set_options_group(NULL);
    add_option('h', "help", "display this help and exit", print_help);
    
    std::vector<std::string> error_messages;
    
    for(int i = 1; i < argc; i++)
    {
        const char * arg = argv[i];
        
        if(arg[0] == '-')
        {
            if(arg[1] == '-')
            {
                const char * name_begin = arg + 2;
                const char * value = strstr(name_begin, "=");
                const char * name_end = 
                    (value ? value : name_begin + strlen(name_begin));
                
                verbose_names_iterator iter = 
                    m_verbose_names.find(std::string(name_begin, name_end));
                
                if(iter == m_verbose_names.end())
                {
                    std::stringstream format;
                    format<<"unknown option  '--"
                          <<std::string(name_begin, name_end)
                          <<"'";
                    error_messages.push_back(format.str());
                    continue;
                }
                
                option * opt = &(iter->second);
                
                if(value)
                {
                    if(!opt->value_name)
                    {
                        std::stringstream format;
                        format<<"option '--"
                              <<std::string(name_begin, name_end)
                              <<"' doesn't allow an argument";
                        error_messages.push_back(format.str());
                        continue;
                    }
                }
                else
                {
                    if(opt->value_name)
                    {
                        std::stringstream format;
                        format<<"option  '--"
                              <<std::string(name_begin, name_end)
                              <<"' requires an argument";
                        error_messages.push_back(format.str());
                        continue;
                    }
                }
                
                opt->binding->parse_value(value + 1);
            }
            else
            {
                names_iterator iter = m_names.find(arg[1]);
                
                if(iter == m_names.end())
                {
                    std::stringstream format;
                    format<<"unknown option  '-"<<arg[1]<<"'";
                    error_messages.push_back(format.str());
                    continue;
                }
                
                option * opt = &(iter->second);
                
                const char * value = "=";
                
                if(opt->value_name)
                {
                    value = argv[++i];
                    
                    if(!value || value[0] == '-')
                    {
                        std::stringstream format;
                        format<<"option  '-"<<arg[1]
                              <<"' requires an argument";
                        error_messages.push_back(format.str());
                        continue;
                    }
                }
                
                opt->binding->parse_value(value);
            }
        }
        else
        {
            if(m_next_argument == m_arguments.end())
            {
                if(m_argument_repeat_last)
                {
                    m_next_argument = m_arguments.begin() + 
                        (m_arguments.size() - m_argument_repeat_last);
                }
                else
                {
                    error_messages.push_back("too many arguments");
                    continue;
                }
            }
            
            (*m_next_argument).binding->parse_value(arg);
            m_next_argument++;
        }
    }
    
    if(print_help)
    {
        help(argv[0]);
        exit(0);
    }
    
    if(m_next_argument < m_arguments.end()  - m_argument_repeat_last)
    {
        for(; m_next_argument != m_arguments.end(); m_next_argument++)
        {
            std::stringstream format;
            format<<"missing "<<uppercase(m_next_argument->value_name)
                  <<" argument";
            error_messages.push_back(format.str());
        }
    }
    
    if(error_messages.size())
    {
        for(std::vector<std::string>::const_iterator iter 
                = error_messages.begin(); iter != error_messages.end(); iter++)
        {
            std::cerr<<argv[0]<<": "<<*iter<<std::endl;
        }
        exit(1);
    }
}

void parser::help(const char * program_name)
{
    std::cout<<"Usage: "<<program_name<<" [OPTION]... ";
    
     for(int i = 0; i < m_arguments.size(); i++)
    {
        std::cout<<(i > 0 ? " " : "" )
            <<uppercase(m_arguments[i].value_name)
            <<(i >= (m_arguments.size() - m_argument_repeat_last) ? "..." : "");
    }
    std::cout<<std::endl<<m_program_description<<std::endl;
    
    std::vector<options_group>::const_iterator group = m_groups.begin();
    
    for(; group != m_groups.end(); group++)
    {
        if(group->description) std::cout<<group->description<<":"<<std::endl;

        std::vector<option>::const_iterator option = group->options.begin();
        for(; option != group->options.end(); option++)
        {
            std::string name;
            std::string long_name;
            std::string description;
            
            if(option->name)
            {
                name = '-';
                name+= option->name;
            }
            
            if(option->verbose_name)
            {
                long_name = "--";
                long_name+= option->verbose_name;
                
                if(option->value_name)
                {
                    long_name += '=';
                    long_name += uppercase(option->value_name);
                }
            }
            
            if(option->description)
            {
                description = option->description;
            }
            
            name += std::string(std::max<int>(0, 2 - name.length()), ' ');
            long_name+=std::string(std::max<int>(0,22-long_name.length()), ' ');
            
            std::cout<<" "<<name<<", "<<long_name<<" "<<description<<std::endl;
        }
        
        std::cout<<std::endl;
    }
    
    std::cout<<std::endl;
}


bool parser::compare_option(option a, option b)
{
    return a.name < b.name;
}

string_binder::string_binder(std::string & output)
:m_str(output)
{
    
}

bool string_binder::parse_value(const char * value)
{
    m_str = value;
    return true;
}

int_binder::int_binder(int & output)
:m_int(output)
{

}

bool int_binder::parse_value(const char * value)
{
    m_int = atoi(value);
    return true;
}

bool_binder::bool_binder(bool & output, bool value)
:m_bool(output), m_value(value)
{
    
}

bool bool_binder::parse_value(const char *)
{
    m_bool = m_value;
    return true;
}

string_vector_binder::string_vector_binder(std::vector<std::string> & input)
 :m_strings(input)
{
        
}

bool string_vector_binder::parse_value(const char * value)
{
    m_strings.push_back(value);
    return true;
}

} //namespace program_arguments

