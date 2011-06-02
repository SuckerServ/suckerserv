#ifndef HOPMOD_SIGNALS_HPP
#define HOPMOD_SIGNALS_HPP

namespace fungu{namespace script{class env;}}
#include <boost/signal.hpp>
#include <limits>

/**
    @brief Event proceed combiner
    
    If any slot function returns -1 it will tell the signaller to veto/cancel the event.
*/
struct proceed
{
    typedef int result_type;
    static const int true_value = 0;
    template<typename InputIterator>
    int operator()(InputIterator first, InputIterator last)const
    {
        int cancel = 0;
        for(InputIterator it = first; it != last; ++it)
            if(*it == -1) cancel = -1;
        return cancel;
    }
};

struct maxvalue
{
    typedef int result_type;
    template<typename InputIterator>
    int operator()(InputIterator first, InputIterator last)const
    {
        if(first == last) return -1;
        int greatest = -1;
        for(InputIterator it = first; it != last; ++it) 
            greatest = std::max(greatest, *it);
        return greatest;
    }
};

#ifdef HOPMOD_GAMESERVER_EVENTS
// Player Events
extern boost::signal<int (int, const char *, const char *, const char *, bool), proceed>     signal_connecting;
extern boost::signal<void (int)>                                    signal_connect;
extern boost::signal<void (int,const char *)>                       signal_disconnect;
extern boost::signal<void (const char *,const char *)>              signal_failedconnect;
extern boost::signal<void (int)>                                    signal_maploaded;
extern boost::signal<void (int,int)>                                signal_renaming;
extern boost::signal<int (int,const char *),proceed>                signal_allow_rename;
extern boost::signal<void (int,const char *,const char *)>          signal_rename;
extern boost::signal<void (int,const char *,const char *)>          signal_reteam;
extern boost::signal<int (int,const char *,const char *),proceed>   signal_chteamrequest;
extern boost::signal<int (int,const char *), proceed>               signal_text;
extern boost::signal<int (int,const char *), proceed>               signal_sayteam;
extern boost::signal<int (int,const char *,const char *), proceed>  signal_mapvote;
extern boost::signal<int (int, const char *,const char *), proceed> signal_setmastermode_request;
extern boost::signal<int (int, const char *,const char *)>          signal_setmastermode;
extern boost::signal<void (int,int)>                                signal_spectator;
extern boost::signal<void (int,int,int)>                            signal_privilege;
extern boost::signal<void (int,int)>                                signal_teamkill;
extern boost::signal<void (int,const char *,const char *)>          signal_authreq;
extern boost::signal<void (int,int,const char *)>                   signal_authrep;
extern boost::signal<void (int,int,int)>                            signal_addbot;
extern boost::signal<void (int)>                                    signal_delbot;
extern boost::signal<void (int)>                                    signal_botleft;
extern boost::signal<void (int)>                                    signal_mapcrcfail;
extern boost::signal<void (int, const char *, int)>                 signal_mapcrc;
extern boost::signal<void (int)>                                    signal_checkmaps;
extern boost::signal<void (int,int)>                                signal_teamkill;
extern boost::signal<void (int,int)>                                signal_frag;
extern boost::signal<void (int,int,int)>                            signal_shot;
extern boost::signal<void (int)>                                    signal_suicide;
extern boost::signal<void (int)>                                    signal_spawn;
extern boost::signal<int (int, int, int, int), proceed>             signal_damage;
extern boost::signal<int (int,const char*,bool), proceed>           signal_setmaster;
extern boost::signal<int (int,int), maxvalue>                       signal_respawnrequest;

// Command Requests
extern boost::signal<void ()>                                           signal_clearbans_request;
extern boost::signal<void (int, const char *, int, int, const char *)>  signal_kick_request;

// Game Events
extern boost::signal<void ()>                                       signal_intermission;
extern boost::signal<void ()>                                       signal_finishedgame;
extern boost::signal<void (int,int)>                                signal_timeupdate;
extern boost::signal<void (const char *,const char *)>              signal_mapchange;
extern boost::signal<void ()>                                       signal_setnextgame;
extern boost::signal<void ()>                                       signal_gamepaused;
extern boost::signal<void ()>                                       signal_gameresumed;
extern boost::signal<void (int,const char *)>                       signal_beginrecord;
extern boost::signal<void (int,int)>                                signal_endrecord;
extern boost::signal<void (const char *,const char *)>              signal_votepassed;
extern boost::signal<void (int, const char *)>                      signal_takeflag;
extern boost::signal<void (int, const char *)>                      signal_dropflag;
extern boost::signal<void (int, const char *)>                      signal_scoreflag;
extern boost::signal<void (int, const char *)>                      signal_returnflag;
extern boost::signal<void (const char *)>                           signal_resetflag;
extern boost::signal<void (const char *, int)>                      signal_scoreupdate;

#endif

// Generic Server Events
extern boost::signal<void ()> signal_started;
extern boost::signal<void (int)> signal_shutdown;
extern boost::signal<void ()> signal_shutdown_scripting;
extern boost::signal<void ()> signal_reloadhopmod;
extern boost::signal<void ()> signal_maintenance;
extern boost::signal<void (const char *)> signal_varchanged;

/**
    @brief Register signals with the global script::slot_factory instance.
*/
void register_signals(fungu::script::env &);

/**
    @brief Deallocates destroyed slots - should be called regularly on the main loop.
*/
void cleanup_dead_slots();

void disconnect_all_slots();

#endif
