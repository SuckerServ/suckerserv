#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "hopmod.hpp"
#include "handle_resolver.hpp"

#include <fungu/script.hpp>
#include <fungu/script/slot_factory.hpp>
#include <fungu/script/lua/lua_function.hpp>
using namespace fungu;

static script::env * env = NULL;
static script::slot_factory slots;

boost::signals2::signal<void ()> signal_started;
boost::signals2::signal<void (int)> signal_shutdown;
boost::signals2::signal<void ()> signal_shutdown_scripting;
boost::signals2::signal<void ()> signal_reloadhopmod;

boost::signals2::signal<int (int, const char *, const char *, const char *, bool), proceed> signal_connecting;
boost::signals2::signal<void (int)> signal_connect;
boost::signals2::signal<void (int,const char *)> signal_disconnect;
boost::signals2::signal<void (const char *,const char *)> signal_failedconnect;
boost::signals2::signal<void (int)> signal_maploaded;
boost::signals2::signal<int (int,const char *), proceed>  signal_allow_rename;
boost::signals2::signal<void (int,const char *,const char *)> signal_rename;
boost::signals2::signal<void (int,int)> signal_renaming;
boost::signals2::signal<void (int,const char *,const char *)> signal_reteam;
boost::signals2::signal<int (int,const char *,const char *), proceed> signal_chteamrequest;
boost::signals2::signal<int (int,const char *), proceed> signal_text;
boost::signals2::signal<int (int,const char *), proceed> signal_sayteam;
boost::signals2::signal<void ()> signal_intermission;
boost::signals2::signal<void ()> signal_finishedgame;
boost::signals2::signal<void (int,int)> signal_timeupdate;
boost::signals2::signal<void (const char *,const char *)> signal_mapchange;
boost::signals2::signal<int (int,const char *,const char *), proceed> signal_mapvote;
boost::signals2::signal<void ()> signal_setnextgame;
boost::signals2::signal<void ()> signal_gamepaused;
boost::signals2::signal<void ()> signal_gameresumed;
boost::signals2::signal<int (int, const char *, const char *), proceed> signal_setmastermode_request;
boost::signals2::signal<int (int, const char *, const char *)> signal_setmastermode;
boost::signals2::signal<void (int,int)> signal_spectator;
boost::signals2::signal<void (int,int,int)> signal_privilege;
boost::signals2::signal<void (int,int)> signal_teamkill;
boost::signals2::signal<void (int,int)> signal_frag;
boost::signals2::signal<void (int,const char *,const char *)> signal_authreq;
boost::signals2::signal<void (int,int,const char *)> signal_authrep;
boost::signals2::signal<void (int,int,int)> signal_addbot;
boost::signals2::signal<void (int)> signal_delbot;
boost::signals2::signal<void (int)> signal_botleft;
boost::signals2::signal<void (int,const char *)> signal_beginrecord;
boost::signals2::signal<void (int,int)> signal_endrecord;
boost::signals2::signal<void (int)> signal_mapcrcfail;
boost::signals2::signal<void (int, const char *, int)> signal_mapcrc;
boost::signals2::signal<void (int)> signal_checkmaps;
boost::signals2::signal<void (const char *,const char *)> signal_votepassed;
boost::signals2::signal<void (int,int,int)> signal_shot;
boost::signals2::signal<void (int)> signal_suicide;
boost::signals2::signal<void (int, const char *)> signal_takeflag;
boost::signals2::signal<void (int, const char *)> signal_dropflag;
boost::signals2::signal<void (int, const char *)> signal_scoreflag;
boost::signals2::signal<void (int, const char *)> signal_returnflag;
boost::signals2::signal<void (const char *)> signal_resetflag;
boost::signals2::signal<void (const char *, int)> signal_scoreupdate;
boost::signals2::signal<void ()> signal_maintenance;
boost::signals2::signal<void (int)> signal_spawn;
boost::signals2::signal<int (int,int,int,int), proceed> signal_damage;
boost::signals2::signal<int (int,const char*, bool), proceed> signal_setmaster;
boost::signals2::signal<int (int,int), maxvalue> signal_respawnrequest;
boost::signals2::signal<void ()> signal_clearbans_request;
boost::signals2::signal<void (int, const char *, int, int, const char *)> signal_kick_request;
boost::signals2::signal<void (const char *)> signal_varchanged;

static void destroy_slot(int handle);
namespace lua{
static int register_event_handler(lua_State * L);
}//namespace lua


