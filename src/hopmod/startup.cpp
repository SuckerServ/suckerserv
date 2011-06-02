#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "cube.h"
#include "hopmod.hpp"
#include "lua/modules.hpp"
#include "main_io_service.hpp"

#include <fungu/script/env.hpp>
#include <fungu/script/execute.hpp>
#include <fungu/script/error.hpp>
using namespace fungu;

#include <signal.h>
#include <iostream>

#include <boost/thread.hpp>

#ifdef HAS_LSQLITE3
extern "C"{
int luaopen_lsqlite3(lua_State * L);
}
#endif

namespace server{
void started();
void sendservmsg(const char *);
extern string smapname;
bool selectnextgame();
}//namespace server

int get_num_async_resolve_operations(); //defined in lua/net.cpp

void start_restarter();
void stop_restarter();

extern "C"{
int lua_packlibopen(lua_State *L);
} //extern "C"

static boost::signals::connection close_listenserver_slot;
static bool reload = false;

unsigned int maintenance_frequency;
static int maintenance_time = 0;
bool reloaded = false;

static boost::thread::id main_thread;

static void load_lua_modules()
{
    lua_State * L = get_script_env().get_lua_state();

    lua::module::open_net(L);
    lua::module::open_timer(L);
    lua::module::open_crypto(L);
    lua::module::open_cubescript(L);
    lua::module::open_geoip(L);
    lua::module::open_filesystem(L);
    lua::module::open_http_server(L);
    
    lua_packlibopen(L);
    
    #ifdef HAS_LSQLITE3
    luaopen_lsqlite3(L);
    #endif
}

/**
    Initializes everything in hopmod. This function is called at server startup and server reload.
*/
void init_hopmod()
{
    main_thread = boost::this_thread::get_id();
    
    /*
        Misc
    */
    set_maintenance_frequency(86400000); // signal maintenance event every 24 hours
    info_file("log/sauer_server.pid", "%i\n", getpid());

    /*
        Scripting environment
    */
    init_scripting(); // create scripting environment
    script::env & env = get_script_env();
    register_server_script_bindings(env); // expose the core server api to the embedded scripting languages
    register_signals(env); // register signals with scripting environment
    init_scheduler(); // expose the scheduler functions to the scripting environment
    load_lua_modules(); // extra lua modules written for hopmod

    /*
        Event handlers
    */
    close_listenserver_slot = signal_shutdown.connect(&stopgameserver);
    signal_shutdown.connect(&shutdown_scripting);
    signal_shutdown.connect(&cleanup_info_files_on_shutdown);

    /*
        Execute the first script. After this script is finished everything on
        the scripting side of hopmod should be loaded.
    */
    try
    {
        fungu::script::execute_file("script/base/init.cs", get_script_env());
    }
    catch(fungu::script::error_trace * error)
    {
        report_script_error(error);
    }
}

static void reload_hopmod_now()
{
    if(get_num_async_resolve_operations())
    {
        //Reload scripts delayed due to pending async resolve operations. Rescheduling reload...
        get_main_io_service().post(reload_hopmod_now);
        return;
    }

    signal_reloadhopmod();

    reloaded = true;

    close_listenserver_slot.block();  // block close_listenserver_slot to keep clients connected
    signal_shutdown(SHUTDOWN_RELOAD);
    close_listenserver_slot.unblock();

    disconnect_all_slots();

    init_hopmod();
    server::started();
    std::cout<<"-> Reloaded Hopmod."<<std::endl;
}

void reload_hopmod()
{
    get_main_io_service().post(reload_hopmod_now);
}

void update_hopmod()
{
    if(reload) reload_hopmod();

    run_script_pipe_service(totalmillis);

    update_scheduler(totalmillis);
    cleanup_dead_slots();

    if(maintenance_frequency != 0 && totalmillis > maintenance_time && !hasnonlocalclients())
    {
        signal_maintenance();
        maintenance_time = totalmillis + maintenance_frequency;
    }
}

namespace server{

void started()
{
    /*
        Startup the serverexec named pipe service. The service runs by polling
        from update_hopmod().
    */
    init_script_pipe();
    open_script_pipe("serverexec",511,get_script_env());

    signal_started();
    if(!server::smapname[0]) selectnextgame();
}

void shutdown()
{
    if(boost::this_thread::get_id() != main_thread) return;
       
    signal_shutdown(SHUTDOWN_NORMAL);
    stop_restarter();
    exit(0);
}

} //namespace server

void restart_now()
{
    server::sendservmsg("Server restarting...");
    start_restarter();
    signal_shutdown(SHUTDOWN_RESTART);
    exit(0);
}

void set_maintenance_frequency(unsigned int frequency)
{
    maintenance_frequency = frequency;
    maintenance_time = totalmillis + frequency;
}
