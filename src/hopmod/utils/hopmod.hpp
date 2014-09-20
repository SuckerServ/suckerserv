#ifndef HOPMOD_UTILS_HOPMOD_HPP
#define HOPMOD_UTILS_HOPMOD_HPP

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
    const char * build_time();
} //namespace hopmod

#endif
