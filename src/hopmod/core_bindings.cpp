#include "cube.h"
#include "game.h"
#include "hopmod.hpp"
#include "extapi.hpp"
extern bool reloaded; // Defined in startup.cpp

/* Forward declaration of Lua value io functions */
#include "lua/push_function_fwd.hpp"
namespace lua{
void push(lua_State * L, string value);
void push(lua_State * L, __uid_t value);
} //namespace lua

#include "lua/push_function.hpp"
#include <iostream>
#include <vector>
#include <string>

/*
    Lua value io functions for cube2 types
*/
namespace lua{
void push(lua_State * L, string value)
{
    lua_pushstring(L, value);   
}
void push(lua_State * L, __uid_t value)
{
    lua_pushinteger(L, value);
}
} //namespace lua

template<typename FunctionPointerType>
static void bind_function(lua_State * L, int table, const char * name, FunctionPointerType function)
{
    lua_pushstring(L, name);
    lua::push_function(L, function);
    lua_settable(L, table);
}

static void bind_function(lua_State * L, int table, const char * name, lua_CFunction function)
{
    lua_pushstring(L, name);
    lua_pushcfunction(L, function);
    lua_settable(L, table);
}

void bind_core_functions(lua_State * L, int T)
{
    bind_function(L, T, "player_msg", server::player_msg);
    bind_function(L, T, "player_name", server::player_name);
    bind_function(L, T, "player_rename", server::player_rename);
    bind_function(L, T, "player_displayname", server::player_displayname);
    bind_function(L, T, "player_team", server::player_team);
    bind_function(L, T, "player_priv", server::player_privilege);
    bind_function(L, T, "player_priv_code", server::player_privilege_code);
    bind_function(L, T, "player_id", server::player_id);
    bind_function(L, T, "player_sessionid", server::player_sessionid);
    bind_function(L, T, "player_ping", server::player_ping);
    bind_function(L, T, "player_ping_update", server::player_ping_update);
    bind_function(L, T, "player_lag", server::player_lag);
    bind_function(L, T, "player_real_lag", server::player_real_lag);
    bind_function(L, T, "player_deathmillis", server::player_deathmillis);
    bind_function(L, T, "player_ip", server::player_ip);
    bind_function(L, T, "player_iplong", server::player_iplong);
    bind_function(L, T, "player_status", server::player_status);
    bind_function(L, T, "player_status_code", server::player_status_code);
    bind_function(L, T, "player_frags", server::player_frags);
    bind_function(L, T, "player_score", server::player_score);
    bind_function(L, T, "player_deaths", server::player_deaths);
    bind_function(L, T, "player_suicides", server::player_suicides);
    bind_function(L, T, "player_teamkills", server::player_teamkills);
    bind_function(L, T, "player_damage", server::player_damage);
    bind_function(L, T, "player_damagewasted", server::player_damagewasted);
    bind_function(L, T, "player_maxhealth", server::player_maxhealth);
    bind_function(L, T, "player_health", server::player_health);
    bind_function(L, T, "player_armour", server::player_armour);
    bind_function(L, T, "player_armour_type", server::player_armour_type);
    bind_function(L, T, "player_gun", server::player_gun);
    bind_function(L, T, "player_hits", server::player_hits);
    bind_function(L, T, "player_misses", server::player_misses);
    bind_function(L, T, "player_shots", server::player_shots);
    bind_function(L, T, "player_accuracy", server::player_accuracy);
    bind_function(L, T, "player_accuracy2", server::player_accuracy2);
    bind_function(L, T, "player_is_spy", server::player_is_spy);
    bind_function(L, T, "player_clientmillis", server::player_clientmillis);
    bind_function(L, T, "player_timetrial", server::player_timetrial);
    bind_function(L, T, "player_timeplayed", server::player_timeplayed);
    bind_function(L, T, "player_win", server::player_win);
    bind_function(L, T, "player_slay", server::player_slay);
    bind_function(L, T, "player_respawn", server::player_respawn);
    bind_function(L, T, "player_nospawn", server::player_nospawn);
    bind_function(L, T, "player_suicide", (void (*)(int))server::suicide);
    bind_function(L, T, "changeteam", server::player_changeteam);
    bind_function(L, T, "player_bots", server::player_bots);
    bind_function(L, T, "player_rank", server::player_rank);
    bind_function(L, T, "player_isbot", server::player_isbot);
    bind_function(L, T, "player_pos", server::player_pos);
    bind_function(L, T, "player_freeze", server::player_freeze);
    bind_function(L, T, "player_unfreeze", server::player_unfreeze);
    bind_function(L, T, "player_connection_time", server::player_connection_time);
    bind_function(L, T, "disconnect", server::disconnect);
    bind_function(L, T, "force_spec", server::player_force_spec);
    bind_function(L, T, "unforce_spec", server::player_unforce_spec);
    bind_function(L, T, "spec", server::player_spec);
    bind_function(L, T, "unspec", server::player_unspec);
    bind_function(L, T, "player_forgive_tk", server::player_forgive_tk);
    bind_function(L, T, "unsetmaster", server::unsetmaster);
    bind_function(L, T, "setmaster", server::set_player_master);
    bind_function(L, T, "setadmin", server::set_player_admin);
    bind_function(L, T, "setspy", server::set_spy);
    bind_function(L, T, "set_invisible_admin", server::set_player_private_admin);
    bind_function(L, T, "set_invisible_master", server::set_player_private_master);
    bind_function(L, T, "unsetpriv", server::unset_player_privilege);
    
    bind_function(L, T, "send_auth_request", server::send_auth_request);
    bind_function(L, T, "send_auth_challenge_to_client", server::send_auth_challenge);
    
    bind_function(L, T, "players", server::cs_player_list);
    bind_function(L, T, "spectators", server::cs_spec_list);
    bind_function(L, T, "bots", server::cs_bot_list);
    bind_function(L, T, "clients", server::cs_client_list);
    
    bind_function(L, T, "teams", server::get_teams);
    bind_function(L, T, "team_msg", server::team_msg);
    bind_function(L, T, "team_score", server::get_team_score);
    bind_function(L, T, "team_win", server::team_win);
    bind_function(L, T, "team_draw", server::team_draw);
    bind_function(L, T, "team_players", server::get_team_players);
    bind_function(L, T, "teamsize", server::team_size);
    
    bind_function(L, T, "get_gamemode_info", server::lua_gamemodeinfo);
    bind_function(L, T, "pausegame", server::pausegame);
    
    bind_function(L, T, "msg", server::sendservmsg);

    bind_function(L, T, "changetime", server::changetime);
    bind_function(L, T, "changemap", server::changemap);
    bind_function(L, T, "addbot", server::addbot);
    bind_function(L, T, "delbot", server::deletebot);
    bind_function(L, T, "recorddemo", server::recorddemo);
    bind_function(L, T, "stopdemo", server::enddemorecord);
    bind_function(L, T, "allow_ip", server::add_allowed_ip);
    bind_function(L, T, "shutdown", server::shutdown);
    bind_function(L, T, "restart_now", restart_now);
    bind_function(L, T, "reload_lua", reload_hopmod);

    bind_function(L, T, "add_item", server::add_item);
    bind_function(L, T, "add_flag", server::add_flag);
    bind_function(L, T, "prepare_hold_mode", server::prepare_hold_mode);
    bind_function(L, T, "add_base", server::add_base);
    bind_function(L, T, "prepare_capture_mode", server::prepare_capture_mode);
    
    bind_function(L, T, "file_exists", file_exists);
    bind_function(L, T, "dir_exists", dir_exists);
    
    bind_function(L, T, "sleep", lua::sleep);
    bind_function(L, T, "interval", lua::interval);
    bind_function(L, T, "cancel_timer", lua::cancel_timer);
    
    bind_function(L, T, "log_event_error", log_event_error);
    
    int get_lua_stack_size();
    bind_function(L, T, "lua_stack_size", get_lua_stack_size);
}

