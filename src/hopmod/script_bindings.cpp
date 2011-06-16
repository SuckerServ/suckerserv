#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "cube.h"
#include "game.h"
#include "hopmod.hpp"
#include "extapi.hpp"
#include "string_var.hpp"
#include <fungu/script.hpp>
#include <fungu/script/variable.hpp>
using namespace fungu;
#include <unistd.h>

static std::vector<script::any> changemap_defargs;
static std::vector<script::any> recorddemo_defargs;
extern bool reloaded; //startup.cpp

namespace server{
    extern int spectator_delay;
 }

static void setup_default_arguments()
{    
    changemap_defargs.clear();
    changemap_defargs.push_back(static_cast<const char *>(""));
    changemap_defargs.push_back(-1);
    
    recorddemo_defargs.clear();
    recorddemo_defargs.push_back(static_cast<const char *>(""));
}

namespace hopmod{

class observed_variable:public script::env_object
{
public:
    observed_variable(script::env_object * decorated, const char *id)
    :m_decorated(decorated), m_id(id)
    {
        
    }
    
    script::env_object::object_type get_object_type()const
    {
        return m_decorated->get_object_type();
    }
    
    void assign(const script::any & value)
    {
        m_decorated->assign(value);
        signal_varchanged(m_id);
    }
    
    script::any call(script::env_object::call_arguments & args, script::env_frame * frm)
    {
        script::any ret =  m_decorated->call(args, frm);
        signal_varchanged(m_id);
        return ret;
    }
    
    script::any value()
    {
        return m_decorated->value();
    }
    
    #ifdef FUNGU_WITH_LUA
    void value(lua_State * L)
    {
        m_decorated->value(L);
    }
    #endif
private:
    script::env_object::shared_ptr m_decorated;
    const char * m_id;
};

template<typename T>
inline script::env_object * bind_var(T & ref, const char * id, script::env & environ)
{
    observed_variable * var = new observed_variable(script::bind_var(ref, id, environ), id);
    var->set_adopted();
    environ.bind_global_object(var, const_string::literal(id));
    return var;
}

template<typename T>
inline void bind_wo_var(T & ref, const char * id, script::env & environ)
{
    observed_variable * var = new observed_variable(script::bind_wo_var(ref, id, environ), id);
    var->set_adopted();
    environ.bind_global_object(var, const_string::literal(id));
}

template<typename T,typename Functor>
inline void bind_funvar(Functor fun, const char * id, script::env & environ)
{
    observed_variable * var = new observed_variable(script::bind_funvar<T>(fun, id, environ), id);
    var->set_adopted();
    environ.bind_global_object(var, const_string::literal(id));
}

template<typename T, typename GetterFunction, typename SetterFunction>
inline script::env_object * bind_property(GetterFunction getter, SetterFunction setter, const char * id, script::env & environ)
{
    observed_variable * var = new observed_variable(script::bind_property<T>(getter, setter, id, environ), id);
    var->set_adopted();
    environ.bind_global_object(var, const_string::literal(id));
    return var;
}

} //namespace hopmod

