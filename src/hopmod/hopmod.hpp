#ifndef HOPMOD_HPP
#define HOPMOD_HPP

extern "C"{
#include <lua.h>
}

namespace fungu{
namespace script{
class env;
class error_trace;
} //namespace script
} //namespace fungu

#include "signals.hpp"
#include "utils.hpp"

void init_hopmod();
void reload_hopmod();
void update_hopmod();
void restart_now();

// Scripting functions
void init_scripting();
void shutdown_scripting(int);
fungu::script::env & get_script_env();
void register_server_script_bindings(fungu::script::env &);
std::string get_script_error_message(fungu::script::error_trace * errinfo);
void report_script_error(fungu::script::error_trace *);
void report_script_error(const char *);
void register_lua_function(lua_CFunction,const char *);
bool unref(const char *);

// Script Pipe Functions
void init_script_pipe();
bool open_script_pipe(const char *,int,fungu::script::env &);
void run_script_pipe_service(int);
void close_script_pipe(int);
void unlink_script_pipe();

// Script Socket Functions
bool script_socket_supported();
void init_script_socket();
bool open_script_socket(unsigned short, const char *);
void run_script_socket_service();
void close_script_socket(int);

int get_player_id(const char * name, unsigned long ip);
void clear_player_ids();

// Scheduler Functions
void init_scheduler();
void update_scheduler(int);
void sched_callback(int (*)(void *),void *);
void sched_callback(int (*)(void *),void *, int);

#include <vector>
#include <string>

// Player command functions and variables
std::vector<std::string> parse_player_command_line(const char *);

// Text Colouring Macros
#define GREEN "\f0"
#define BLUE "\f1"
#define YELLOW "\f2"
#define RED "\f3"
#define GREY "\f4"
#define MAGENTA "\f5"
#define ORANGE "\f6"

extern unsigned int maintenance_frequency;
void set_maintenance_frequency(unsigned int);

enum{
    SHUTDOWN_NORMAL,
    SHUTDOWN_RESTART,
    SHUTDOWN_RELOAD
};

#endif