template<int Constant>
static int get_constant(lua_State * L)
{
    if(lua_gettop(L)) luaL_error(L, "cannot set constant");
    lua_pushinteger(L, Constant);
    return 1;   
}

void bind_core_constants(lua_State * L, int T)
{    
    bind_function(L, T, "DISC_NONE", get_constant<DISC_NONE>);
    bind_function(L, T, "DISC_EOP", get_constant<DISC_EOP>);
    bind_function(L, T, "DISC_CN", get_constant<DISC_CN>);
    bind_function(L, T, "DISC_KICK", get_constant<DISC_KICK>);
    bind_function(L, T, "DISC_TAGT", get_constant<DISC_TAGT>);
    bind_function(L, T, "DISC_IPBAN", get_constant<DISC_IPBAN>);
    bind_function(L, T, "DISC_PRIVATE", get_constant<DISC_PRIVATE>);
    bind_function(L, T, "DISC_MAXCLIENTS", get_constant<DISC_MAXCLIENTS>);
    bind_function(L, T, "DISC_TIMEOUT", get_constant<DISC_TIMEOUT>);
    bind_function(L, T, "DISC_OVERFLOW", get_constant<DISC_OVERFLOW>);
    bind_function(L, T, "DISC_NUM", get_constant<DISC_NUM>);
    
    bind_function(L, T, "DISC_NONE", get_constant<DISC_NONE>);
    bind_function(L, T, "DISC_EOP", get_constant<DISC_EOP>);
    bind_function(L, T, "DISC_CN", get_constant<DISC_CN>);
    bind_function(L, T, "DISC_KICK", get_constant<DISC_KICK>);
    bind_function(L, T, "DISC_TAGT", get_constant<DISC_TAGT>);
    bind_function(L, T, "DISC_IPBAN", get_constant<DISC_IPBAN>);
    bind_function(L, T, "DISC_PRIVATE", get_constant<DISC_PRIVATE>);
    bind_function(L, T, "DISC_MAXCLIENTS", get_constant<DISC_MAXCLIENTS>);
    bind_function(L, T, "DISC_TIMEOUT", get_constant<DISC_TIMEOUT>);
    bind_function(L, T, "DISC_OVERFLOW", get_constant<DISC_OVERFLOW>);
    bind_function(L, T, "DISC_NUM", get_constant<DISC_NUM>);

    bind_function(L, T, "PRIV_NONE", get_constant<PRIV_NONE>);
    bind_function(L, T, "PRIV_MASTER", get_constant<PRIV_MASTER>);
    bind_function(L, T, "PRIV_ADMIN", get_constant<PRIV_ADMIN>);

    bind_function(L, T, "GREEN_ARMOUR", get_constant<A_GREEN>);
    bind_function(L, T, "YELLOW_ARMOUR", get_constant<A_YELLOW>);

    bind_function(L, T, "ALIVE", get_constant<CS_ALIVE>);
    bind_function(L, T, "DEAD", get_constant<CS_DEAD>);
    bind_function(L, T, "SPAWNING", get_constant<CS_SPAWNING>);
    bind_function(L, T, "LAGGED", get_constant<CS_LAGGED>);
    bind_function(L, T, "SPECTATOR", get_constant<CS_SPECTATOR>);
    bind_function(L, T, "EDITING", get_constant<CS_EDITING>);
    
    bind_function(L, T, "MM_OPEN", get_constant<MM_OPEN>);
    bind_function(L, T, "MM_VETO", get_constant<MM_VETO>);
    bind_function(L, T, "MM_LOCKED", get_constant<MM_LOCKED>);
    bind_function(L, T, "MM_PRIVATE", get_constant<MM_PRIVATE>);
    bind_function(L, T, "MM_PASSWORD", get_constant<MM_PASSWORD>);
}

