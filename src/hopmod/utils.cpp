#include "utils.hpp"
#include <cstdio>
#include <cstdarg>
#include <string>
#include <stdlib.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#if defined(__APPLE__) && !defined(__IPHONE_OS_VERSION_MAX_ALLOWED)
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif //__APPLE__ && !__IPHONE_OS_VERSION_MAX_ALLOWED

#ifdef WIN32
static unsigned long long clockspeed;

static struct initnanotimer
{
    long long getclockspeed()
    {
        LARGE_INTEGER speed;
        if(!QueryPerformanceFrequency(&speed))
            speed.QuadPart = 0;

        long long result;

        result = speed.QuadPart;

        if(result <= 0)
            abort();

        return result;
    }

    initnanotimer()
    {
        clockspeed = getclockspeed();
    }
} initnanotimer;
#endif //WIN32

extern "C" {
unsigned long long getnanoseconds()
{
    extern unsigned long long nanosecbase;
#ifdef WIN32
    unsigned long long ticks;
    if(!QueryPerformanceCounter((LARGE_INTEGER*)&ticks))
        abort();
    double tmp = (ticks/(double)clockspeed)*1000000000.0;
    return tmp-nanosecbase;
#else
#ifndef __APPLE__
    struct timespec tp;
    if(clock_gettime(CLOCK_MONOTONIC, &tp) == 0)
        return (unsigned long long)((tp.tv_sec*1000000000LL)+tp.tv_nsec)-nanosecbase;
#else
#ifndef __IPHONE_OS_VERSION_MAX_ALLOWED 
    unsigned long long tmp;
    static mach_timebase_info_data_t sTimebaseInfo;
    tmp = mach_absolute_time();
    if (sTimebaseInfo.denom == 0) mach_timebase_info(&sTimebaseInfo);
    unsigned long long ns = tmp * sTimebaseInfo.numer / sTimebaseInfo.denom;
    tmp = ns-nanosecbase;
    return tmp;
#endif //!__IPHONE_OS_VERSION_MAX_ALLOWED
#endif //__APPLE__
#ifndef WIN32
    struct timeval tv;
    if(gettimeofday(&tv, NULL) == 0)
        return (unsigned long long)((tv.tv_sec*1000000000LL)+(tv.tv_usec*1000))-nanosecbase;
    abort();
#endif //!WIN32
#endif //WIN32
}

unsigned long long nanosecbase = getnanoseconds();
} // extern "C"

in_addr to_in_addr(in_addr_t x)
{
    in_addr r;
    r.s_addr = x;
    return r;
}

static unsigned long usec_diff(const unsigned long long & t1, const unsigned long long & t2)
{
    return (t2 - t1)/1000;
}

timer::timer()
{
    m_start = getnanoseconds();
}

timer::time_diff_t timer::usec_elapsed()const
{
    unsigned long long now = getnanoseconds();
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

static std::vector<const char *> temp_files;

void temp_file(const char * filename)
{
    temp_files.push_back(filename);
}

void temp_file_printf(const char * filename, const char * format, ...)
{
    FILE * file = fopen(filename, "w");
    if(!file) return;
    
    temp_file(filename);
    
    va_list args;
    va_start (args, format);
    vfprintf(file, format, args);
    va_end(args);
    
    fclose(file);
}

void delete_temp_files()
{
    for(std::vector<const char *>::const_iterator it = temp_files.begin(); it != temp_files.end(); it++)
        unlink(*it);
    temp_files.clear();
}

void delete_temp_files_on_shutdown(int type)
{
    delete_temp_files();
}

namespace hopmod{

int revision()
{
#if defined(REVISION) && (REVISION + 0)
    return REVISION;
#endif
    return -1;
}

const char * build_date()
{
    return __DATE__;
}

const char * build_time()
{
    return __TIME__;
}

} //namespace hopmod
