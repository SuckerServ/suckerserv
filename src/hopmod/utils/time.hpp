#ifndef HOPMOD_UTILS_TIME_HPP
#define HOPMOD_UTILS_TIME_HPP

#include <time.h>
#include <string>

extern "C" unsigned long long getnanoseconds();

static inline unsigned long long getmicroseconds()
{
    return getnanoseconds()/1000;
}

static inline unsigned long long getmilliseconds()
{
    return getmicroseconds()/1000;
}

extern unsigned long long int startup;

static inline int getuptime()
{
    return (getnanoseconds()-startup)/1000000000ULL;
}

class timer
{
public:
    typedef unsigned time_diff_t;
    timer();
    time_diff_t usec_elapsed()const;
private:
    unsigned long long m_start;
};

#endif
