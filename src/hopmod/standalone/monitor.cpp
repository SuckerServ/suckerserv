#include "lib/program_arguments.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

extern char ** environ;

static void create_process(const std::string & filename, 
                           const std::vector<std::string> & arguments)
{
    pid_t child_pid = fork();
    if(child_pid == -1) exit(1);
    
    if(child_pid == 0)
    {
        char ** argv = new char *[arguments.size() + 2];
        for(size_t i = 0; i < arguments.size(); i++)
            argv[i] = const_cast<char *>(arguments[i].c_str());
        argv[arguments.size()] = NULL;
        if(execve(filename.c_str(), argv, environ) == -1) exit(1);
    }
}

static bool file_exists(const std::string & filename)
{
    struct stat info;
    if(stat(filename.c_str(), &info)==0) return !(info.st_mode & S_IFDIR);
    else return false;
}

static bool find_program(const std::string & program, std::string & output_filename)
{
    if(file_exists(program))
    {
        output_filename = program;
        return true;
    }
    
    const char * search_paths = getenv("PATH");
    const char * search_paths_begin = search_paths;
    const char * search_paths_end = search_paths + strlen(search_paths);
    
    std::string filename;
    
    while(search_paths_begin < search_paths_end)
    {
        const char * limit = strstr(search_paths_begin, ":");
        if(!limit) limit = search_paths_end;
        
        filename.reserve(std::max(filename.capacity(), 
            (limit - search_paths_begin) + program.length() + 1));
        
        filename.assign(search_paths_begin, limit);
        filename.append(1, '/');
        filename.append(program);
        
        if(file_exists(filename))
        {
            output_filename = filename;
            return true;
        }
        
        search_paths_begin = (*limit == ':' ? limit + 1 : limit);
    }
    
    return false;
}

int main(int argc, const char ** argv)
{
    std::string program;
    std::vector<std::string> program_arguments;
    int max_restarts = -1;
    
    {
        program_arguments::parser p;
        
        p.add_option(
            'm',
            "max-restarts",
            "N",
            "Limit the number of restarts",
            max_restarts
        );
        
        p.add_argument("PROGRAM-ARGUMENTS", program_arguments);
        
        p.parse(argc, argv);
    }
    
    if(program_arguments.size() == 0)
    {
        std::cerr<<"Not enough program arguments"<<std::endl;
        return 1;
    }
    
    std::string program_filename;
    if(!find_program(program_arguments[0], program_filename))
    {
        std::cout<<program_arguments[0]<<" file not found"<<std::endl;
        return 1;
    }
    
    bool repeat = false;
    int count_restarts = 0;
    
    do
    {
        repeat = false;
        
        create_process(program_filename, program_arguments);
        
        int child_exit_status = 0;
        if(::wait(&child_exit_status) == -1) return 1;
        
        if(WIFSIGNALED(child_exit_status))
        {
            int signal = WTERMSIG(child_exit_status);
            
            switch(signal)
            {
                case SIGILL:
                case SIGABRT:
                case SIGFPE:
                case SIGSEGV:
                case SIGPIPE:
                    repeat = true;
                    break;
                default:;
            }
        }
    }
    while(repeat && (max_restarts == -1 || ++count_restarts <= max_restarts));
    
    return 0;
}

