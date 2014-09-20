#include "lua/event.hpp"

lua::event< std::tuple<> >                                         event_init("init");
lua::event< std::tuple<int, const char *, const char *, const char *, bool> >  event_connecting("connecting");
lua::event< std::tuple<int, bool> >                                event_connect("connect");
lua::event< std::tuple<int,const char *> >                         event_disconnect("disconnect");
lua::event< std::tuple<const char *,const char *> >                event_failedconnect("failedconnect");
lua::event< std::tuple<int> >                                      event_maploaded("maploaded");
lua::event< std::tuple<int,int> >                                  event_renaming("renaming");
lua::event< std::tuple<int,const char *> >                         event_allow_rename("allow_rename");
lua::event< std::tuple<int,const char *,const char *> >            event_rename("rename");
lua::event< std::tuple<int,const char *,const char *> >            event_reteam("reteam");
lua::event< std::tuple<int,const char *,const char *, int> >       event_chteamrequest("chteamrequest");
lua::event< std::tuple<int,const char *> >                         event_text("text");
lua::event< std::tuple<int,const char *> >                         event_sayteam("sayteam");
lua::event< std::tuple<int,const char *> >                         event_servcmd("servcmd");
lua::event< std::tuple<int,const char *,const char *> >            event_mapvote("mapvote");
lua::event< std::tuple<int, const char *,const char *> >           event_setmastermode("setmastermode");
lua::event< std::tuple<int, const char *, const char *> >          event_setmastermode_request("setmastermode_request");
lua::event< std::tuple<int,int> >                                  event_spectator("spectator");
lua::event< std::tuple<int,int> >                                  event_editmode("editmode");
lua::event< std::tuple<int> >                                      event_editpacket("editpacket");
lua::event< std::tuple<int,int,int> >                              event_privilege("privilege");
lua::event< std::tuple<int,int> >                                  event_teamkill("teamkill");
lua::event< std::tuple<int,const char *,const char *, int> >       event_authreq("request_auth_challenge");
lua::event< std::tuple<int,int,const char *> >                     event_authrep("auth_challenge_response");
lua::event< std::tuple<int,int,int> >                              event_addbot("addbot");
lua::event< std::tuple<int> >                                      event_delbot("delbot");
lua::event< std::tuple<int> >                                      event_botleft("botleft");
lua::event< std::tuple<int, const char *, int> >                   event_modmap("modmap");
lua::event< std::tuple<int,int> >                                  event_frag("frag");
lua::event< std::tuple<int,int,int> >                              event_shot("shot");
lua::event< std::tuple<int> >                                      event_suicide("suicide");
lua::event< std::tuple<int> >                                      event_spawn("spawn");
lua::event< std::tuple<int, int, int, int, double, double, double> >        event_damage("damage");
lua::event< std::tuple<int,const char*,bool> >                     event_setmaster("setmaster");
lua::event< std::tuple<int,int> >                                  event_respawnrequest("respawnrequest");
lua::event< std::tuple<int> >                                      event_clearbans_request("clearbans_request");
lua::event< std::tuple<int, const char *, int, int, const char *> >  event_kick_request("kick_request");
lua::event< std::tuple<> >                                         event_intermission("intermission");
lua::event< std::tuple<> >                                         event_finishedgame("finishedgame");
lua::event< std::tuple<int,int> >                                  event_timeupdate("timeupdate");
lua::event< std::tuple<const char *,const char *> >                event_mapchange("mapchange");
lua::event< std::tuple<> >                                         event_setnextgame("setnextgame");
lua::event< std::tuple<> >                                         event_gamepaused("gamepaused");
lua::event< std::tuple<> >                                         event_gameresumed("gameresumed");
lua::event< std::tuple<int,const char *> >                         event_beginrecord("beginrecord");
lua::event< std::tuple<int,int> >                                  event_endrecord("endrecord");
lua::event< std::tuple<const char *,const char *> >                event_votepassed("votepassed");
lua::event< std::tuple<int, const char *> >                        event_takeflag("takeflag");
lua::event< std::tuple<int, const char *> >                        event_dropflag("dropflag");
lua::event< std::tuple<int, const char *, int> >                   event_scoreflag("scoreflag");
lua::event< std::tuple<int, const char *> >                        event_returnflag("returnflag");
lua::event< std::tuple<const char *> >                             event_resetflag("resetflag");
lua::event< std::tuple<const char *, int> >                        event_scoreupdate("scoreupdate");
lua::event< std::tuple<> >                                         event_started("started");
lua::event< std::tuple<int> >                                      event_shutdown("shutdown");
lua::event< std::tuple<> >                                         event_shutdown_scripting("shutdown_scripting");
lua::event< std::tuple<> >                                         event_reloadhopmod("reloadhopmod");
lua::event< std::tuple<const char *> >                             event_varchanged("varchanged");
lua::event< std::tuple<> >                                         event_sleep("sleep");
lua::event< std::tuple<> >                                         event_interval("interval");
lua::event< std::tuple<int, int, int, const char *> >              event_cheat("cheat");

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
        & event_servcmd,
        & event_mapvote,
        & event_setmastermode,
        & event_setmastermode_request,
        & event_spectator,
        & event_editmode,
        & event_editpacket,
        & event_privilege,
        & event_teamkill,
        & event_authreq,
        & event_authrep,
        & event_addbot,
        & event_delbot,
        & event_botleft,
        & event_modmap,
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