template<typename T, bool READ_ONLY, bool WRITE_ONLY>
static int variable_accessor(lua_State * L)
{
    T * var = reinterpret_cast<T *>(lua_touserdata(L, lua_upvalueindex(1)));
    if(lua_gettop(L) > 0) // Set variable
    {
        if(READ_ONLY) luaL_error(L, "variable is read-only");
        *var = lua::to(L, 1, lua::return_tag<T>());
        event_varchanged(event_listeners(), boost::make_tuple(lua_tostring(L, lua_upvalueindex(2))));
        return 0;
    }
    else // Get variable
    {
        if(WRITE_ONLY) luaL_error(L, "variable is write-only");
        lua::push(L, *var);
        return 1;
    }
}

template<bool READ_ONLY, bool WRITE_ONLY>
static int string_accessor(lua_State * L)
{
    char * var = reinterpret_cast<char *>(lua_touserdata(L, lua_upvalueindex(1)));
    if(lua_gettop(L) > 0) // Set variable
    {
        if(READ_ONLY) luaL_error(L, "variable is read-only");
        copystring(var, lua_tostring(L, 1));
        event_varchanged(event_listeners(), boost::make_tuple(lua_tostring(L, lua_upvalueindex(2))));
        return 0;
    }
    else // Get variable
    {
        if(WRITE_ONLY) luaL_error(L, "variable is write-only");
        lua::push(L, var);
        return 1;
    }
}

