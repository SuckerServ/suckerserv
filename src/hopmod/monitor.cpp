#include "program_arguments.hpp"

#include <iostream>
#include <string>
#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

extern char ** environ;

static void create_process(const std::string & program, 
                           const std::vector<std::string> & arguments)
{
    pid_t child_pid = fork();
    if(child_pid == -1) exit(1);
    
    if(child_pid == 0)
    {
        char ** argv = new char *[arguments.size() + 1];
        for(size_t i = 0; i < arguments.size(); i++)
            argv[i] = const_cast<char *>(arguments[i].c_str());
        argv[arguments.size()] = NULL;
        if(execve(program.c_str(), argv, environ) == -1) exit(1);
    }
}

int main(int argc, const char ** argv)
{
    bool disable_autorestart;

    std::string program;
    std::vector<std::string> program_arguments;
    
    {
        program_arguments::parser p;
        
        p.add_option(
            'd', 
            "disable-autorestart",
            "Disable automatic restart of process",
            disable_autorestart);
        
        p.add_argument("program", program);
        p.add_argument("argument", program_arguments);
        
        p.parse(argc, argv);
    }

    bool restart;
    
    do{
        restart = false;
        
        create_process(program, program_arguments);
        
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
                    if(disable_autorestart) break;
                    restart = true;
                    break;
                default:;
            }
        }
    }
    while(restart);
    
    return 0;
}

