#include "lua/event.hpp"

lua::event< boost::tuple<> >                                         event_init("init");
lua::event< boost::tuple<int, const char *, const char *, const char *, bool> >  event_connecting("connecting");
lua::event< boost::tuple<int, bool> >                                event_connect("connect");
lua::event< boost::tuple<int,const char *> >                         event_disconnect("disconnect");
lua::event< boost::tuple<const char *,const char *> >                event_failedconnect("failedconnect");
lua::event< boost::tuple<int> >                                      event_maploaded("maploaded");
lua::event< boost::tuple<int,int> >                                  event_renaming("renaming");
lua::event< boost::tuple<int,const char *> >                         event_allow_rename("allow_rename");
lua::event< boost::tuple<int,const char *,const char *> >            event_rename("rename");
lua::event< boost::tuple<int,const char *,const char *> >            event_reteam("reteam");
lua::event< boost::tuple<int,const char *,const char *> >            event_chteamrequest("chteamrequest");
lua::event< boost::tuple<int,const char *> >                         event_text("text");
lua::event< boost::tuple<int,const char *> >                         event_sayteam("sayteam");
lua::event< boost::tuple<int,const char *,const char *> >            event_mapvote("mapvote");
lua::event< boost::tuple<int, const char *,const char *> >           event_setmastermode("setmastermode");
lua::event< boost::tuple<int, const char *, const char *> >          event_setmastermode_request("setmastermode_request");
lua::event< boost::tuple<int,int> >                                  event_spectator("spectator");
lua::event< boost::tuple<int,int,int> >                              event_privilege("privilege");
lua::event< boost::tuple<int,int> >                                  event_teamkill("teamkill");
lua::event< boost::tuple<int,const char *,const char *> >            event_authreq("request_auth_challenge");
lua::event< boost::tuple<int,int,const char *> >                     event_authrep("auth_challenge_response");
lua::event< boost::tuple<int,int,int> >                              event_addbot("addbot");
lua::event< boost::tuple<int> >                                      event_delbot("delbot");
lua::event< boost::tuple<int> >                                      event_botleft("botleft");
lua::event< boost::tuple<int> >                                      event_mapcrcfail("mapcrcfail");
lua::event< boost::tuple<int, const char *, int> >                   event_mapcrc("mapcrc");
lua::event< boost::tuple<int> >                                      event_checkmaps("checkmaps");
lua::event< boost::tuple<int,int> >                                  event_frag("frag");
lua::event< boost::tuple<int,int,int> >                              event_shot("shot");
lua::event< boost::tuple<int> >                                      event_suicide("suicide");
lua::event< boost::tuple<int> >                                      event_spawn("spawn");
lua::event< boost::tuple<int, int, int, int> >                       event_damage("damage");
lua::event< boost::tuple<int,const char*,bool> >                     event_setmaster("setmaster");
lua::event< boost::tuple<int,int> >                                  event_respawnrequest("respawnrequest");
lua::event< boost::tuple<> >                                         event_clearbans_request("clearbans_request");
lua::event< boost::tuple<int, const char *, int, int, const char *> >  event_kick_request("kick_request");
lua::event< boost::tuple<> >                                         event_intermission("intermission");
lua::event< boost::tuple<> >                                         event_finishedgame("finishedgame");
lua::event< boost::tuple<int,int> >                                  event_timeupdate("timeupdate");
lua::event< boost::tuple<const char *,const char *> >                event_mapchange("mapchange");
lua::event< boost::tuple<> >                                         event_setnextgame("setnextgame");
lua::event< boost::tuple<> >                                         event_gamepaused("gamepaused");
lua::event< boost::tuple<> >                                         event_gameresumed("gameresumed");
lua::event< boost::tuple<int,const char *> >                         event_beginrecord("beginrecord");
lua::event< boost::tuple<int,int> >                                  event_endrecord("endrecord");
lua::event< boost::tuple<const char *,const char *> >                event_votepassed("votepassed");
lua::event< boost::tuple<int, const char *> >                        event_takeflag("takeflag");
lua::event< boost::tuple<int, const char *> >                        event_dropflag("dropflag");
lua::event< boost::tuple<int, const char *, int, int> >              event_scoreflag("scoreflag");
lua::event< boost::tuple<int, const char *> >                        event_returnflag("returnflag");
lua::event< boost::tuple<const char *> >                             event_resetflag("resetflag");
lua::event< boost::tuple<const char *, int> >                        event_scoreupdate("scoreupdate");
lua::event< boost::tuple<> >                                         event_started("started");
lua::event< boost::tuple<int> >                                      event_shutdown("shutdown");
lua::event< boost::tuple<> >                                         event_shutdown_scripting("shutdown_scripting");
lua::event< boost::tuple<> >                                         event_reloadhopmod("reloadhopmod");
lua::event< boost::tuple<const char *> >                             event_varchanged("varchanged");
lua::event< boost::tuple<> >                                         event_sleep("sleep");
lua::event< boost::tuple<> >                                         event_interval("interval");
lua::event< boost::tuple<int, int, int, const char *> >              event_cheat("cheat");

void register_event_idents(lua::event_environment & env)
{
    lua::event_base * events[] = {
        & event_init,
        & event_connecting,
        & event_connect,
        & event_disconnect,
        & event_failedconnect,
        & event_maploaded,
        & event_renaming,
        & event_allow_rename,
        & event_rename,
        & event_reteam,
        & event_chteamrequest,
        & event_text,
        & event_sayteam,
        & event_mapvote,
        & event_setmastermode,
        & event_setmastermode_request,
        & event_spectator,
        & event_privilege,
        & event_teamkill,
        & event_authreq,
        & event_authrep,
        & event_addbot,
        & event_delbot,
        & event_botleft,
        & event_mapcrcfail,
        & event_mapcrc,
        & event_checkmaps,
        & event_frag,
        & event_shot,
        & event_suicide,
        & event_spawn,
        & event_damage,
        & event_setmaster,
        & event_respawnrequest,
        & event_clearbans_request,
        & event_kick_request,
        & event_intermission,
        & event_finishedgame,
        & event_timeupdate,
        & event_mapchange,
        & event_setnextgame,
        & event_gamepaused,
        & event_gameresumed,
        & event_beginrecord,
        & event_endrecord,
        & event_votepassed,
        & event_takeflag,
        & event_dropflag,
        & event_scoreflag,
        & event_returnflag,
        & event_resetflag,
        & event_scoreupdate,
        & event_started,
        & event_shutdown,
        & event_shutdown_scripting,
        & event_reloadhopmod,
        & event_varchanged,
        & event_sleep,
        & event_interval,
        & event_cheat,
        NULL
    };
    
    env.register_event_idents(events);
}