template<typename T>
static void bind_var(lua_State * L, int table, const char * name, T & var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, &var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, variable_accessor<T, false, false>, 2);
    lua_settable(L, table);
}

static void bind_var(lua_State * L, int table, const char * name, string var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, string_accessor<false, false>, 2);
    lua_settable(L, table);
}

template<typename T>
static void bind_ro_var(lua_State * L, int table, const char * name, T & var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, &var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, variable_accessor<T, true, false>, 2);
    lua_settable(L, table);
}

static void bind_ro_var(lua_State * L, int table, const char * name, string var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, string_accessor<true, false>, 2);
    lua_settable(L, table);
}

template<typename T>
static void bind_wo_var(lua_State * L, int table, const char * name, T & var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, &var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, variable_accessor<T, false, true>, 2);
    lua_settable(L, table);
}

static void bind_wo_var(lua_State * L, int table, const char * name, string var)
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, var);
    lua_pushstring(L, name);
    lua_pushcclosure(L, string_accessor<false, true>, 2);
    lua_settable(L, table);
}

template<typename T>
static int property_accessor(lua_State * L)
{
    T (* get)() = reinterpret_cast<T (*)()>(lua_touserdata(L, lua_upvalueindex(1)));
    void (* set)(T) = reinterpret_cast<void (*)(T)>(lua_touserdata(L, lua_upvalueindex(2)));
    if(lua_gettop(L) > 0)
    {
        if(!set) luaL_error(L, "cannot set value");
        set(lua::to(L, 1, lua::return_tag<T>()));
        event_varchanged(event_listeners(), boost::make_tuple(lua_tostring(L, lua_upvalueindex(3))));
        return 0;
    }
    else
    {
        if(!get) luaL_error(L, "cannot get value");
        lua::push(L, get());
        return 1;
    }
}

template<typename T>
static void bind_prop(lua_State * L, int table, const char * name, T (* get)(), void (* set)(T))
{
    lua_pushstring(L, name);
    lua_pushlightuserdata(L, reinterpret_cast<void *>(get));
    lua_pushlightuserdata(L, reinterpret_cast<void *>(set));
    lua_pushstring(L, name);
    lua_pushcclosure(L, property_accessor<T>, 3);
    lua_settable(L, table);
}

