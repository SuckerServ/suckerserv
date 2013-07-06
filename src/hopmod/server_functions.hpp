#ifndef HOPMOD_EXTAPI_HPP
#define HOPMOD_EXTAPI_HPP

#include "utils.hpp"

extern "C"{
#include <lua.h>
}

#include <string>
#include <vector>

namespace server
{
    struct clientinfo;
    
    namespace aiman
    {
        extern int botlimit;
        extern bool botbalance;
        extern bool deleteai();
    }
    
    extern string serverdesc;
    extern string smapname;
    extern string serverpass;
    extern string serverauth;
    extern string adminpass;
    extern string slotpass;
    extern int currentmaster;
    extern int interm;
    extern bool reassignteams;
    extern int gamemillis;
    extern int gamelimit;
    extern bool gamepaused;
    extern int gamemode;
    extern int intermtime;
    extern stream *mapdata;
    
    extern int mastermode;
    extern int mastermode_owner;
    extern string next_gamemode;
    extern string next_mapname;
    extern int next_gametime;
    
    extern int reservedslots;
    extern int reservedslots_use;
    
    extern bool display_open;
    extern bool allow_mm_veto;
    extern bool allow_mm_locked;
    extern bool allow_mm_private;
    extern bool allow_mm_private_reconnect;
    extern bool reset_mm;
    extern bool allow_item[11];

    extern bool broadcast_mapmodified;
    extern timer::time_diff_t timer_alarm_threshold;
    
    extern bool enable_extinfo;
    
    extern int spectator_delay;
    
    namespace message{
        
        namespace resend_time{
            
            extern unsigned text;
            extern unsigned sayteam;
            extern unsigned mapvote;
            extern unsigned switchname;
            extern unsigned switchteam;
            extern unsigned kick;
            extern unsigned remip;
            extern unsigned newmap;
            extern unsigned spec;
            
        } //namespace resend_time
        
        bool limit(clientinfo *, unsigned long long * millis, unsigned long long resend_time, const char * message_type);
        
        extern string set_player_privilege;
        
    } //namespace message
    
    const char *extfiltertext(const char *src);
    
    void started();
    int player_sessionid(int);
    int player_id(lua_State * L);
    int player_ownernum(int);
    void player_msg(int,const char *);
    void server_msg(const char *);
    std::string player_name(int);
    void player_rename(int, const char *, bool);
    std::string player_displayname(int);
    std::string player_team(int);
    const char * player_privilege(int);
    int player_privilege_code(int);
    int player_ping(int);
    int player_ping_update(int);
    int player_lag(int);
    int player_real_lag(int);
    int player_maploaded(int);
    int player_deathmillis(int);
    const char * player_ip(int);
    unsigned long player_iplong(int);
    const char * player_status(int);
    int player_status_code(int);
    int player_frags(int);
    int player_score(int);
    int player_real_frags(int);
    int player_deaths(int);
    int player_suicides(int);
    int player_teamkills(int);
    int player_damage(int);
    int player_damagewasted(int);
    int player_maxhealth(int);
    int player_health(int);
    int player_armour(int);
    int player_armour_type(int);
    int player_gun(int);
    int player_hits(int);
    int player_misses(int);
    int player_shots(int);
    int player_accuracy(int);
    int player_accuracy2(int);
    bool player_is_spy(int cn);
    int player_clientmillis(int);
    int player_timetrial(int);
    int player_connection_time(int);
    bool player_has_joined_game(int);
    void player_join_game(int);
    void player_reject_join_game(int);
    int player_timeplayed(int);
    int player_win(int);
    void player_force_spec(int);
    void player_unforce_spec(int);
    void player_spec(int);
    void player_unspec(int);
    void spec_all();
    int player_bots(int);
    int player_pos(lua_State *);
    bool hasmaster();
    void unsetmaster();
    bool set_player_master(int);
    void set_player_auth(int);
    void set_player_admin(int);
    void player_forgive_tk(int);
    void player_slay(int);
    void player_respawn(int);
    void player_nospawn(int, int);
    bool player_changeteam(int,const char *);
    int player_rank(int);
    bool player_isbot(int);
    void set_player_private_admin(int);
    void set_player_private_master(int);
    void unset_player_privilege(int);
    void set_player_privilege(int, int);
    void player_freeze(int);
    void player_unfreeze(int);

    void updateservinfo(int, const char*);
    void editvar(int, const char *, int);
    void sendmap(int, int);
    int hitpush(lua_State * L);
    void set_spy(int, bool);
    
    void team_msg(const char *,const char *);
    std::vector<std::string> get_teams();
    int lua_team_list(lua_State * L);
    int get_team_score(const char * );
    std::vector<int> get_team_players(const char * team);
    int lua_team_players(lua_State *);
    int team_win(const char *);
    int team_draw(const char *);
    int team_size(const char *);
    
    void pausegame(bool);
    void pausegame(bool, clientinfo *);
    
    void kick(int cn,int time,const std::string & admin,const std::string & reason);
    void disconnect(int cn, int code, std::string reason);
    void changetime(int remaining);
    int get_minutes_left();
    void set_minutes_left(int);
    int get_seconds_left();
    void set_seconds_left(int);
    void changemap(const char * map,const char * mode,int mins);
    int modecode(const char * modename);
    int getplayercount();
    int getbotcount();
    int getspeccount();
    void addbot(int);
    void deletebot(int);
    void update_mastermask();
    const char * gamemodename();
    int lua_gamemodeinfo(lua_State *);
    void recorddemo(const char *);
    void enddemorecord();
    void calc_player_ranks();
    void set_mastermode_cn(int, int);
    void set_mastermode(int);
    int get_mastermode();
    void add_allowed_ip(const char *);
    bool compare_admin_password(const char *);
    
    std::vector<int> cs_player_list();
    std::vector<int> cs_spec_list();
    std::vector<int> cs_bot_list();
    std::vector<int> cs_client_list();
    int lua_player_list(lua_State *);
    int lua_spec_list(lua_State *);
    int lua_bot_list(lua_State *);
    int lua_client_list(lua_State *);

    bool rotatemap();
    
    void suicide(int);
    void player_servcmd(int cn, const char *string);
    
    extern string ext_admin_pass;
    
    void sendservmsg(const char *);
    
    void send_auth_challenge(int,int,const char *,const char *);
    void send_auth_request(int, const char *);
    
    bool send_item(int item_code, int recipient);
    
    void try_respawn(clientinfo * ci, clientinfo * cq);
    
} //namespace server

#endif
