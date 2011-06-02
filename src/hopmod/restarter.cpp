
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <stdlib.h>

extern int prog_argc;
extern char * const * prog_argv;
static bool running_restarter = false;
static pid_t restarter_pid;
static pid_t server_pid;

void stop_restarter()
{
    if(!running_restarter) return;
    std::cout<<"Moving to background..."<<std::endl;
    kill(restarter_pid, SIGUSR1);
    running_restarter = false;
}

static void deactivate_signal(int)
{
    setpgid(0,0);
    exit(0);
}

void start_restarter()
{
    if(running_restarter) return;
    
    pid_t child = fork();
    bool parent = child != 0;
    
    if(child == -1)
    {
        std::cerr<<"Could not fork process in exec_restarter() - errno "<<errno<<std::endl;
        return;
    }
    
    running_restarter = true;
    
    if(parent)
    {
        server_pid = child;
        
        signal(SIGUSR1, &deactivate_signal);
        
        int status;
        waitpid(child, &status, 0);
        
        umask(0);
        int maxfd = getdtablesize();
        for(int i = 3; i < maxfd; i++) close(i);
        
        execv(prog_argv[0], prog_argv);
        exit(1);
    }
    else
    {
        restarter_pid = getppid();
    }
}
