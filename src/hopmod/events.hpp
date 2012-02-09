#ifndef HOPMOD_EVENTS_HPP
#define HOPMOD_EVENTS_HPP

#include "lua/event.hpp"

extern lua::event< boost::tuple<> >                                         event_init;
extern lua::event< boost::tuple<int, const char *, const char *, const char *, bool> >  event_connecting;
extern lua::event< boost::tuple<int, bool> >                                event_connect;
extern lua::event< boost::tuple<int,const char *> >                         event_disconnect;
extern lua::event< boost::tuple<const char *,const char *> >                event_failedconnect;
extern lua::event< boost::tuple<int> >                                      event_maploaded;
extern lua::event< boost::tuple<int,int> >                                  event_renaming;
extern lua::event< boost::tuple<int,const char *> >                         event_allow_rename;
extern lua::event< boost::tuple<int,const char *,const char *> >            event_rename;
extern lua::event< boost::tuple<int,const char *,const char *> >            event_reteam;
extern lua::event< boost::tuple<int,const char *,const char *> >            event_chteamrequest;
extern lua::event< boost::tuple<int,const char *> >                         event_text;
extern lua::event< boost::tuple<int,const char *> >                         event_sayteam;
extern lua::event< boost::tuple<int,const char *,const char *> >            event_mapvote;
extern lua::event< boost::tuple<int, const char *,const char *> >           event_setmastermode;
extern lua::event< boost::tuple<int, const char *,const char *> >           event_setmastermode_request;
extern lua::event< boost::tuple<int,int> >                                  event_spectator;
extern lua::event< boost::tuple<int,int,int> >                              event_privilege;
extern lua::event< boost::tuple<int,int> >                                  event_teamkill;
extern lua::event< boost::tuple<int,const char *,const char *> >            event_authreq;
extern lua::event< boost::tuple<int,int,const char *> >                     event_authrep;
extern lua::event< boost::tuple<int,int,int> >                              event_addbot;
extern lua::event< boost::tuple<int> >                                      event_delbot;
extern lua::event< boost::tuple<int> >                                      event_botleft;
extern lua::event< boost::tuple<int, const char *, int> >                   event_modmap;
extern lua::event< boost::tuple<int,int> >                                  event_teamkill;
extern lua::event< boost::tuple<int,int> >                                  event_frag;
extern lua::event< boost::tuple<int,int,int> >                              event_shot;
extern lua::event< boost::tuple<int> >                                      event_suicide;
extern lua::event< boost::tuple<int> >                                      event_spawn;
extern lua::event< boost::tuple<int, int, int, int> >                       event_damage;
extern lua::event< boost::tuple<int,const char*,bool> >                     event_setmaster;
extern lua::event< boost::tuple<int,int> >                                  event_respawnrequest;
extern lua::event< boost::tuple<> >                                         event_clearbans_request;
extern lua::event< boost::tuple<int, const char *, int, int, const char *> >  event_kick_request;
extern lua::event< boost::tuple<> >                                         event_intermission;
extern lua::event< boost::tuple<> >                                         event_finishedgame;
extern lua::event< boost::tuple<int,int> >                                  event_timeupdate;
extern lua::event< boost::tuple<const char *,const char *> >                event_mapchange;
extern lua::event< boost::tuple<> >                                         event_setnextgame;
extern lua::event< boost::tuple<> >                                         event_gamepaused;
extern lua::event< boost::tuple<> >                                         event_gameresumed;
extern lua::event< boost::tuple<int,const char *> >                         event_beginrecord;
extern lua::event< boost::tuple<int,int> >                                  event_endrecord;
extern lua::event< boost::tuple<const char *,const char *> >                event_votepassed;
extern lua::event< boost::tuple<int, const char *> >                        event_takeflag;
extern lua::event< boost::tuple<int, const char *> >                        event_dropflag;
extern lua::event< boost::tuple<int, const char *, int, int> >              event_scoreflag;
extern lua::event< boost::tuple<int, const char *> >                        event_returnflag;
extern lua::event< boost::tuple<const char *> >                             event_resetflag;
extern lua::event< boost::tuple<const char *, int> >                        event_scoreupdate;
extern lua::event< boost::tuple<> >                                         event_started;
extern lua::event< boost::tuple<int> >                                      event_shutdown;
extern lua::event< boost::tuple<> >                                         event_shutdown_scripting;
extern lua::event< boost::tuple<> >                                         event_reloadhopmod;
extern lua::event< boost::tuple<const char *> >                             event_varchanged;
extern lua::event< boost::tuple<> >                                         event_sleep;
extern lua::event< boost::tuple<> >                                         event_interval;
extern lua::event< boost::tuple<int, int, int, const char *> >              event_cheat;

void register_event_idents(lua::event_environment &);

#endif

