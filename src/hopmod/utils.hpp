#ifndef HOPMOD_UTILS_HPP
#define HOPMOD_UTILS_HPP

#include <time.h>
#include <string>

class timer
{
public:
    typedef unsigned time_diff_t;
    timer();
    time_diff_t usec_elapsed()const;
private:
    timespec m_start;
};

bool file_exists(const char *);
bool dir_exists(const char *);

bool info_file(const char *, const char *, ...);
void cleanup_info_files();
void cleanup_info_files_on_shutdown(int);

// Text Colouring Macros
#define GREEN "\f0"
#define BLUE "\f1"
#define YELLOW "\f2"
#define RED "\f3"
#define GREY "\f4"
#define MAGENTA "\f5"
#define ORANGE "\f6"

namespace hopmod{
int revision();
const char * build_date();
} //namespace hopmod

#endif