void bind_core_variables(lua_State * L, int T)
{
    bind_ro_var(L, T, "paused", server::gamepaused);
    bind_var(L, T, "servername", server::serverdesc);
    bind_ro_var(L, T, "map", server::smapname);
    bind_var(L, T, "server_password", server::serverpass);
    bind_ro_var(L, T, "master", server::currentmaster);
    bind_prop(L, T, "timeleft", server::get_minutes_left, server::set_minutes_left);
    bind_prop(L, T, "seconds_left", server::get_seconds_left, server::set_seconds_left);
    bind_var(L, T, "intermission", server::interm);
    bind_var(L, T, "intermission_time", server::intermtime);
    bind_ro_var(L, T, "uptime", totalmillis);
    bind_ro_var(L, T, "gamemillis", server::gamemillis);
    bind_ro_var(L, T, "gamelimit", server::gamelimit);
    bind_var(L, T, "maxplayers", maxclients);
    bind_var(L, T, "maxclients", maxclients);
    bind_var(L, T, "serverip", serverip);
    bind_var(L, T, "serverport", serverport);
    bind_var(L, T, "next_mode", server::next_gamemode);
    bind_var(L, T, "next_map", server::next_mapname);
    bind_var(L, T, "next_gametime", server::next_gametime);
    bind_var(L, T, "reassignteams", server::reassignteams);
    bind_prop<int>(L, T, "playercount", server::getplayercount, NULL);
    bind_prop<int>(L, T, "speccount", server::getspeccount, NULL);
    bind_prop<int>(L, T, "botcount", server::getbotcount, NULL);
    bind_var(L, T, "botlimit", server::aiman::botlimit);
    bind_var(L, T, "botbalance", server::aiman::botbalance);
    bind_prop<const char *>(L, T, "gamemode", server::gamemodename, NULL);
    bind_var(L, T, "display_open", server::display_open);
    bind_var(L, T, "allow_mastermode_veto", server::allow_mm_veto);
    bind_var(L, T, "allow_mastermode_locked", server::allow_mm_locked);
    bind_var(L, T, "allow_mastermode_private", server::allow_mm_private);
    bind_var(L, T, "reserved_slots", server::reservedslots);
    bind_wo_var(L, T, "reserved_slots_password", server::slotpass);
    bind_ro_var(L, T, "reserved_slots_occupied", server::reservedslots_use);
    bind_ro_var(L, T, "reloaded", reloaded);
    bind_prop<__uid_t>(L, T, "UID", getuid, NULL);
    bind_var(L, T, "spectator_delay", server::spectator_delay);
    bind_var(L, T, "ctf_teamkill_penalty", server::ctftkpenalty);
    bind_var(L, T, "specslots", server::spec_slots);
    
    bind_var(L, T, "flood_protect_text", server::sv_text_hit_length);
    bind_var(L, T, "flood_protect_sayteam", server::sv_sayteam_hit_length);
    bind_var(L, T, "flood_protect_mapvote", server::sv_mapvote_hit_length);
    bind_var(L, T, "flood_protect_switchname", server::sv_switchname_hit_length);
    bind_var(L, T, "flood_protect_switchteam", server::sv_switchteam_hit_length);
    bind_var(L, T, "flood_protect_kick", server::sv_kick_hit_length);
    bind_var(L, T, "flood_protect_remip", server::sv_remip_hit_length);
    bind_var(L, T, "flood_protect_newmap", server::sv_newmap_hit_length);
    bind_var(L, T, "flood_protect_spectator", server::sv_spec_hit_length);
    
    bind_ro_var(L, T, "tx_bytes", tx_bytes);
    bind_ro_var(L, T, "rx_bytes", rx_bytes);
    bind_ro_var(L, T, "tx_packets", tx_packets);
    bind_ro_var(L, T, "rx_packets", rx_packets);
        
    bind_var(L, T, "timer_alarm_threshold", server::timer_alarm_threshold);
    bind_var(L, T, "enable_extinfo", server::enable_extinfo);
    
    bind_prop<int>(L, T, "mastermode", server::get_mastermode, server::script_set_mastermode);
}

