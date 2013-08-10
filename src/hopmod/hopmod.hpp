#ifndef HOPMOD_HPP
#define HOPMOD_HPP

#include "signals.hpp"
#include "utils.hpp"
#include "events.hpp"

#include "net/address.hpp"
#include "net/address_mask.hpp"
#include "net/address_prefix.hpp"
#include "netbans.hpp"

void init_hopmod();
void reload_hopmod();
void restart_now();

// Scripting functions
void init_lua();
void shutdown_lua();
lua_State * get_lua_state();
lua::event_environment & event_listeners();
void log_event_error(const char * event_id, const char * error_message);

// Scheduler Functions
void sched_callback(int (*)(void *),void *);
void sched_callback(int (*)(void *),void *, int);
void cancel_timer(int);
namespace lua{
int sleep(lua_State *);
int interval(lua_State *);
int cancel_timer(lua_State *);
} //namespace lua

enum{
    SHUTDOWN_NORMAL,
    SHUTDOWN_RESTART,
    SHUTDOWN_RELOAD
};

#endif

