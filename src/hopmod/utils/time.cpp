#include "utils/time.hpp"
#include <sys/time.h>


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
