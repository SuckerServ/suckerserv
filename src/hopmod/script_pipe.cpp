#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "hopmod.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include <iostream>

#include <fungu/script.hpp>
using namespace fungu;

static const char * filename = NULL;
static int fd = -1;
static script::env * env = NULL;
static script::env_frame * frm = NULL;
static script::eval_stream * scriptexec = NULL;
static script::file_source_context * script_context = NULL;
static int pending_idle_time = 0;
static int pending_idle_timeout = 2000;
static bool in_feed = false;
static bool pending_close = false;

void init_script_pipe()
{
    signal_shutdown.connect(&close_script_pipe,boost::signals2::at_front);
}

bool open_script_pipe(const char * filename, int mode, script::env & server_env)
{
    assert(fd == -1);
    
    umask(0);
    if(mkfifo(filename,mode) == -1)
    {
        std::cerr<<"cannot create fifo file "<<filename<<": ";
        switch(errno)
        {
            case EACCES:
                std::cerr<<"permission denied.";
                break;
            case EEXIST:
                std::cerr<<"file already exists.";
                break;
            default:
                std::cerr<<"errno "<<errno;
        }
        std::cerr<<std::endl;
        
        bool failed = (errno == EEXIST ? false : true);
        if(failed) return false;
    }
    
    fd = open(filename, O_RDONLY | O_NONBLOCK);
    if(fd == -1)
    {
        std::cerr<<"cannot open "<<filename<<": ";
        switch(errno)
        {
            default:
                std::cerr<<"errno "<<errno;
        }
        std::cerr<<std::endl;
        return false;
    }
    
    ::filename = filename;
    ::env = &server_env;
    frm = new script::env_frame(::env);
    
    scriptexec = new script::eval_stream(frm);
    script_context = new script::file_source_context(filename);
    
    script::bind_var(pending_idle_timeout,"script_pipe_pending_idle_timeout",server_env);
    
    pending_close = false;
    
    return true;
}

void run_script_pipe_service(int curtime)
{
    if(fd == -1) return;
    
    if(pending_idle_time && curtime - pending_idle_time >= pending_idle_timeout)
    {
        std::cerr<<"discarded pending code in the script pipe (timed out)"<<std::endl;
        pending_idle_time = 0;
        scriptexec->reset();
    }
    
    pollfd p;
    p.fd = fd;
    p.events = POLLIN;
    p.revents = 0;
    
    int pret = poll(&p, 1, 0);
    if(pret == -1)
    {
        std::cerr<<"in run_script_pipe_service(): poll() returned error (errno "<<errno<<")."<<std::endl;
        return;
    }
    
    if( p.revents & POLLIN )
    {
        const script::source_context * prev_context = env->get_source_context();
        env->set_source_context(script_context);
        
        char buffer[256];
        ssize_t readsize;
        do
        {
            readsize = read(fd, buffer, sizeof(buffer) - 1);
            
            if(readsize == -1)
            {
                if(errno != EAGAIN)
                    std::cerr<<"in run_script_pipe_service(): read() returned error (errno "<<errno<<")."<<std::endl;
                break;
            }
            
            buffer[readsize] = '\0';
            
            try
            {
                in_feed = true;
                scriptexec->feed(buffer, readsize);
            }
            catch(script::error_trace * errinfo)
            {
                report_script_error(errinfo);
            }
            
            in_feed = false;
            
        }while(readsize > 0 && !pending_close);
        
        pending_idle_time = scriptexec->is_parsing_expression() ? curtime : 0;
        env->set_source_context(prev_context);
        
        if(pending_close)
        {
            delete scriptexec;
            scriptexec = NULL;
            
            delete script_context;
            script_context = NULL;
            
            return;
        }
    }
}

void close_script_pipe(int)
{
    close(fd);
    if(filename && unlink(filename) == -1)
        std::cerr<<"could not delete "<<filename<<": errno "<<errno<<"."<<std::endl;
    fd = -1;
    filename = NULL;
    pending_idle_time = 0;
    
    pending_close = true;
    if(in_feed) return;
    
    delete scriptexec;
    scriptexec = NULL;
    
    delete script_context;
    script_context = NULL;
    
    delete frm;
    frm = NULL;
}

void unlink_script_pipe()
{
    if(filename) unlink(filename);
}
