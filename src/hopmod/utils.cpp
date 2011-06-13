#include "utils.hpp"
#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

in_addr to_in_addr(in_addr_t x)
{
    in_addr r;
    r.s_addr = x;
    return r;
}

static unsigned long usec_diff(const timespec & t1, const timespec & t2)
{
    long secs = t2.tv_sec - t1.tv_sec;
    if(secs > 0) return ((secs - 1) * 1000000) + (1000000000 - t1.tv_nsec + t2.tv_nsec)/1000;
    else return (t2.tv_nsec - t1.tv_nsec)/1000;
}

timer::timer()
{
    clock_gettime(CLOCK_MONOTONIC, &m_start);
}

timer::time_diff_t timer::usec_elapsed()const
{
    timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return usec_diff(m_start, now);
}

bool file_exists(const char * name)
{
    struct stat info;
    if(stat(name, &info)==0) return !(info.st_mode & S_IFDIR);
    else return false;
}

bool dir_exists(const char * name)
{
    struct stat info;
    if(stat(name, &info)==0) return info.st_mode & S_IFDIR;
    else return false;
}

freqlimit::freqlimit(int length)
 :m_length(length),
  m_hit(0)
{
    
}

int freqlimit::next(int time)
{
    if(time >= m_hit)
    {
        m_hit = time + m_length;
        return 0;
    }else return m_hit - time; 
}

static std::vector<const char *> info_files;

bool info_file(const char * filename, const char * format, ...)
{
    FILE * file = fopen(filename, "w");
    if(!file) return false;
    va_list args;
    va_start (args, format);
    vfprintf(file, format, args);
    va_end(args);
    fclose(file);
    info_files.push_back(filename);
    return true;
}

void cleanup_info_files()
{
    for(std::vector<const char *>::const_iterator it = info_files.begin(); it != info_files.end(); it++)
        unlink(*it);
    info_files.clear();
}

void cleanup_info_files_on_shutdown(int type)
{
    cleanup_info_files();
}
