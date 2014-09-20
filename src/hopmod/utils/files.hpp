#ifndef HOPMOD_UTILS_FILES_HPP
#define HOPMOD_UTILS_FILES_HPP

bool file_exists(const char *);
bool dir_exists(const char *);

void temp_file(const char *);
void temp_file_printf(const char *, const char *, ...);
void delete_temp_files();
void delete_temp_files_on_shutdown(int);

#endif