void register_server_script_bindings(script::env & env)
{
    setup_default_arguments();
    
    // Player-oriented functions
    script::bind_freefunc(server::player_msg, "player_msg", env);
    script::bind_const((int)DISC_NONE, "DISC_NONE", env);
    script::bind_const((int)DISC_EOP, "DISC_EOP", env);
    script::bind_const((int)DISC_CN, "DISC_CN", env);
    script::bind_const((int)DISC_KICK, "DISC_KICK", env);
    script::bind_const((int)DISC_TAGT, "DISC_TAGT", env);
    script::bind_const((int)DISC_IPBAN, "DISC_IPBAN", env);
    script::bind_const((int)DISC_PRIVATE, "DISC_PRIVATE", env);
    script::bind_const((int)DISC_MAXCLIENTS, "DISC_MAXCLIENTS", env);
    script::bind_const((int)DISC_TIMEOUT, "DISC_TIMEOUT", env);
    script::bind_const((int)DISC_OVERFLOW, "DISC_OVERFLOW", env);
    script::bind_const((int)DISC_NUM, "DISC_NUM", env);
    script::bind_freefunc(server::disconnect, "disconnect", env);
    script::bind_freefunc(server::player_name, "player_name", env);
    script::bind_freefunc(server::player_rename, "player_rename", env);
    script::bind_freefunc(server::player_displayname, "player_displayname", env);
    script::bind_freefunc(server::player_team, "player_team", env);
    script::bind_freefunc(server::player_privilege, "player_priv", env);
    script::bind_freefunc(server::player_privilege_code, "player_priv_code", env);
    script::bind_const((int)PRIV_NONE, "PRIV_NONE", env);
    script::bind_const((int)PRIV_MASTER, "PRIV_MASTER", env);
    script::bind_const((int)PRIV_ADMIN, "PRIV_ADMIN", env);
    script::bind_freefunc(server::player_id, "player_id", env);
    script::bind_freefunc(clear_player_ids, "clear_player_ids", env);
    script::bind_freefunc(server::player_sessionid, "player_sessionid", env);
    script::bind_freefunc(server::player_ping, "player_ping", env);
    script::bind_freefunc(server::player_ping_update, "player_ping_update", env);
    script::bind_freefunc(server::player_lag, "player_lag", env);
    script::bind_freefunc(server::player_ip, "player_ip", env);
    script::bind_freefunc(server::player_iplong, "player_iplong", env);
    script::bind_freefunc(server::player_status, "player_status", env);
    script::bind_freefunc(server::player_status_code, "player_status_code", env);
    script::bind_freefunc(server::player_frags, "player_frags", env);
    script::bind_freefunc(server::player_score, "player_score", env);
    script::bind_freefunc(server::player_deaths, "player_deaths", env);
    script::bind_freefunc(server::player_suicides, "player_suicides", env);
    script::bind_freefunc(server::player_teamkills, "player_teamkills", env);
    script::bind_freefunc(server::player_damage, "player_damage", env);
    script::bind_freefunc(server::player_damagewasted, "player_damagewasted", env);
    script::bind_freefunc(server::player_maxhealth, "player_maxhealth", env);
    script::bind_freefunc(server::player_health, "player_health", env);
    script::bind_freefunc(server::player_armour, "player_armour", env);
    script::bind_freefunc(server::player_armour_type, "player_armour_type", env);
    script::bind_const((int)A_GREEN, "GREEN_ARMOUR", env);
    script::bind_const((int)A_YELLOW, "YELLOW_ARMOUR", env);
    script::bind_freefunc(server::player_gun, "player_gun", env);
    script::bind_freefunc(server::player_hits, "player_hits", env);
    script::bind_freefunc(server::player_misses, "player_misses", env);
    script::bind_freefunc(server::player_shots, "player_shots", env);
    script::bind_freefunc(server::player_accuracy, "player_accuracy", env);
    script::bind_freefunc(server::player_timeplayed, "player_timeplayed", env);
    script::bind_freefunc(server::player_win, "player_win", env);
    script::bind_freefunc(server::player_slay, "player_slay", env);
    script::bind_freefunc(server::player_respawn, "player_respawn", env);
    script::bind_freefunc(server::player_nospawn, "player_nospawn", env);
    script::bind_freefunc((void (*)(int))server::suicide, "player_suicide", env);
    script::bind_freefunc(server::player_changeteam, "changeteam", env);
    script::bind_freefunc(server::player_bots, "player_bots", env);
    script::bind_freefunc(server::player_rank, "player_rank", env);
    script::bind_freefunc(server::player_isbot, "player_isbot", env);
    script::bind_freefunc(server::player_mapcrc, "player_mapcrc", env);
    script::bind_freefunc((std::vector<float>(*)(int))server::player_pos, "player_pos", env);
    register_lua_function((int (*)(lua_State *))&server::player_pos, "player_pos");
    script::bind_freefunc(server::send_auth_request, "send_auth_request", env);
    script::bind_freefunc(server::send_auth_challenge, "send_auth_challenge_to_client", env);
    //script::bind_freefunc(server::send_item, "send_item", env);
    script::bind_freefunc(server::player_freeze, "player_freeze", env);
    script::bind_freefunc(server::player_unfreeze, "player_unfreeze", env);
    
    script::bind_const((int)CS_ALIVE, "ALIVE", env);
    script::bind_const((int)CS_DEAD, "DEAD", env);
    script::bind_const((int)CS_SPAWNING, "SPAWNING", env);
    script::bind_const((int)CS_LAGGED, "LAGGED", env);
    script::bind_const((int)CS_SPECTATOR, "SPECTATOR", env);
    script::bind_const((int)CS_EDITING, "EDITING", env);
    script::bind_freefunc(server::player_connection_time, "player_connection_time", env);
    script::bind_freefunc(server::player_force_spec, "force_spec", env);
    script::bind_freefunc(server::player_spec, "spec", env);
    script::bind_freefunc(server::player_unspec, "unspec", env);
    script::bind_freefunc(server::player_unforce_spec, "unforce_spec", env);
    script::bind_freefunc(server::unsetmaster, "unsetmaster", env);
    script::bind_freefunc(server::set_player_master, "setmaster", env);
    script::bind_freefunc(server::set_player_admin, "setadmin", env);
    script::bind_freefunc(server::set_player_private_admin, "set_invisible_admin", env);
    script::bind_freefunc(server::set_player_private_master, "set_invisible_master", env);
    script::bind_freefunc(server::unset_player_privilege, "unsetpriv", env);
    
    script::bind_freefunc(server::cs_player_list, "players", env);
    script::bind_freefunc(server::cs_spec_list, "spectators", env);
    script::bind_freefunc(server::cs_bot_list, "bots", env);
    script::bind_freefunc(server::cs_client_list, "clients", env);
    
    register_lua_function(&server::lua_player_list, "players");
    register_lua_function(&server::lua_spec_list, "spectators");
    register_lua_function(&server::lua_bot_list, "bots");
    register_lua_function(&server::lua_client_list, "clients");
    
    register_lua_function(&server::lua_gamemodeinfo, "get_gamemode_info");
    
    // Team-oriented functions
    script::bind_freefunc(server::team_msg,"team_msg", env);
    script::bind_freefunc(server::get_teams, "teams", env);
    register_lua_function(&server::lua_team_list, "teams");
    script::bind_freefunc(server::get_team_score, "team_score", env);
    script::bind_freefunc(server::team_win, "team_win", env);
    script::bind_freefunc(server::team_draw, "team_draw", env);
    script::bind_freefunc(server::get_team_players, "team_players", env);
    register_lua_function(&server::lua_team_players, "team_players");
    script::bind_freefunc(server::team_size, "teamsize", env);
    
    // Server-oriented functions and variables
    script::bind_freefunc(reload_hopmod, "reloadscripts", env);
    script::bind_freefunc(server::pausegame,"pausegame",env);
    script::bind_ro_var(server::gamepaused, "paused", env);
    script::bind_freefunc(server::sendservmsg, "msg", env);
    script::bind_freefunc(server::shutdown, "shutdown", env);
    script::bind_freefunc(restart_now, "restart_now", env);
    script::bind_freefunc(server::changetime, "changetime", env);
    script::bind_freefunc(server::changemap,"changemap", env, &changemap_defargs);
    script::bind_freefunc(server::addbot, "addbot", env);
    script::bind_freefunc(server::deletebot, "delbot", env);
    script::bind_freefunc(server::recorddemo, "recorddemo", env, &recorddemo_defargs);
    script::bind_freefunc(server::enddemorecord, "stopdemo", env);
    script::bind_freefunc(server::add_allowed_ip, "allow_ip", env);
    
    hopmod::bind_var(server::serverdesc, "servername", env);
    script::bind_ro_var(server::smapname, "map", env);
    hopmod::bind_var(server::serverpass, "server_password", env);
    //hopmod::bind_wo_var(server::adminpass, "admin_password", env);
    //script::bind_freefunc(server::compare_admin_password, "check_admin_password", env);
    script::bind_ro_var(server::currentmaster, "master", env);
    hopmod::bind_property<int>(server::get_minutes_left, server::set_minutes_left, "timeleft", env);
    hopmod::bind_property<int>(server::get_seconds_left, server::set_seconds_left, "seconds_left", env);
    hopmod::bind_var(server::interm, "intermission", env);
    script::bind_ro_var(totalmillis, "uptime", env);
    script::bind_ro_var(server::gamemillis, "gamemillis", env);
    script::bind_ro_var(server::gamelimit, "gamelimit", env);
    hopmod::bind_var(maxclients, "maxplayers", env);
    hopmod::bind_var(maxclients, "maxclients", env);
    hopmod::bind_var(serverip, "serverip", env);
    hopmod::bind_var(serverport, "serverport", env);
    hopmod::bind_var(server::next_gamemode, "next_mode", env);
    hopmod::bind_var(server::next_mapname, "next_map", env);
    hopmod::bind_var(server::next_gametime, "next_gametime", env);
    hopmod::bind_var(server::reassignteams, "reassignteams", env);
    hopmod::bind_funvar<int>(server::getplayercount, "playercount", env);
    hopmod::bind_funvar<int>(server::getspeccount, "speccount", env);
    hopmod::bind_funvar<int>(server::getbotcount, "botcount", env);
    hopmod::bind_var(server::aiman::botlimit, "botlimit", env);
    hopmod::bind_var(server::aiman::botbalance, "botbalance", env);
    hopmod::bind_funvar<const char *>(server::gamemodename, "gamemode", env);
    
    hopmod::bind_var(server::display_open, "display_open", env);
    hopmod::bind_var(server::allow_mm_veto, "allow_mastermode_veto", env);
    hopmod::bind_var(server::allow_mm_locked, "allow_mastermode_locked", env);
    hopmod::bind_var(server::allow_mm_private, "allow_mastermode_private", env);

    hopmod::bind_var(server::reservedslots, "reserved_slots", env);
    hopmod::bind_wo_var(server::slotpass, "reserved_slots_password", env);
    script::bind_ro_var(server::reservedslots_use, "reserved_slots_occupied", env);
    
    script::bind_ro_var(reloaded, "reloaded", env);
    
    script::bind_const((int)SHUTDOWN_NORMAL, "SHUTDOWN_NORMAL", env);
    script::bind_const((int)SHUTDOWN_RESTART, "SHUTDOWN_RESTART", env);
    script::bind_const((int)SHUTDOWN_RELOAD, "SHUTDOWN_RELOAD", env);
    
    hopmod::bind_property<int>(
        boost::bind(script::property<int>::generic_getter, boost::ref(server::mastermode)),
        server::script_set_mastermode, "mastermode", env);
    
    hopmod::bind_var(server::mastermode_owner, "mastermode_owner", env);
    script::bind_const((int)MM_OPEN, "MM_OPEN", env);
    script::bind_const((int)MM_VETO, "MM_VETO", env);
    script::bind_const((int)MM_LOCKED, "MM_LOCKED", env);
    script::bind_const((int)MM_PRIVATE, "MM_PRIVATE", env);
    script::bind_const((int)MM_PASSWORD, "MM_PASSWORD", env);
    
    hopmod::bind_var(server::sv_text_hit_length, "flood_protect_text", env);
    hopmod::bind_var(server::sv_sayteam_hit_length, "flood_protect_sayteam", env);
    hopmod::bind_var(server::sv_mapvote_hit_length, "flood_protect_mapvote", env);
    hopmod::bind_var(server::sv_switchname_hit_length, "flood_protect_switchname", env);
    hopmod::bind_var(server::sv_switchteam_hit_length, "flood_protect_switchteam", env);
    hopmod::bind_var(server::sv_kick_hit_length, "flood_protect_kick", env);
    hopmod::bind_var(server::sv_remip_hit_length, "flood_protect_remip", env);
    hopmod::bind_var(server::sv_newmap_hit_length, "flood_protect_newmap", env);
    hopmod::bind_var(server::sv_spec_hit_length, "flood_protect_spectator", env);
    
    script::bind_ro_var(tx_bytes, "tx_bytes", env);
    script::bind_ro_var(rx_bytes, "rx_bytes", env);
    script::bind_ro_var(tx_packets, "tx_packets", env);
    script::bind_ro_var(rx_packets, "rx_packets", env);
    
    hopmod::bind_var(server::timer_alarm_threshold, "timer_alarm_threshold", env);
    
    hopmod::bind_var(server::enable_extinfo, "enable_extinfo", env);
    
    static char cwd[1024];
    if(getcwd(cwd,sizeof(cwd)))
        script::bind_const((const char *)cwd, "PWD", env);
    
    script::bind_const(getuid(), "UID", env); //FIXME user id is not constant

    // Utility Functions
    
    script::bind_freefunc(unref, "unref", env);
    
    script::bind_freefunc(concol, "concol", env);
    script::bind_freefunc(green, "green", env);
    script::bind_freefunc(info, "info", env);
    script::bind_freefunc(err, "err", env);
    script::bind_freefunc(grey, "grey", env);
    script::bind_freefunc(magenta, "magenta", env);
    script::bind_freefunc(orange, "orange", env);
    script::bind_freefunc(gameplay, "gameplay", env);
    script::bind_freefunc(red, "red", env);
    script::bind_freefunc(blue, "blue", env);
    script::bind_freefunc(yellow, "yellow", env);
    
    script::bind_freefunc(mins, "mins", env);
    script::bind_freefunc(secs, "secs", env);
    
    script::bind_freefunc(parse_player_command_line, "parse_player_command", env);
    
    hopmod::bind_property<unsigned int>(
        boost::bind(script::property<unsigned int>::generic_getter, maintenance_frequency),
        set_maintenance_frequency, "maintenance_frequency", env);
    
    script::bind_freefunc(file_exists, "file_exists", env);
    script::bind_freefunc(dir_exists, "dir_exists", env);
    
    script::bind_var(server::spectator_delay, "spectator_delay", env);
    script::bind_freefunc(server::player_forgive_tk, "player_forgive_tk", env);
}

