#ifndef PROGRAM_ARGUMENTS_HPP
#define PROGRAM_ARGUMENTS_HPP

#include <string>
#include <vector>
#include <map>
#include <list>

namespace program_arguments{

class basic_binder;

class parser
{
public:
    parser();
    ~parser();
    
    parser & add_option(const char   name, 
                        const char * verbose_name, 
                        const char * value_name, 
                        const char * description,
                        basic_binder * binding);
                        
    parser & add_option(const char   name,
                        const char * verbose_name,
                        const char * value_name,
                        const char * description,
                        std::string & output);
    
    parser & add_option(const char   name,
                        const char * verbose_name,
                        const char * value_name,
                        const char * description,
                        int &        output);
    
    parser & add_option(const char   name,
                        const char * verbose_Name,
                        const char * description,
                        bool &       output,
                        bool         value = true);
    
    parser & add_argument(const char * value_name, basic_binder * binding);
    parser & add_argument(const char * value_name, std::string & output);
    parser & add_argument(const char * value_name, int & output);
    parser & add_argument(const char * value_name, std::vector<std::string> &);
    
    void set_program_description(const char *);
    void set_options_group(const char *);
    void set_argument_repeat_last(int);
     
    void parse(int, const char **);
private:
    struct option
    {
        char           name;
        const char *   verbose_name;
        const char *   value_name;
        const char *   description;
        basic_binder * binding;
    };
    
    struct options_group
    {
        const char * description;
        std::vector<option> options;
    };
    
    struct argument
    {
        const char *   value_name;
        basic_binder * binding;
    };
    
    void help(const char *);
    static bool compare_option(option, option);
    
    std::map<char, option> m_names;
    std::map<std::string, option> m_verbose_names;
    typedef std::map<char, option>::iterator names_iterator;
    typedef std::map<std::string, option>::iterator verbose_names_iterator;
    
    std::vector<options_group> m_groups;
    
    std::vector<argument> m_arguments;
    std::vector<argument>::iterator m_next_argument;
    int m_argument_repeat_last;
    
    const char * m_program_description;
    
    std::list<basic_binder *> m_binders;
};

class basic_binder
{
public:
    virtual bool parse_value(const char *)=0;
};

class string_binder:public basic_binder
{
public:
    string_binder(std::string & output);
    bool parse_value(const char * value);
private:
    std::string & m_str;
};

class int_binder:public basic_binder
{
public:
    int_binder(int & output);
    bool parse_value(const char * value);
private:
    int & m_int;
};

class bool_binder:public basic_binder
{
public:
    bool_binder(bool & output, bool value);
    bool parse_value(const char * value);
private:
    bool & m_bool;
    bool m_value;
};

class string_vector_binder:public basic_binder
{
public:
    string_vector_binder(std::vector<std::string> &);
    bool parse_value(const char * value);
private:
    std::vector<std::string> & m_strings;
};

} //namespace program_arguments

#endif