static script::any proceed_error_handler(script::error_trace * errinfo)
{
    report_script_error(errinfo);
    return true;
}

static script::any normal_error_handler(script::error_trace * errinfo)
{
    report_script_error(errinfo);
    return script::any::null_value();
}

static script::any maxvalue_error_handler(script::error_trace * errinfo)
{
    report_script_error(errinfo);
    return 0;
}

namespace lua{

typedef std::vector<int> lua_function_vector;
typedef std::map<std::string, lua_function_vector>::iterator created_event_slots_iterator;
typedef std::map<int, std::pair<lua_function_vector *, int> > handle_slot_map;

static std::map<std::string, lua_function_vector> created_event_slots;
static handle_slot_map handle_to_slot;
static handle_resolver<created_event_slots_iterator> signal_handles;

static lua_function_vector::iterator get_free_vector_iterator(lua_function_vector & v)
{
    for(lua_function_vector::iterator it = v.begin(); it != v.end(); it++)
        if(*it == -1) return it;
    v.push_back(-1);
    return v.end() - 1;
}

static int event_trigger(lua_State * L)
{
    lua_function_vector & handlers = *reinterpret_cast<lua_function_vector *>(lua_touserdata(L, lua_upvalueindex(1)));
    int argc = lua_gettop(L);
    
    for(lua_function_vector::const_iterator it = handlers.begin(); it != handlers.end(); it++)
    {
        int handler_ref = *it;
        if(handler_ref == -1) continue;
        
        lua_rawgeti(L, LUA_REGISTRYINDEX, handler_ref);
        
        for(int i = 1; i <= argc; i++)
        {
            lua_pushvalue(L, i);
        }
        
        if(lua_pcall(L, argc, 0, 0) != 0)
        {
            report_script_error(lua_tostring(L, -1));
        }
    }
    
    return 0;
}

static int create_signal(lua_State * L)
{
    const char * name = luaL_checkstring(L, 1);
    
    std::pair<created_event_slots_iterator, bool> inserted = created_event_slots.insert(std::pair<std::string, lua_function_vector>(name, std::vector<int>()));
    
    if(inserted.second == false)
    {
        luaL_argerror(L, 1, "name already in use");
        return 0;
    }
    
    lua_pushlightuserdata(L, &inserted.first->second);
    lua_pushcclosure(L, &event_trigger, 1);
    
    lua_pushinteger(L, signal_handles.assign(inserted.first));
    
    return 2;
}

static int register_event_handler(lua_State * L)
{
    const char * name = luaL_checkstring(L, 1);
    
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    
    script::env_object::shared_ptr luaFunctionObject = new script::lua::lua_function(L);
    luaFunctionObject->set_adopted();

    int handle = slots.create_slot(name, luaFunctionObject, env);
    
    if(handle == -1) // signal was not found
    {
        std::map<std::string, lua_function_vector>::iterator it = created_event_slots.find(name);
        if(it != created_event_slots.end())
        {
            lua_function_vector::iterator pos = get_free_vector_iterator(it->second);
            
            lua_pushvalue(L, 2);
            *pos = luaL_ref(L, LUA_REGISTRYINDEX);
            
            handle = slots.skip_slot_id();
            handle_to_slot[handle] = std::pair<lua_function_vector *, int>(&it->second, static_cast<int>(pos - it->second.begin()));
        }
    }
    
    lua_pushinteger(L, handle);
    
    return 1;
}

static int cancel_signal(lua_State * L)
{
    int signalId = luaL_checkint(L, 1);
    created_event_slots_iterator iter = signal_handles.resolve(signalId);
    if(iter == created_event_slots_iterator()) return 0;
    
    for(lua_function_vector::const_iterator functionIter = iter->second.begin(); functionIter != iter->second.end(); ++functionIter)
        luaL_unref(L, LUA_REGISTRYINDEX, *functionIter);
    
    created_event_slots.erase(iter);
    signal_handles.free(signalId);
    
    return 0;
}

static void cancel_event_handler(lua_State * L, int handle)
{
    handle_slot_map::iterator it = handle_to_slot.find(handle);
    if(it == handle_to_slot.end()) return;
    int index = it->second.second;
    int luaFunctionRef = (*it->second.first)[index];
    luaL_unref(L, LUA_REGISTRYINDEX, luaFunctionRef);
    (*it->second.first)[index] = -1;
}

static int destroy_slot(lua_State * L)
{
    int handle = luaL_checkint(L, 1);
    if(handle < 0) return 0;
    bool done = slots.destroy_slot(handle);
    if(!done) cancel_event_handler(L, handle);
    return 0;
}

static void cleanup()
{
    handle_to_slot.clear();
    created_event_slots.clear();
}

} //namespace lua

