#include "utils/files.hpp"
#include <vector>
#include <cstdarg>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>

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