namespace cubescript{    
static int register_event_handler(const std::string & name, script::any obj)
{
    if(obj.get_type() != typeid(script::env_object::shared_ptr)) throw script::error(script::BAD_CAST);
    return slots.create_slot(name, script::any_cast<script::env_object::shared_ptr>(obj), env);
}
}//namespace cubescript

/*
    Cancel event handler
*/
static void destroy_slot(int handle)
{
    if(handle < 0) return;
    slots.destroy_slot(handle);
}

static void cleanup(int)
{
    slots.clear();
    slots.deallocate_destroyed_slots();
}

void register_signals(script::env & env)
{
    ::env = &env;
    
    signal_shutdown.connect(&cleanup, boost::signals2::at_front);
    
    slots.register_signal(signal_started, "started", normal_error_handler);
    slots.register_signal(signal_shutdown,"shutdown",normal_error_handler, boost::signals2::at_front);
    slots.register_signal(signal_shutdown_scripting, "shutdown_scripting", normal_error_handler);
    slots.register_signal(signal_reloadhopmod, "reloadhopmod", normal_error_handler);
    slots.register_signal(signal_maintenance, "maintenance", normal_error_handler);
    
    slots.register_signal(signal_connecting, "connecting", normal_error_handler);
    slots.register_signal(signal_connect,"connect",normal_error_handler);
    slots.register_signal(signal_disconnect,"disconnect",normal_error_handler);
    slots.register_signal(signal_failedconnect, "failedconnect",normal_error_handler);
    slots.register_signal(signal_maploaded, "maploaded", normal_error_handler);
    slots.register_signal(signal_allow_rename,"allow_rename",normal_error_handler);
    slots.register_signal(signal_rename,"rename",normal_error_handler);
    slots.register_signal(signal_renaming, "renaming", normal_error_handler);
    slots.register_signal(signal_reteam, "reteam", normal_error_handler);
    slots.register_signal(signal_chteamrequest, "chteamrequest", proceed_error_handler);
    slots.register_signal(signal_text,"text",proceed_error_handler);
    slots.register_signal(signal_sayteam,"sayteam",proceed_error_handler);
    slots.register_signal(signal_intermission,"intermission", normal_error_handler);
    slots.register_signal(signal_finishedgame, "finishedgame", normal_error_handler);
    slots.register_signal(signal_timeupdate,"timeupdate", normal_error_handler);
    slots.register_signal(signal_mapchange,"mapchange", normal_error_handler);
    slots.register_signal(signal_mapvote, "mapvote", proceed_error_handler);
    slots.register_signal(signal_setnextgame, "setnextgame", normal_error_handler);
    slots.register_signal(signal_gamepaused, "gamepaused", normal_error_handler);
    slots.register_signal(signal_gameresumed, "gameresumed", normal_error_handler);
    slots.register_signal(signal_setmastermode_request, "setmastermode_request", proceed_error_handler);
    slots.register_signal(signal_setmastermode, "setmastermode", proceed_error_handler);
    slots.register_signal(signal_spectator, "spectator", normal_error_handler);
    slots.register_signal(signal_privilege, "privilege", normal_error_handler);
    slots.register_signal(signal_teamkill, "teamkill", normal_error_handler);
    slots.register_signal(signal_frag, "frag", normal_error_handler);
    slots.register_signal(signal_authreq, "request_auth_challenge", normal_error_handler);
    slots.register_signal(signal_authrep, "auth_challenge_response", normal_error_handler);
    slots.register_signal(signal_addbot, "addbot", normal_error_handler);
    slots.register_signal(signal_delbot, "delbot", normal_error_handler);
    slots.register_signal(signal_botleft, "botleft", normal_error_handler);
    slots.register_signal(signal_beginrecord, "beginrecord", normal_error_handler);
    slots.register_signal(signal_endrecord, "endrecord", normal_error_handler);
    slots.register_signal(signal_mapcrc, "mapcrc", normal_error_handler);
    slots.register_signal(signal_checkmaps, "checkmaps", normal_error_handler);
    slots.register_signal(signal_votepassed, "votepassed", normal_error_handler);
    slots.register_signal(signal_shot, "shot", normal_error_handler);
    slots.register_signal(signal_suicide, "suicide", normal_error_handler);
    slots.register_signal(signal_takeflag, "takeflag", normal_error_handler);
    slots.register_signal(signal_dropflag, "dropflag", normal_error_handler);
    slots.register_signal(signal_scoreflag, "scoreflag", normal_error_handler);
    slots.register_signal(signal_returnflag, "returnflag", normal_error_handler);
    slots.register_signal(signal_resetflag, "resetflag", normal_error_handler);
    slots.register_signal(signal_scoreupdate, "scoreupdate", normal_error_handler);
    slots.register_signal(signal_spawn, "spawn", normal_error_handler);
    slots.register_signal(signal_damage, "damage", normal_error_handler);
    slots.register_signal(signal_setmaster, "setmaster", normal_error_handler);
    slots.register_signal(signal_respawnrequest, "respawn_request", maxvalue_error_handler); 
   
    slots.register_signal(signal_clearbans_request, "clearbans_request", normal_error_handler);
    slots.register_signal(signal_kick_request, "kick_request", normal_error_handler);
    
    slots.register_signal(signal_varchanged, "varchanged", normal_error_handler);
    
    script::bind_freefunc(cubescript::register_event_handler, "event_handler", env);
    script::bind_freefunc(destroy_slot, "cancel_handler", env);
    
    register_lua_function(lua::register_event_handler,"event_handler");
    register_lua_function(lua::destroy_slot, "cancel_handler");
    register_lua_function(lua::create_signal, "create_event_signal");
    register_lua_function(lua::cancel_signal, "cancel_event_signal");
}

void cleanup_dead_slots()
{
    slots.deallocate_destroyed_slots();
}

void disconnect_all_slots()
{
    signal_started.disconnect_all_slots();
    signal_shutdown.disconnect_all_slots();
    signal_shutdown_scripting.disconnect_all_slots();
    signal_reloadhopmod.disconnect_all_slots();
    signal_maintenance.disconnect_all_slots();
    signal_connecting.disconnect_all_slots();
    signal_connect.disconnect_all_slots();
    signal_disconnect.disconnect_all_slots();
    signal_failedconnect.disconnect_all_slots();
    signal_maploaded.disconnect_all_slots();
    signal_allow_rename.disconnect_all_slots();
    signal_rename.disconnect_all_slots();
    signal_renaming.disconnect_all_slots();
    signal_reteam.disconnect_all_slots();
    signal_chteamrequest.disconnect_all_slots();
    signal_text.disconnect_all_slots();
    signal_sayteam.disconnect_all_slots();
    signal_intermission.disconnect_all_slots();
    signal_finishedgame.disconnect_all_slots();
    signal_timeupdate.disconnect_all_slots();
    signal_mapchange.disconnect_all_slots();
    signal_mapvote.disconnect_all_slots();
    signal_setnextgame.disconnect_all_slots();
    signal_gamepaused.disconnect_all_slots();
    signal_gameresumed.disconnect_all_slots();
    signal_setmastermode_request.disconnect_all_slots();
    signal_setmastermode.disconnect_all_slots();
    signal_spectator.disconnect_all_slots();
    signal_privilege.disconnect_all_slots();
    signal_teamkill.disconnect_all_slots();
    signal_frag.disconnect_all_slots();
    signal_authreq.disconnect_all_slots();
    signal_authrep.disconnect_all_slots();
    signal_addbot.disconnect_all_slots();
    signal_delbot.disconnect_all_slots();
    signal_botleft.disconnect_all_slots();
    signal_beginrecord.disconnect_all_slots();
    signal_endrecord.disconnect_all_slots();
    signal_mapcrc.disconnect_all_slots();
    signal_checkmaps.disconnect_all_slots();
    signal_votepassed.disconnect_all_slots();
    signal_shot.disconnect_all_slots();
    signal_suicide.disconnect_all_slots();
    signal_takeflag.disconnect_all_slots();
    signal_dropflag.disconnect_all_slots();
    signal_scoreflag.disconnect_all_slots();
    signal_returnflag.disconnect_all_slots();

    signal_resetflag.disconnect_all_slots();
    signal_scoreupdate.disconnect_all_slots();
    signal_spawn.disconnect_all_slots();
    signal_damage.disconnect_all_slots();
    signal_respawnrequest.disconnect_all_slots();
    signal_kick_request.disconnect_all_slots();
    signal_clearbans_request.disconnect_all_slots();
    signal_varchanged.disconnect_all_slots();
    
    lua::cleanup();
}
