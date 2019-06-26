#ifdef BOOST_BUILD_PCH_ENABLED
#include "hopmod/pch.hpp"
#endif

#include "hopmod/hopmod.hpp"
#include "game.h"

#include "hopmod/extapi.hpp"
#include "hopmod/hopmod.hpp"
#include "hopmod/utils.hpp"

#include <fungu/script/error.hpp>
#include <fungu/script/lexical_cast.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
boost::asio::io_service & get_main_io_service();

#include <iostream>
#include <set>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

extern "C"{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace game
{         
    void parseoptions(vector<const char *> &args)
    {   
        loopv(args)
#ifndef STANDALONE
            if(!game::clientoption(args[i]))
#endif
            if(!server::serveroption(args[i]))
                printf("unknown command-line option: %s", args[i]);
    }
}

extern ENetAddress masteraddress;

namespace server
{
    struct server_entity            // server side version of "entity" type
    {
        int type;
        int spawntime;
        char spawned;
    };

    static const int DEATHMILLIS = 300;
    
    int spectator_delay = 0;
    struct delayed_sendpacket
    {
        int     time;
        int     channel;
        void *  data;
        int     length;
    };
    vector<delayed_sendpacket> delayed_sendpackets;
    
    struct clientinfo;
    
    struct gameevent
    {
        virtual ~gameevent() {}

        virtual bool flush(clientinfo *ci, int fmillis);
        virtual void process(clientinfo *ci) {}

        virtual bool keepable() const { return false; }
    };

    struct timedevent : gameevent
    {
        int millis;

        bool flush(clientinfo *ci, int fmillis);
    };

    struct hitinfo
    {
        int target;
        int lifesequence;
        int rays;
        float dist;
        vec dir;
    };

    struct shotevent : timedevent
    {
        int id, gun;
        vec from, to;
        vector<hitinfo> hits;

        void process(clientinfo *ci);
    };

    struct explodeevent : timedevent
    {
        int id, gun;
        vector<hitinfo> hits;

        bool keepable() const { return true; }

        void process(clientinfo *ci);
    };

    struct suicideevent : gameevent
    {
        void process(clientinfo *ci);
    };

    struct pickupevent : gameevent
    {
        int ent;

        void process(clientinfo *ci);
    };

    template <int N>
    struct projectilestate
    {
        int projs[N];
        int numprojs;

        projectilestate() : numprojs(0) {}

        void reset() { numprojs = 0; }

        void add(int val)
        {
            if(numprojs>=N) numprojs = 0;
            projs[numprojs++] = val;
        }

        bool remove(int val)
        {
            loopi(numprojs) if(projs[i]==val)
            {
                projs[i] = projs[--numprojs];
                return true;
            }
            return false;
        }
    };

    struct gamestate : fpsstate
    {
        vec o;
        int state, editstate;
        int lastdeath, lastspawn, lifesequence;
        int lastshot;
        projectilestate<8> rockets, grenades;
        int frags, flags, deaths, suicides, shotdamage, damage, explosivedamage, teamkills, hits, misses, shots;
        int lasttimeplayed, timeplayed;
        float effectiveness;
        int disconnecttime;
        
        gamestate() : state(CS_DEAD), editstate(CS_DEAD), lifesequence(0), disconnecttime(0) {}
    
        bool isalive(int gamemillis)
        {
            return state==CS_ALIVE || (state==CS_DEAD && gamemillis - lastdeath <= DEATHMILLIS);
        }

        bool waitexpired(int gamemillis)
        {
            return gamemillis - lastshot >= gunwait;
        }

        void reset()
        {
            if(state!=CS_SPECTATOR) state = editstate = CS_DEAD;
            maxhealth = 100;
            rockets.reset();
            grenades.reset();

            timeplayed = 0;
            effectiveness = 0;
            frags = flags = deaths = suicides = teamkills = shotdamage = explosivedamage = damage = hits = misses = shots = 0;

            respawn();
        }

        void respawn()
        {
            fpsstate::respawn();
            o = vec(-1e10f, -1e10f, -1e10f);
            lastdeath = 0;
            lastspawn = -1;
            lastshot = 0;
        }

        void reassign()
        {
            respawn();
            rockets.reset();
            grenades.reset();
        }
    };

    struct savedscore
    {
        uint ip;
        string name;
        int maxhealth, frags, flags, deaths, suicides, teamkills, shotdamage, explosivedamage, damage, hits, misses, shots;
        int timeplayed;
        float effectiveness;
        int state;
        int disconnecttime;
        
        void save(gamestate &gs)
        {
            maxhealth = gs.maxhealth;
            frags = gs.frags;
            flags = gs.flags;
            deaths = gs.deaths;
            suicides = gs.suicides;
            teamkills = gs.teamkills;
            shotdamage = gs.shotdamage;
            explosivedamage = gs.explosivedamage;
            damage = gs.damage;
            timeplayed = gs.timeplayed;
            effectiveness = gs.effectiveness;
            hits = gs.hits;
            misses = gs.misses;
            shots = gs.shots;
            state = gs.state;
            disconnecttime = gs.disconnecttime;
        }

        void restore(gamestate &gs)
        {
            if(gs.health==gs.maxhealth) gs.health = maxhealth;
            gs.maxhealth = maxhealth;
            gs.frags = frags;
            gs.flags = flags;
            gs.deaths = deaths;
            gs.suicides = suicides;
            gs.teamkills = teamkills;
            gs.shotdamage = shotdamage;
            gs.explosivedamage = explosivedamage;
            gs.damage = damage;
            gs.timeplayed = timeplayed;
            gs.effectiveness = effectiveness;
            gs.hits = hits;
            gs.misses = misses;
            gs.shots = shots;
            gs.state = state;
            gs.disconnecttime = disconnecttime;
        }
    };
    
    extern int gamemillis, nextexceeded;
    
    struct clientinfo
    {
        int clientnum, ownernum, connectmillis, specmillis, sessionid, overflow, playerid;
        string name, team, mapvote;
        int playermodel;
        int modevote;
        int privilege;
        bool connected, local, timesync;
        int gameoffset, lastevent, pushed, exceeded;
        gamestate state;
        vector<gameevent *> events;
        vector<uchar> position, messages;
        int posoff, poslen, msgoff, msglen;
        vector<clientinfo *> bots;
        int ping, lastpingupdate, lastposupdate, lag, aireinit;
        string clientmap;
        int mapcrc;
        int no_spawn;
        bool warned, gameclip;
        ENetPacket *getdemo, *getmap, *clipboard;
        int lastclipboard, needclipboard;
        
        freqlimit sv_text_hit;
        freqlimit sv_sayteam_hit;
        freqlimit sv_mapvote_hit;
        freqlimit sv_switchname_hit;
        freqlimit sv_switchteam_hit;
        freqlimit sv_kick_hit;
        freqlimit sv_remip_hit;
        freqlimit sv_newmap_hit;
        freqlimit sv_spec_hit;
        std::string disconnect_reason;
        bool maploaded;
        int rank;
        bool using_reservedslot;
        bool allow_self_unspec;
        
        clientinfo() 
         : 
           getdemo(NULL),
           getmap(NULL),
           clipboard(NULL),
           sv_text_hit(sv_text_hit_length),
           sv_sayteam_hit(sv_sayteam_hit_length),
           sv_mapvote_hit(sv_mapvote_hit_length),
           sv_switchname_hit(sv_switchname_hit_length),
           sv_switchteam_hit(sv_switchteam_hit_length),
           sv_kick_hit(sv_kick_hit_length),
           sv_remip_hit(sv_remip_hit_length),
           sv_newmap_hit(sv_newmap_hit_length),
           sv_spec_hit(sv_spec_hit_length)
        { reset(); }
        
        ~clientinfo() { events.deletecontents(); cleanclipboard(); }
        
        void addevent(gameevent *e)
        {
            if(state.state==CS_SPECTATOR || events.length()>100) delete e;
            else events.add(e);
        }

        enum
        {
            PUSHMILLIS = 2500
        };
        
        int calcpushrange()
        {
            ENetPeer *peer = getclientpeer(ownernum);
            return PUSHMILLIS + (peer ? peer->roundTripTime + peer->roundTripTimeVariance : ENET_PEER_DEFAULT_ROUND_TRIP_TIME);
        }

        bool checkpushed(int millis, int range)
        {
            return millis >= pushed - range && millis <= pushed + range;
        }

        void scheduleexceeded()
        {
            if(state.state!=CS_ALIVE || !exceeded) return;
            int range = calcpushrange();
            if(!nextexceeded || exceeded + range < nextexceeded) nextexceeded = exceeded + range;
        }

        void setexceeded()
        {
            if(state.state==CS_ALIVE && !exceeded && !checkpushed(gamemillis, calcpushrange())) exceeded = gamemillis;
            scheduleexceeded(); 
        }
            
        void setpushed()
        {
            pushed = max(pushed, gamemillis);
            if(exceeded && checkpushed(exceeded, calcpushrange())) exceeded = 0;
        }
        
        bool checkexceeded()
        {
            return state.state==CS_ALIVE && exceeded && gamemillis > exceeded + calcpushrange();
        }
        
        void mapchange()
        {
            mapvote[0] = 0;
            state.reset();
            events.deletecontents();
            overflow = 0;
            timesync = false;
            lastevent = 0;
            exceeded = 0;
            pushed = 0;
            maploaded = false;
            clientmap[0] = '\0';
            mapcrc = 0;
            warned = false;
            gameclip = false;
            rank = 0;
            specmillis = -1;
        }

        void reassign()
        {
            state.reassign();
            events.deletecontents();
            timesync = false;
            lastevent = 0;
        }
        
        void cleanclipboard(bool fullclean = true)
        {
            if(clipboard) { if(--clipboard->referenceCount <= 0) enet_packet_destroy(clipboard); clipboard = NULL; }
            if(fullclean) lastclipboard = 0;
        }
        
        void reset()
        {
            name[0] = team[0] = 0;
            playermodel = -1;
            privilege = PRIV_NONE;
            connected = local = false;
            position.setsize(0);
            messages.setsize(0);
            ping = 0;
            lastpingupdate = 0;
            lastposupdate = 0;
            lag = 0;
            aireinit = 0;
            using_reservedslot = false;
            needclipboard = 0;
            cleanclipboard();
            mapchange();
        }
        
        int geteventmillis(int servmillis, int clientmillis)
        {
            if(!timesync || (events.empty() && state.waitexpired(servmillis)))
            {
                timesync = true;
                gameoffset = servmillis - clientmillis;
                return servmillis;
            }
            else return gameoffset + clientmillis;
        }

        void sendprivtext(const char * text)
        {
            if(state.aitype != AI_NONE) return;//TODO assert(false) to catch bugs
            sendf(clientnum, 1, "ris", N_SERVMSG, text);
        }
        
        bool check_flooding(freqlimit & hit, const char * activity = NULL, bool sendwarning = true)
        {
            int remaining = hit.next(totalmillis);
            bool flooding = remaining > 0;
            if(flooding && activity && sendwarning)
            {
                defformatstring(blockedinfo)(RED "[Flood Protection] You are blocked from %s for another %i seconds.", activity, static_cast<int>(std::ceil(remaining/1000.0)));
                sendprivtext(blockedinfo);
            }
            return flooding;
        }
        
        const char * hostname()const
        {
            static char hostname_buffer[16];            
            ENetAddress addr;
            addr.host = getclientip(clientnum);
            return ((enet_address_get_host_ip(&addr, hostname_buffer, sizeof(hostname_buffer)) == 0) ? hostname_buffer : "unknown");
        }
        
        bool is_delayed_spectator()const
        {
            return spectator_delay && state.state == CS_SPECTATOR && privilege != PRIV_ADMIN;
        }
    };
    
    struct worldstate
    {
        int uses;
        vector<uchar> positions, messages;
    };

    struct ban
    {
        int time;
        uint ip;
    };

    namespace aiman 
    {
        extern bool dorefresh, botbalance;
        extern int botlimit;
        extern void removeai(clientinfo *ci);
        extern void clearai();
        extern void checkai();
        extern void reqadd(clientinfo *ci, int skill);
        extern void reqdel(clientinfo *ci);
        extern void setbotlimit(clientinfo *ci, int limit);
        extern void setbotbalance(clientinfo *ci, bool balance);
        extern void changemap();
        extern void addclient(clientinfo *ci);
        extern void changeteam(clientinfo *ci);
        extern clientinfo * addai(int skill, int limit);
        extern bool deleteai();
        extern void deleteai(clientinfo *);
    }

    #define MM_MODE 0xF
    #define MM_AUTOAPPROVE 0x1000
    #define MM_PRIVSERV (MM_MODE | MM_AUTOAPPROVE)
    #define MM_PUBSERV ((1<<MM_OPEN) | (1<<MM_VETO))
    #define MM_COOPSERV (MM_AUTOAPPROVE | MM_PUBSERV | (1<<MM_LOCKED))
    
    bool notgotitems = true;        // true when map has changed and waiting for clients to send item
    int gamemode = 0;
    int gamemillis = 0, gamelimit = 0, nextexceeded = 0, next_timeupdate = 0;
    bool gamepaused = false;
    int pausegame_owner = -1;
    bool reassignteams = true;
    
    bool display_open = false;
    bool allow_mm_veto = false;
    bool allow_mm_locked = false;
    bool allow_mm_private = false;
    bool allow_item[11] = {true, true, true, true, true, true, true, true, true, true, true};
    
    string next_gamemode = "";
    string next_mapname = "";
    int next_gametime = -1;

    int reservedslots = 0;
    int reservedslots_use = 0;
    
    int intermtime = 10000;

    string serverdesc = "", serverpass = "";
    string smapname = "";
    int interm = 0;
    bool mapreload = false;
    enet_uint32 lastsend = 0;
    int mastermode = MM_OPEN, mastermask = MM_PRIVSERV, mastermode_owner = -1, mastermode_mtime = 0;
    int currentmaster = -1;
    bool masterupdate = false;
    string adminpass = "";
    string slotpass = "";
    stream *mapdata = NULL;
    
    vector<uint> allowedips;
    
    vector<clientinfo *> connects, clients, bots;
    vector<worldstate *> worldstates;
    
    bool reliablemessages = false;

    bool broadcast_mapmodified = true;
    
    bool enable_extinfo = true;
    
    struct demofile
    {
        string info;
        uchar *data;
        int len;
    };

    #define MAXDEMOS 5
    vector<demofile> demos;

    bool demonextmatch = false;
    stream *demotmp = NULL, *demorecord = NULL, *demoplayback = NULL;
    int demo_id = 0;
    int nextplayback = 0, demomillis, demoffset = 0;
    
    timer::time_diff_t timer_alarm_threshold = 1000000;
    
    void *newclientinfo() { return new clientinfo; }
    void deleteclientinfo(void *ci) { delete (clientinfo *)ci; } 
    
    clientinfo *getinfo(int n)
    {
        if(n < MAXCLIENTS) return (clientinfo *)getclientinfo(n);
        n -= MAXCLIENTS;
        return bots.inrange(n) ? bots[n] : NULL;
    }
    
    clientinfo * get_ci(int cn)
    {
        clientinfo * ci = getinfo(cn);
        if(!ci) throw fungu::script::error(fungu::script::OPERATION_ERROR,boost::make_tuple(std::string("invalid cn")));
        return ci;
    }
    
    struct servmode
    {
        virtual ~servmode() {}

        virtual void entergame(clientinfo *ci) {}
        virtual void leavegame(clientinfo *ci, bool disconnecting = false) {}

        virtual void moved(clientinfo *ci, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip) {}
        virtual bool canspawn(clientinfo *ci, bool connecting = false) { return true; }
        virtual void spawned(clientinfo *ci) {}
        virtual int fragvalue(clientinfo *victim, clientinfo *actor)
        {
            if(victim==actor || isteam(victim->team, actor->team)) return -1;
            return 1;
        }
        virtual void died(clientinfo *victim, clientinfo *actor) {}
        virtual bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam) { return true; }
        virtual void changeteam(clientinfo *ci, const char *oldteam, const char *newteam) {}
        virtual void initclient(clientinfo *ci, packetbuf &p, bool connecting) {}
        virtual void update() {}
        virtual void reset(bool empty) {}
        virtual void intermission() {}
        virtual bool hidefrags() { return false; }
        virtual int getteamscore(const char *team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual bool extinfoteam(const char *team, ucharbuf &p) { return false; }
        virtual bool setteamscore(const char *, int){ return false; }
    };

    #define SERVMODE 1
    #include "capture.h"
    #include "ctf.h"

    captureservmode capturemode;
    ctfservmode ctfmode;
    servmode *smode = NULL;
    
    bool canspawnitem(int type) { return !m_noitems && (type>=I_SHELLS && type<=I_QUAD && (!m_noammo || type<I_SHELLS || type>I_CARTRIDGES)); }
    
    int numclients(int exclude, bool nospec, bool noai);
    
    int spawntime(int type)
    {
        if(m_classicsp) return INT_MAX;
        int np = numclients(-1, true, false);
        np = np<3 ? 4 : (np>4 ? 2 : 3);         // spawn times are dependent on number of players
        int sec = 0;
        switch(type)
        {
            case I_SHELLS:
            case I_BULLETS:
            case I_ROCKETS:
            case I_ROUNDS:
            case I_GRENADES:
            case I_CARTRIDGES: sec = np*4; break;
            case I_HEALTH: sec = np*5; break;
            case I_GREENARMOUR:
            case I_YELLOWARMOUR: sec = 20; break;
            case I_BOOST:
            case I_QUAD: sec = 40+rnd(40); break;
        }
        return sec*1000;
    }
    
    bool delayspawn(int type)
    {
        switch(type)
        {
            case I_GREENARMOUR:
            case I_YELLOWARMOUR:
                return !m_classicsp;
            case I_BOOST:
            case I_QUAD:
                return true;
            default:
                return false;
        }
    }
    
    vector<server_entity> sents;
    int sents_type_index[MAXENTTYPES];
    vector<savedscore> scores;

    int msgsizelookup(int msg)
    {
        static int sizetable[NUMSV] = { -1 };
        if(sizetable[0] < 0)
        {
            memset(sizetable, -1, sizeof(sizetable));
            for(const int *p = msgsizes; *p >= 0; p += 2) sizetable[p[0]] = p[1];
        }
        return msg >= 0 && msg < NUMSV ? sizetable[msg] : -1;
    }

    const char *modename(int n, const char *unknown) 
    { 
        if(m_valid(n)) return gamemodes[n - STARTGAMEMODE].name;
        return unknown;
    }
    
    int modecode(const char * modename)
    {
        int count = sizeof(gamemodes)/sizeof(gamemodeinfo);
        for(int i = 0-STARTGAMEMODE; i < count; i++)
            if(strcmp(modename,gamemodes[i].name)==0) return i+STARTGAMEMODE;
        return 0;
    }
    
    const char *mastermodename(int n, const char *unknown)
    {
        return (n>=MM_START && size_t(n-MM_START)<sizeof(mastermodenames)/sizeof(mastermodenames[0])) ? mastermodenames[n-MM_START] : unknown;
    }

    const char *privname(int type)
    {
        switch(type)
        {
            case PRIV_ADMIN: return "admin";
            case PRIV_MASTER: return "master";
            default: return "none";
        }
    }

    void sendservmsg(const char *s) { sendf(-1, 1, "ris", N_SERVMSG, s); }

    void resetitems() 
    { 
        sents.shrink(0);
        //cps.reset(); 
    }

    bool serveroption(const char *arg)
    {
        if(arg[0]=='-') switch(arg[1])
        {
            case 'n': copystring(serverdesc, &arg[2]); return true;
            case 'y': copystring(serverpass, &arg[2]); return true;
            case 'p': copystring(adminpass, &arg[2]); return true;
            case 'o': {mastermask = (atoi(&arg[2]) ? MM_PUBSERV : MM_PRIVSERV); return true;}
            case 'g': aiman::botlimit = clamp(atoi(&arg[2]), 0, MAXBOTS); return true;
        }
        return false;
    }

    void serverinit()
    {
        smapname[0] = '\0';
        resetitems();
        
        init_hopmod();
    }
    
    int numclients(int exclude = -1, bool nospec = true, bool noai = true)
    {
        int n = 0;
        loopv(clients) if(i!=exclude && (!nospec || clients[i]->state.state!=CS_SPECTATOR) && (!noai || clients[i]->state.aitype == AI_NONE)) n++;
        return n;
    }
    
    bool duplicatename(clientinfo *ci, char *name)
    {
        if(!name) name = ci->name;
        loopv(clients) if(clients[i]!=ci && !strcmp(name, clients[i]->name)) return true;
        return false;
    }

    const char *colorname(clientinfo *ci, char *name = NULL)
    {
        if(!name) name = ci->name;
        if(name[0] && !duplicatename(ci, name) && ci->state.aitype == AI_NONE) return name;
        static string cname[3];
        static int cidx = 0;
        cidx = (cidx+1)%3;
        formatstring(cname[cidx])(ci->state.aitype == AI_NONE ? "%s \fs\f5(%d)\fr" : "%s \fs\f5[%d]\fr", name, ci->clientnum);
        return cname[cidx];
    }

    bool pickup(int i, int sender)         // server side item pickup, acknowledge first client that gets it
    {
        if(gamemillis>=gamelimit || !sents.inrange(i) || !sents[i].spawned) return false;
        clientinfo *ci = getinfo(sender);
        if(!ci || (!ci->local && !ci->state.canpickup(sents[i].type))) return false;
        sents[i].spawned = false;
        sents[i].spawntime = spawntime(sents[i].type);
        sendf(-1, 1, "ri3", N_ITEMACC, i, sender);
        ci->state.pickup(sents[i].type);
        return true;
    }

    clientinfo *choosebestclient(float &bestrank)
    {
        clientinfo *best = NULL;
        bestrank = -1;
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.timeplayed<0) continue;
            float rank = ci->state.state!=CS_SPECTATOR ? ci->state.effectiveness/max(ci->state.timeplayed, 1) : -1;
            if(!best || rank > bestrank) { best = ci; bestrank = rank; }
        }
        return best;
    }  

    void autoteam()
    {
        static const char *teamnames[2] = {"good", "evil"};
        vector<clientinfo *> team[2];
        float teamrank[2] = {0, 0};
        for(int round = 0, remaining = clients.length(); remaining>=0; round++)
        {
            int first = round&1, second = (round+1)&1, selected = 0;
            while(teamrank[first] <= teamrank[second])
            {
                float rank;
                clientinfo *ci = choosebestclient(rank);
                if(!ci) break;
                if(smode && smode->hidefrags()) rank = 1;
                else if(selected && rank<=0) break;    
                ci->state.timeplayed = -1;
                team[first].add(ci);
                if(rank>0) teamrank[first] += rank;
                selected++;
                if(rank<=0) break;
            }
            if(!selected) break;
            remaining -= selected;
        }
        loopi(sizeof(team)/sizeof(team[0]))
        {
            loopvj(team[i])
            {
                clientinfo *ci = team[i][j];
                if(!strcmp(ci->team, teamnames[i])) continue;
                copystring(ci->team, teamnames[i], MAXTEAMLEN+1);
                sendf(-1, 1, "riisi", N_SETTEAM, ci->clientnum, teamnames[i], -1);
            }
        }
    }

    struct teamrank
    {
        const char *name;
        float rank;
        int clients;

        teamrank(const char *name) : name(name), rank(0), clients(0) {}
    };
    
    const char *chooseworstteam(const char *suggest = NULL, clientinfo *exclude = NULL)
    {
        teamrank teamranks[2] = { teamrank("good"), teamrank("evil") };
        const int numteams = sizeof(teamranks)/sizeof(teamranks[0]);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci==exclude || ci->state.aitype!=AI_NONE || ci->state.state==CS_SPECTATOR || !ci->team[0]) continue;
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
            ci->state.lasttimeplayed = lastmillis;

            loopj(numteams) if(!strcmp(ci->team, teamranks[j].name)) 
            { 
                teamrank &ts = teamranks[j];
                ts.rank += ci->state.effectiveness/max(ci->state.timeplayed, 1);
                ts.clients++;
                break;
            }
        }
        teamrank *worst = &teamranks[numteams-1];
        loopi(numteams-1)
        {
            teamrank &ts = teamranks[i];
            if(smode && smode->hidefrags())
            {
                if(ts.clients < worst->clients || (ts.clients == worst->clients && ts.rank < worst->rank)) worst = &ts;
            }
            else if(ts.rank < worst->rank || (ts.rank == worst->rank && ts.clients < worst->clients)) worst = &ts;
        }
        return worst->name;
    }

    void writedemo(int chan, void *data, int len)
    {
        if(!demorecord) return;
        int stamp[3] = { gamemillis - demoffset, chan, len };
        lilswap(stamp, 3);
        demorecord->write(stamp, sizeof(stamp));
        demorecord->write(data, len);
    }

    void write_delayed_broadcast(int chan, void * data, int len)
    {
        if(!spectator_delay || interm) return;
        
        delayed_sendpacket sendpacket;
        sendpacket.time = totalmillis + spectator_delay;
        sendpacket.channel = chan;
        sendpacket.data = malloc(len);
        memcpy(sendpacket.data, data, len);
        sendpacket.length = len;
        
        delayed_sendpackets.add(sendpacket);
    }

    void recordpacket(int chan, void *data, int len)
    {
        writedemo(chan, data, len);
        write_delayed_broadcast(chan, data, len);
    }

    void enddemorecord()
    {
        if(!demorecord) return;

        DELETEP(demorecord);

        if(!demotmp) return;

        int len = demotmp->size();
        if(demos.length()>=MAXDEMOS)
        {
            delete[] demos[0].data;
            demos.remove(0);
        }
        demofile &d = demos.add();
        time_t t = time(NULL);
        char *timestr = ctime(&t), *trim = timestr + strlen(timestr);
        while(trim>timestr && isspace(*--trim)) *trim = '\0';
        formatstring(d.info)("%s: %s, %s, %.2f%s", timestr, modename(gamemode), smapname, len > 1024*1024 ? len/(1024*1024.f) : len/1024.0f, len > 1024*1024 ? "MB" : "kB");
        defformatstring(msg)("demo \"%s\" recorded", d.info);
        sendservmsg(msg);
        d.data = new uchar[len];
        d.len = len;
        demotmp->seek(0, SEEK_SET);
        demotmp->read(d.data, len);
        DELETEP(demotmp);
        
        signal_endrecord(demo_id,len);
    }

    int welcomepacket(packetbuf &p, clientinfo *ci);
    void sendwelcome(clientinfo *ci);

    int setupdemorecord(bool broadcast = true, const char * filename = NULL)
    {
        if(!m_mp(gamemode) || m_edit) return -1;
        
        string defaultfilename;
        
        if(!filename || filename[0]=='\0')
        {
            demo_id = demos.length() + (demos.length()>=MAXDEMOS ? 0 : 1);
            char ftime[32];
            ftime[0]='\0';
            time_t now = time(NULL);
            strftime(ftime,sizeof(ftime),"%0e%b%Y_%H:%M",localtime(&now));
            formatstring(defaultfilename)("log/demo/%s_%s_%i.dmo",ftime,smapname,demo_id);
            filename = defaultfilename;
        }
        
        demotmp = openfile(filename,"w+b");
        if(!demotmp || m_edit) return -1;
        
        stream *f = opengzfile(NULL, "wb", demotmp);
        if(!f) { DELETEP(demotmp); return -1; } 

        if(broadcast) sendservmsg("recording demo");

        demorecord = f;
        demoffset = gamemillis;

        demoheader hdr;
        memcpy(hdr.magic, DEMO_MAGIC, sizeof(hdr.magic));
        hdr.version = DEMO_VERSION;
        hdr.protocol = PROTOCOL_VERSION;
        lilswap(&hdr.version, 2);
        demorecord->write(&hdr, sizeof(demoheader));

        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        welcomepacket(p, NULL);
        writedemo(1, p.buf, p.len);

        signal_beginrecord(demo_id, filename);

        return demo_id;
    }

    void listdemos(int cn)
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        putint(p, N_SENDDEMOLIST);
        putint(p, demos.length());
        loopv(demos) sendstring(demos[i].info, p);
        sendpacket(cn, 1, p.finalize());
    }

    void cleardemos(int n)
    {
        if(demorecord) return; //HOPMOD
        if(!n)
        {
            loopv(demos) delete[] demos[i].data;
            demos.shrink(0);
            sendservmsg("cleared all demos");
        }
        else if(demos.inrange(n-1))
        {
            delete[] demos[n-1].data;
            demos.remove(n-1);
            defformatstring(msg)("cleared demo %d", n);
            sendservmsg(msg);
        }
    }
	
    static void freegetmap(ENetPacket *packet)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->getmap == packet) ci->getmap = NULL;
        }
    }

    static void freegetdemo(ENetPacket *packet)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->getdemo == packet) ci->getdemo = NULL;
        }
    }

    void senddemo(clientinfo *ci, int num)
    {
        if(ci->getdemo) return;
        if(!num) num = demos.length();
        if(!demos.inrange(num-1)) return;
        demofile &d = demos[num-1];
        if((ci->getdemo = sendf(ci->clientnum, 2, "rim", N_SENDDEMO, d.len, d.data)))
            ci->getdemo->freeCallback = freegetdemo;
    }

    void enddemoplayback()
    {
        if(!demoplayback) return;
        DELETEP(demoplayback);

        loopv(clients) sendf(clients[i]->clientnum, 1, "ri3", N_DEMOPLAYBACK, 0, clients[i]->clientnum);

        sendservmsg("demo playback finished");

        loopv(clients) sendwelcome(clients[i]);
    }

    void setupdemoplayback()
    {
        if(demoplayback) return;
        demoheader hdr;
        string msg;
        msg[0] = '\0';
        defformatstring(file)("%s.dmo", smapname);
        demoplayback = opengzfile(file, "rb");
        if(!demoplayback) formatstring(msg)("could not read demo \"%s\"", file);
        else if(demoplayback->read(&hdr, sizeof(demoheader))!=sizeof(demoheader) || memcmp(hdr.magic, DEMO_MAGIC, sizeof(hdr.magic)))
            formatstring(msg)("\"%s\" is not a demo file", file);
        else 
        { 
            lilswap(&hdr.version, 2);
            if(hdr.version!=DEMO_VERSION) formatstring(msg)("demo \"%s\" requires an %s version of Cube 2: Sauerbraten", file, hdr.version<DEMO_VERSION ? "older" : "newer");
            else if(hdr.protocol!=PROTOCOL_VERSION) formatstring(msg)("demo \"%s\" requires an %s version of Cube 2: Sauerbraten", file, hdr.protocol<PROTOCOL_VERSION ? "older" : "newer");
        }
        if(msg[0])
        {
            DELETEP(demoplayback);
            sendservmsg(msg);
            return;
        }

        formatstring(msg)("playing demo \"%s\"", file);
        sendservmsg(msg);

        demomillis = 0;
        sendf(-1, 1, "ri3", N_DEMOPLAYBACK, 1, -1);

        if(demoplayback->read(&nextplayback, sizeof(nextplayback))!=sizeof(nextplayback))
        {
            enddemoplayback();
            return;
        }
        lilswap(&nextplayback, 1);
    }

    void readdemo()
    {
        if(!demoplayback || gamepaused) return;
        demomillis += curtime;
        while(demomillis>=nextplayback)
        {
            int chan, len;
            if(demoplayback->read(&chan, sizeof(chan))!=sizeof(chan) ||
               demoplayback->read(&len, sizeof(len))!=sizeof(len))
            {
                enddemoplayback();
                return;
            }
            lilswap(&chan, 1);
            lilswap(&len, 1);
            ENetPacket *packet = enet_packet_create(NULL, len, 0);
            if(!packet || demoplayback->read(packet->data, len)!=len)
            {
                if(packet) enet_packet_destroy(packet);
                enddemoplayback();
                return;
            }
            sendpacket(-1, chan, packet);
            if(!packet->referenceCount) enet_packet_destroy(packet);
            if(demoplayback->read(&nextplayback, sizeof(nextplayback))!=sizeof(nextplayback))
            {
                enddemoplayback();
                return;
            }
            lilswap(&nextplayback, 1);
        }
    }

    void stopdemo()
    {
        if(m_demo) enddemoplayback();
        else enddemorecord();
    }
    
    void pausegame(bool val)
    {
        pausegame_owner = -1;
        if(gamepaused==val) return;
        gamepaused = val;
        sendf(-1, 1, "rii", N_PAUSEGAME, gamepaused ? 1 : 0);
        if(gamepaused) signal_gamepaused();
        else signal_gameresumed();
    }
    
    void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen)
    {
        char buf[2*sizeof(string)];
        formatstring(buf)("%d %d ", cn, sessionid);
        copystring(&buf[strlen(buf)], pwd);
        if(!hashstring(buf, result, maxlen)) *result = '\0';
    }

    bool checkpassword(clientinfo *ci, const char *wanted, const char *given)
    {
        string hash;
        hashpassword(ci->clientnum, ci->sessionid, wanted, hash, sizeof(hash));
        return !strcmp(hash, given);
    }
    
    void revokemaster(clientinfo *ci)
    {
        ci->privilege = PRIV_NONE;
        if(ci->state.state==CS_SPECTATOR && !ci->local) aiman::removeai(ci);
    }

    void setmaster(clientinfo *ci, bool request_claim_master, const char * hashed_password = "", const char *authname = NULL)
    {
        assert(!authname);
        update_mastermask();
        signal_setmaster(ci->clientnum, hashed_password, request_claim_master);
        return;
    }
    
    savedscore &findscore(clientinfo *ci, bool insert)
    {
        uint ip = getclientip(ci->clientnum);
        if(!ip && !ci->local) return *(savedscore *)0;
        if(!insert) 
        {
            loopv(clients)
            {
                clientinfo *oi = clients[i];
                if(oi->clientnum != ci->clientnum && getclientip(oi->clientnum) == ip && !strcmp(oi->name, ci->name))
                {
                    oi->state.timeplayed += lastmillis - oi->state.lasttimeplayed;
                    oi->state.lasttimeplayed = lastmillis;
                    static savedscore curscore;
                    curscore.save(oi->state);
                    return curscore;
                }
            }
        }
        loopv(scores)
        {
            savedscore &sc = scores[i];
            if(sc.ip == ip && !strcmp(sc.name, ci->name)) return sc;
        }
        if(!insert) return *(savedscore *)0;
        savedscore &sc = scores.add();
        sc.ip = ip;
        copystring(sc.name, ci->name);
        return sc;
    }

    void savescore(clientinfo *ci)
    {
        savedscore &sc = findscore(ci, true);
        if(&sc) sc.save(ci->state);
    }

    int checktype(int type, clientinfo *ci)
    {
        if(ci && ci->local) return type;
        // only allow edit messages in coop-edit mode
        if(type>=N_EDITENT && type<=N_EDITVAR && !m_edit) return -1;
        // server only messages
        static const int servtypes[] = { N_SERVINFO, N_INITCLIENT, N_WELCOME, N_MAPRELOAD, N_SERVMSG, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_DIED, N_SPAWNSTATE, N_FORCEDEATH, N_ITEMACC, N_ITEMSPAWN, N_TIMEUP, N_CDIS, N_CURRENTMASTER, N_PONG, N_RESUME, N_BASESCORE, N_BASEINFO, N_BASEREGEN, N_ANNOUNCE, N_SENDDEMOLIST, N_SENDDEMO, N_DEMOPLAYBACK, N_SENDMAP, N_DROPFLAG, N_SCOREFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_CLIENT, N_AUTHCHAL, N_INITAI };
        if(ci)
        {
            loopi(sizeof(servtypes)/sizeof(int)) if(type == servtypes[i]) return -1;
            if(type < N_EDITENT || type > N_EDITVAR || !m_edit) 
            {
                if(type != N_POS && ++ci->overflow >= 200) return -2;
            }
        }
        return type;
    }

    void cleanworldstate(ENetPacket *packet)
    {
        loopv(worldstates)
        {
            worldstate *ws = worldstates[i];
            if(ws->positions.inbuf(packet->data) || ws->messages.inbuf(packet->data)) ws->uses--;
            else continue;
            if(!ws->uses)
            {
                delete ws;
                worldstates.remove(i);
            }
            break;
        }
    }

    void flushclientposition(clientinfo &ci)
    {
        if(ci.position.empty() || (!hasnonlocalclients() && !demorecord)) return;
        packetbuf p(ci.position.length(), 0);
        p.put(ci.position.getbuf(), ci.position.length());
        ci.position.setsize(0);
        sendpacket(-1, 0, p.finalize(), ci.ownernum);
    }

    void addclientstate(worldstate &ws, clientinfo &ci)
    {
        if(ci.position.empty()) ci.posoff = -1;
        else
        {
            ci.posoff = ws.positions.length();
            ws.positions.put(ci.position.getbuf(), ci.position.length());
            ci.poslen = ws.positions.length() - ci.posoff;
            ci.position.setsize(0);
        }
        if(ci.messages.empty()) ci.msgoff = -1;
        else
        {
            ci.msgoff = ws.messages.length();
            putint(ws.messages, N_CLIENT);
            putint(ws.messages, ci.clientnum);
            putuint(ws.messages, ci.messages.length());
            ws.messages.put(ci.messages.getbuf(), ci.messages.length());
            ci.msglen = ws.messages.length() - ci.msgoff;
            ci.messages.setsize(0);
        }
    }
    
    bool buildworldstate()
    {
        worldstate &ws = *new worldstate;
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            ci.overflow = 0;
            addclientstate(ws, ci);
            loopv(ci.bots)
            {
                clientinfo &bi = *ci.bots[i];
                addclientstate(ws, bi);
                if(bi.posoff >= 0)
                {
                    if(ci.posoff < 0) { ci.posoff = bi.posoff; ci.poslen = bi.poslen; }
                    else ci.poslen += bi.poslen;
                }
                if(bi.msgoff >= 0)
                {
                    if(ci.msgoff < 0) { ci.msgoff = bi.msgoff; ci.msglen = bi.msglen; }
                    else ci.msglen += bi.msglen;
                }
            }
        }
        int psize = ws.positions.length(), msize = ws.messages.length();
        
        if(psize)
        {
            recordpacket(0, ws.positions.getbuf(), psize);
            ucharbuf p = ws.positions.reserve(psize);
            p.put(ws.positions.getbuf(), psize);
            ws.positions.addbuf(p);
        }
        if(msize)
        {
            recordpacket(1, ws.messages.getbuf(), msize);
            ucharbuf p = ws.messages.reserve(msize);
            p.put(ws.messages.getbuf(), msize);
            ws.messages.addbuf(p);
        }
        ws.uses = 0;
        if(psize || msize) loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE || ci.is_delayed_spectator()) continue;
            ENetPacket *packet;
            if(psize && (ci.posoff<0 || psize-ci.poslen>0))
            {
                packet = enet_packet_create(&ws.positions[ci.posoff<0 ? 0 : ci.posoff+ci.poslen],
                                            ci.posoff<0 ? psize : psize-ci.poslen,
                                            ENET_PACKET_FLAG_NO_ALLOCATE);
                sendpacket(ci.clientnum, 0, packet);
                if(!packet->referenceCount) enet_packet_destroy(packet);
                else { ++ws.uses; packet->freeCallback = cleanworldstate; }
            }

            if(msize && (ci.msgoff<0 || msize-ci.msglen>0))
            {
                packet = enet_packet_create(&ws.messages[ci.msgoff<0 ? 0 : ci.msgoff+ci.msglen],
                                            ci.msgoff<0 ? msize : msize-ci.msglen,
                                            (reliablemessages ? ENET_PACKET_FLAG_RELIABLE : 0) | ENET_PACKET_FLAG_NO_ALLOCATE);
                sendpacket(ci.clientnum, 1, packet);
                if(!packet->referenceCount) enet_packet_destroy(packet);
                else { ++ws.uses; packet->freeCallback = cleanworldstate; }
            }
        }
        reliablemessages = false;
        if(!ws.uses)
        {
            delete &ws;
            return false;
        }
        else
        {
            worldstates.add(&ws);
            return true;
        }
    }
    
    void free_packet_data(ENetPacket * packet)
    {
        free(packet->data);
    }

    bool sendpackets(bool force)
    {
        if(clients.empty() || (!hasnonlocalclients() && !demorecord)) return false;
        
        if(delayed_sendpackets.length() && totalmillis >= delayed_sendpackets[0].time)
        {
            int flags = (delayed_sendpackets[0].channel == 1 ? ENET_PACKET_FLAG_RELIABLE : 0);
            ENetPacket * packet = enet_packet_create(delayed_sendpackets[0].data, delayed_sendpackets[0].length, flags | ENET_PACKET_FLAG_NO_ALLOCATE);
            packet->freeCallback = free_packet_data;
            
            loopv(clients)
            {
                clientinfo &ci = *clients[i];
                if(!ci.is_delayed_spectator() || ci.specmillis + spectator_delay > totalmillis) continue;
                sendpacket(ci.clientnum, delayed_sendpackets[0].channel, packet);
            }
            
            delayed_sendpackets.remove(0);
        }
        
        enet_uint32 curtime = enet_time_get()-lastsend;
        if(curtime<33 && !force) return false;
        bool flush = buildworldstate();
        
        lastsend += curtime - (curtime%33);
        return flush;
    }

    template<class T>
    void sendstate(gamestate &gs, T &p)
    {
        putint(p, gs.lifesequence);
        putint(p, gs.health);
        putint(p, gs.maxhealth);
        putint(p, gs.armour);
        putint(p, gs.armourtype);
        putint(p, gs.gunselect);
        loopi(GUN_PISTOL-GUN_SG+1) putint(p, gs.ammo[GUN_SG+i]);
    }

    void spawnstate(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        gs.spawnstate(gamemode);
        gs.lifesequence = (gs.lifesequence + 1)&0x7F;
    }

    void sendspawn(clientinfo *ci)
    {
        if (ci->no_spawn == 1) return;
        gamestate &gs = ci->state;
        spawnstate(ci);
        sendf(ci->ownernum, 1, "rii7v", N_SPAWNSTATE, ci->clientnum, gs.lifesequence,
            gs.health, gs.maxhealth,
            gs.armour, gs.armourtype,
            gs.gunselect, GUN_PISTOL-GUN_SG+1, &gs.ammo[GUN_SG]);
        gs.lastspawn = gamemillis;
    }

    void sendwelcome(clientinfo *ci)
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        int chan = welcomepacket(p, ci);
        sendpacket(ci->clientnum, chan, p.finalize());
    }

    void putinitclient(clientinfo *ci, packetbuf &p)
    {
        if(ci->state.aitype != AI_NONE)
        {
            putint(p, N_INITAI);
            putint(p, ci->clientnum);
            putint(p, ci->ownernum);
            putint(p, ci->state.aitype);
            putint(p, ci->state.skill);
            putint(p, ci->playermodel);
            sendstring(ci->name, p);
            sendstring(ci->team, p);
        }
        else
        {
            putint(p, N_INITCLIENT);
            putint(p, ci->clientnum);
            sendstring(ci->name, p);
            sendstring(ci->team, p);
            putint(p, ci->playermodel);
        }
    }

    void welcomeinitclient(packetbuf &p, int exclude = -1)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(!ci->connected || ci->clientnum == exclude) continue;

            putinitclient(ci, p);
        }
    }

    int welcomepacket(packetbuf &p, clientinfo *ci)
    {
        int hasmap = (m_edit && (clients.length()>1 || (ci && ci->local))) || (smapname[0] && (gamemillis<gamelimit || (ci && ci->state.state==CS_SPECTATOR) || numclients(ci && ci->local ? ci->clientnum : -1)));
        putint(p, N_WELCOME);
        putint(p, hasmap);
        if(hasmap)
        {
            putint(p, N_MAPCHANGE);
            sendstring(smapname, p);
            putint(p, gamemode);
            putint(p, notgotitems ? 1 : 0);
            if(!ci || (m_timed && smapname[0]))
            {
                putint(p, N_TIMEUP);
                int timeleft = gamemillis < gamelimit && !interm ? max((gamelimit - gamemillis)/1000, 1) : 0;
                if(spectator_delay) timeleft += spectator_delay/1000;
                putint(p, timeleft);
            }
            if(!notgotitems)
            {
                putint(p, N_ITEMLIST);
                loopv(sents) if(sents[i].spawned)
                {
                    putint(p, i);
                    putint(p, sents[i].type);
                }
                putint(p, -1);
            }
        }
        if(currentmaster >= 0 || mastermode != MM_OPEN)
        {
            putint(p, N_CURRENTMASTER);
            putint(p, currentmaster);
            clientinfo *m = currentmaster >= 0 ? getinfo(currentmaster) : NULL;
            putint(p, m ? m->privilege : 0);
            putint(p, mastermode);
        }
        if(gamepaused)
        {
            putint(p, N_PAUSEGAME);
            putint(p, 1);
        }
        if(ci)
        {
            putint(p, N_SETTEAM);
            putint(p, ci->clientnum);
            sendstring(ci->team, p);
            putint(p, -1);
        }
        if(ci && (m_demo || m_mp(gamemode)) && ci->state.state!=CS_SPECTATOR)
        {
            if(smode && !smode->canspawn(ci, true))
            {
                ci->state.state = CS_DEAD;
                putint(p, N_FORCEDEATH);
                putint(p, ci->clientnum);
                sendf(-1, 1, "ri2x", N_FORCEDEATH, ci->clientnum, ci->clientnum);
            }
            else
            {
                gamestate &gs = ci->state;
                spawnstate(ci);
                putint(p, N_SPAWNSTATE);
                putint(p, ci->clientnum);
                sendstate(gs, p);
                gs.lastspawn = gamemillis;
            }
        }
        if(ci && ci->state.state==CS_SPECTATOR)
        {
            putint(p, N_SPECTATOR);
            putint(p, ci->clientnum);
            putint(p, 1);
            sendf(-1, 1, "ri3x", N_SPECTATOR, ci->clientnum, 1, ci->clientnum);
        }
        if(!ci || clients.length()>1)
        {
            putint(p, N_RESUME);
            loopv(clients)
            {
                clientinfo *oi = clients[i];
                if(ci && oi->clientnum==ci->clientnum) continue;
                putint(p, oi->clientnum);
                putint(p, oi->state.state);
                putint(p, oi->state.frags);
                putint(p, oi->state.flags);
                putint(p, oi->state.quadmillis);
                sendstate(oi->state, p);
            }
            putint(p, -1);
            welcomeinitclient(p, ci ? ci->clientnum : -1);
        }
        if(smode) smode->initclient(ci, p, true);
        return 1;
    }

    bool restorescore(clientinfo *ci)
    {
        //if(ci->local) return false;
        savedscore &sc = findscore(ci, false);
        if(&sc)
        {
            sc.restore(ci->state);
            return true;
        }
        return false;
    }
    
    void sendresume(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        sendf(-1, 1, "ri3i9vi", N_RESUME, ci->clientnum,
            gs.state, gs.frags, gs.flags, gs.quadmillis,
            gs.lifesequence,
            gs.health, gs.maxhealth,
            gs.armour, gs.armourtype,
            gs.gunselect, GUN_PISTOL-GUN_SG+1, &gs.ammo[GUN_SG], -1);
    }

    void sendinitclient(clientinfo *ci)
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        putinitclient(ci, p);
        sendpacket(-1, 1, p.finalize(), ci->clientnum);
    }

    void changemap(const char *s, int mode,int mins = -1)
    {
        calc_player_ranks();
        signal_finishedgame();
        
        stopdemo();
        pausegame(false);
        if(smode) smode->reset(false);
        aiman::clearai();
        
        mapreload = false;
        gamemode = mode;
        gamemillis = 0;
        gamelimit = (mins == -1 ? (m_overtime ? 15 : 10) : mins) * 60000;
        interm = 0;
        nextexceeded = 0;
        copystring(smapname, s);
        resetitems();
        notgotitems = true;
        scores.shrink(0);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
        }

        if(!m_mp(gamemode)) kicknonlocalclients(DISC_PRIVATE);

        if(m_teammode && reassignteams) autoteam();

        if(m_capture) smode = &capturemode;
        else if(m_ctf) smode = &ctfmode;
        else smode = NULL;
        if(smode) smode->reset(false);

        if(m_timed && smapname[0]) sendf(-1, 1, "ri2", N_TIMEUP, gamemillis < gamelimit && !interm ? max((gamelimit - gamemillis)/1000, 1) : 0);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            ci->mapchange();
            ci->state.lasttimeplayed = lastmillis;
            if(m_mp(gamemode) && ci->state.state!=CS_SPECTATOR) sendspawn(ci);
        }

        aiman::changemap();

        if(m_demo) 
        {
            if(clients.length()) setupdemoplayback();
        }
        else if(demonextmatch)
        {
            demonextmatch = false;
            setupdemorecord();
        }
        
        signal_mapchange(smapname,modename(gamemode,"unknown"));
        
        next_timeupdate = 0; //as soon as possible
    }

    struct votecount
    {
        char *map;
        int mode, count;
        votecount() {}
        votecount(char *s, int n) : map(s), mode(n), count(0) {}
    };

    void checkvotes(bool force = false)
    {
        vector<votecount> votes;
        int maxvotes = 0;
        loopv(clients)
        {
            clientinfo *oi = clients[i];
            if(oi->state.state==CS_SPECTATOR && !oi->privilege && !oi->local) continue;
            if(oi->state.aitype!=AI_NONE) continue;
            maxvotes++;
            if(!oi->mapvote[0]) continue;
            votecount *vc = NULL;
            loopvj(votes) if(!strcmp(oi->mapvote, votes[j].map) && oi->modevote==votes[j].mode)
            {
                vc = &votes[j];
                break;
            }
            if(!vc) vc = &votes.add(votecount(oi->mapvote, oi->modevote));
            vc->count++;
        }
        votecount *best = NULL;
        loopv(votes) if(!best || votes[i].count > best->count || (votes[i].count == best->count && rnd(2))) best = &votes[i];
        if(force || (best && best->count > maxvotes/2))
        {
            if(demorecord) enddemorecord();
            if(best && (best->count > (force ? 1 : maxvotes/2)))
            {
                sendservmsg(force ? "vote passed by default" : "vote passed by majority");
                signal_votepassed(best->map, modename(best->mode));
                sendf(-1, 1, "risii", N_MAPCHANGE, best->map, best->mode, 1);
                changemap(best->map, best->mode);
            }
            else
            {
                mapreload = true;
                if(clients.length() && !selectnextgame()) sendf(-1, 1, "ri", N_MAPRELOAD);
            }
        }
    }

    void forcemap(const char *map, int mode)
    {
        stopdemo();
        if(hasnonlocalclients() && !mapreload)
        {
            defformatstring(msg)("local player forced %s on map %s", modename(mode), map);
            sendservmsg(msg);
        }
        sendf(-1, 1, "risii", N_MAPCHANGE, map, mode, 1);
        changemap(map, mode);
    }

    void vote(char *map, int reqmode, int sender)
    {
        clientinfo *ci = getinfo(sender);
        if(!ci || (ci->state.state==CS_SPECTATOR && !ci->privilege && !ci->local) || (!ci->local && !m_mp(reqmode))) return;
        strncpy(ci->mapvote, map, 50);
        ci->modevote = reqmode;
        if(!ci->mapvote[0]) return;
        if(ci->local || mapreload || (ci->privilege && mastermode>=MM_VETO))
        {
            if(demorecord) enddemorecord();
            if((!ci->local || hasnonlocalclients()) && !mapreload)
            {
                defformatstring(msg)("%s forced %s on map %s", ci->privilege && mastermode>=MM_VETO ? privname(ci->privilege) : "local player", modename(ci->modevote), ci->mapvote);
                sendservmsg(msg);
            }
            sendf(-1, 1, "risii", N_MAPCHANGE, ci->mapvote, ci->modevote, 1);
            changemap(ci->mapvote, ci->modevote);
        }
        else
        {
            defformatstring(msg)("%s suggests %s on map %s (select map to vote)", colorname(ci), modename(reqmode), map);
            sendservmsg(msg);
            checkvotes();
        }
    }

    void checkintermission()
    {
        if(gamemillis >= gamelimit && !interm)
        {
            signal_timeupdate(0, 0);
            if(gamemillis < gamelimit) return;
            
            sendf(-1, 1, "ri2", N_TIMEUP, 0);
            interm = gamemillis+intermtime;
            calc_player_ranks();
            signal_intermission();
        }
    }

    void startintermission() { gamelimit = min(gamelimit, gamemillis); checkintermission(); }

    void dodamage(clientinfo *target, clientinfo *actor, int damage, int gun, const vec &hitpush = vec(0, 0, 0))
    {
        if(signal_damage(target->clientnum, actor->clientnum, damage, gun) == -1){
            return;
        }
        
        gamestate &ts = target->state;
        ts.dodamage(damage);
        actor->state.damage += damage;
        sendf(-1, 1, "ri6", N_DAMAGE, target->clientnum, actor->clientnum, damage, ts.armour, ts.health); 
        if(target==actor) target->setpushed();
        else if(target!=actor && !hitpush.iszero())
        {
            ivec v = vec(hitpush).rescale(DNF);
            sendf(ts.health<=0 ? -1 : target->ownernum, 1, "ri7", N_HITPUSH, target->clientnum, gun, damage, v.x, v.y, v.z);
            target->setpushed();
        }
        if(ts.health<=0)
        {
            target->state.deaths++;
            target->state.suicides += actor==target;
            if(actor!=target && isteam(actor->team, target->team))
            {
                actor->state.teamkills++;
                signal_teamkill(actor->clientnum, target->clientnum);
            }
            int fragvalue = smode ? smode->fragvalue(target, actor) : (target==actor || isteam(target->team, actor->team) ? -1 : 1);
            actor->state.frags += fragvalue;
            if(fragvalue>0)
            {
                int friends = 0, enemies = 0; // note: friends also includes the fragger
                if(m_teammode) loopv(clients) if(strcmp(clients[i]->team, actor->team)) enemies++; else friends++;
                else { friends = 1; enemies = clients.length()-1; }
                actor->state.effectiveness += fragvalue*friends/float(max(enemies, 1));
            }
            signal_frag(target->clientnum, actor->clientnum);
            sendf(-1, 1, "ri4", N_DIED, target->clientnum, actor->clientnum, actor->state.frags);
            target->position.setsize(0);
            if(smode) smode->died(target, actor);
            ts.state = CS_DEAD;
            ts.lastdeath = gamemillis;
            // don't issue respawn yet until DEATHMILLIS has elapsed
            // ts.respawn();
        }
    }
    
    void suicide(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        if(gs.state!=CS_ALIVE) return;
        ci->state.frags += smode ? smode->fragvalue(ci, ci) : -1;
        ci->state.deaths++;
        ci->state.suicides++;
        sendf(-1, 1, "ri4", N_DIED, ci->clientnum, ci->clientnum, gs.frags);
        ci->position.setsize(0);
        if(smode) smode->died(ci, NULL);
        gs.state = CS_DEAD;
        gs.respawn();
        signal_suicide(ci->clientnum);
    }


    void suicideevent::process(clientinfo *ci)
    {
        suicide(ci);
    }

    void explodeevent::process(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        switch(gun)
        {
            case GUN_RL:
                if(!gs.rockets.remove(id)) return;
                break;

            case GUN_GL:
                if(!gs.grenades.remove(id)) return;
                break;

            default:
                return;
        }
        gs.explosivedamage += guns[gun].damage * (gs.quadmillis ? 4 : 1);
        sendf(-1, 1, "ri4x", N_EXPLODEFX, ci->clientnum, gun, id, ci->ownernum);
        loopv(hits)
        {
            hitinfo &h = hits[i];
            clientinfo *target = getinfo(h.target);
            if(!target || target->state.state!=CS_ALIVE || h.lifesequence!=target->state.lifesequence || h.dist<0 || h.dist>RL_DAMRAD) continue;
            
            bool dup = false;
            loopj(i) if(hits[j].target==h.target) { dup = true; break; }
            if(dup) continue;
            
            gs.hits += (ci != target ? 1 : 0);
            
            int damage = guns[gun].damage;
            if(gs.quadmillis) damage *= 4;        
            damage = int(damage*(1-h.dist/RL_DISTSCALE/RL_DAMRAD));
            if(gun==GUN_RL && target==ci) damage /= RL_SELFDAMDIV;
            dodamage(target, ci, damage, gun, h.dir);
        }
    }
        
    void shotevent::process(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        int wait = millis - gs.lastshot;
        if(!gs.isalive(gamemillis) ||
           wait<gs.gunwait ||
           gun<GUN_FIST || gun>GUN_PISTOL ||
           gs.ammo[gun]<=0 || (guns[gun].range && from.dist(to) > guns[gun].range + 1))
            return;
        if(gun!=GUN_FIST) gs.ammo[gun]--;
        gs.lastshot = millis;
        gs.gunwait = guns[gun].attackdelay;
        sendf(-1, 1, "rii9x", N_SHOTFX, ci->clientnum, gun, id,
                int(from.x*DMF), int(from.y*DMF), int(from.z*DMF),
                int(to.x*DMF), int(to.y*DMF), int(to.z*DMF),
                ci->ownernum);
        gs.shotdamage += guns[gun].damage*(gs.quadmillis ? 4 : 1)*(gun==GUN_SG ? SGRAYS : 1);
        
        gs.shots++;
        int old_hits = gs.hits; 
        
        switch(gun)
        {
            case GUN_RL: gs.rockets.add(id); break;
            case GUN_GL: gs.grenades.add(id); break;
            default:
            {
                int totalrays = 0, maxrays = gun==GUN_SG ? SGRAYS : 1;
                loopv(hits)
                {
                    hitinfo &h = hits[i];
                    clientinfo *target = getinfo(h.target);
                    if(!target || target->state.state!=CS_ALIVE || h.lifesequence!=target->state.lifesequence || h.rays<1 || h.dist > guns[gun].range + 1) continue;

                    totalrays += h.rays;
                    if(totalrays>maxrays) continue;
                    
                    gs.hits += (ci != target ? 1 : 0);
                    
                    int damage = h.rays*guns[gun].damage;
                    if(gs.quadmillis) damage *= 4;
                    dodamage(target, ci, damage, gun, h.dir);
                }
                break;
            }
        }
        
        gs.misses += (gs.hits - old_hits == 0);
        
        signal_shot(ci->clientnum, gun, gs.hits - old_hits);
    }

    void pickupevent::process(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        if(m_mp(gamemode) && !gs.isalive(gamemillis)) return;
        pickup(ent, ci->clientnum);
    }

    bool gameevent::flush(clientinfo *ci, int fmillis)
    {
        process(ci);
        return true;
    }

    bool timedevent::flush(clientinfo *ci, int fmillis)
    {
        if(millis > fmillis) return false;
        else if(millis >= ci->lastevent)
        {
            ci->lastevent = millis;
            process(ci);
        }
        return true;
    }

    void clearevent(clientinfo *ci)
    {
        delete ci->events.remove(0);
    }

    void flushevents(clientinfo *ci, int millis)
    {
        while(ci->events.length())
        {
            gameevent *ev = ci->events[0];
            if(ev->flush(ci, millis)) clearevent(ci);
            else break;
        }
    }

    void processevents()
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(curtime>0 && ci->state.quadmillis) ci->state.quadmillis = max(ci->state.quadmillis-curtime, 0);
            flushevents(ci, gamemillis);
        }
    }

    void cleartimedevents(clientinfo *ci)
    {
        int keep = 0;
        loopv(ci->events)
        {
            if(ci->events[i]->keepable())
            {
                if(keep < i)
                {
                    for(int j = keep; j < i; j++) delete ci->events[j];
                    ci->events.remove(keep, i - keep);
                    i = keep;
                }
                keep = i+1;
                continue;
            }
        }
        while(ci->events.length() > keep) delete ci->events.pop();
        ci->timesync = false;
    }

    bool ispaused() { return gamepaused; }

    void serverupdate()
    {
        timer serverupdate_time;
        
        if(!gamepaused) gamemillis += curtime;
        
        if(m_demo) readdemo();
        else if(!gamepaused && (!m_timed || gamemillis < gamelimit))
        {
            processevents();
            if(curtime) 
            {
                loopv(sents) if(sents[i].spawntime) // spawn entities when timer reached
                {
                    int oldtime = sents[i].spawntime;
                    sents[i].spawntime -= curtime;
                    if(sents[i].spawntime<=0)
                    {
                        sents[i].spawntime = 0;
                        sents[i].spawned = true;
                        sendf(-1, 1, "ri2", N_ITEMSPAWN, i);
                    }
                    else if(sents[i].spawntime<=10000 && oldtime>10000 && (sents[i].type==I_QUAD || sents[i].type==I_BOOST))
                    {
                        sendf(-1, 1, "ri2", N_ANNOUNCE, sents[i].type);
                    }
                }
            }
            aiman::checkai();
            if(smode) smode->update();
        }

        loopv(connects) if(totalmillis-connects[i]->connectmillis>15000) disconnect_client(connects[i]->clientnum, DISC_TIMEOUT);
        
        if(nextexceeded && gamemillis > nextexceeded && (!m_timed || gamemillis < gamelimit))
        {
            nextexceeded = 0;
            loopvrev(clients) 
            {
                clientinfo &c = *clients[i];
                if(c.state.aitype != AI_NONE) continue;
                if(c.checkexceeded()) disconnect_client(c.clientnum, DISC_TAGT);
                else c.scheduleexceeded();
            }
        }
        
        if(masterupdate) 
        { 
            clientinfo *m = currentmaster>=0 ? getinfo(currentmaster) : NULL;
            loopv(clients)
            {
                if(clients[i]->state.aitype != AI_NONE) continue;
                if(clients[i]->privilege) sendf(clients[i]->clientnum, 1, "ri4", N_CURRENTMASTER, clients[i]->clientnum, clients[i]->privilege, mastermode);
                else sendf(clients[i]->clientnum, 1, "ri4", N_CURRENTMASTER, currentmaster, m ? m->privilege : 0, mastermode);
            }
            masterupdate = false; 
        } 

        if(gamemillis > next_timeupdate)
        {
            signal_timeupdate(get_minutes_left(), get_seconds_left());
            next_timeupdate = gamemillis + 60000;
        }
        
        if(!gamepaused && m_timed && smapname[0] && gamemillis-curtime>0) checkintermission();
        
        if(interm > 0 && gamemillis > interm + spectator_delay && delayed_sendpackets.length() == 0)
        {
            spectator_delay = 0;
            if(demorecord) enddemorecord();
            interm = -1;
            checkvotes(true);
        }
        
        update_hopmod();
        
        timer::time_diff_t e = serverupdate_time.usec_elapsed();
        if(e >= timer_alarm_threshold) std::cout<<"serverupdate timing: "<<e<<" usecs."<<std::endl;
    }

    struct crcinfo 
    { 
        int crc, matches; 

        crcinfo(int crc, int matches) : crc(crc), matches(matches) {}

        static int compare(const crcinfo *x, const crcinfo *y)
        {
            if(x->matches > y->matches) return -1;
            if(x->matches < y->matches) return 1;
            return 0;
        }
    };

    void sendservinfo(clientinfo *ci)
    {
        sendf(ci->clientnum, 1, "ri5s", N_SERVINFO, ci->clientnum, PROTOCOL_VERSION, ci->sessionid, serverpass[0] ? 1 : 0, serverdesc);
    }

    void noclients()
    {
        signal_clearbans_request();
        aiman::clearai();
    }
    
    void localconnect(int n)
    {
        clientinfo *ci = getinfo(n);
        ci->clientnum = ci->ownernum = n;
        ci->connectmillis = totalmillis;
        ci->sessionid = (rnd(0x1000000)*((totalmillis%10000)+1))&0xFFFFFF;
        ci->local = true;

        connects.add(ci);
        sendservinfo(ci);
    }
    
    void localdisconnect(int n)
    {
        if(m_demo) enddemoplayback();
        clientdisconnect(n,DISC_NONE);
    }

    int clientconnect(int n, uint ip)
    {
        clientinfo *ci = getinfo(n);
        ci->clientnum = ci->ownernum = n;
        ci->connectmillis = totalmillis;
        ci->sessionid = (rnd(0x1000000)*((totalmillis%10000)+1))&0xFFFFFF;
        
        connects.add(ci);
        if(!m_mp(gamemode)) return DISC_PRIVATE;
        sendservinfo(ci);
        
        return DISC_NONE;
    }
    
    void cleanup_masterstate(clientinfo *);

    void clientdisconnect(int n,int reason) 
    {
        clientinfo *ci = (clientinfo *)getinfo(n);

        const char * disc_reason_msg = "normal";
        if(reason != DISC_NONE || ci->disconnect_reason.length())
        {
            disc_reason_msg = (ci->disconnect_reason.length() ? ci->disconnect_reason.c_str() : disconnect_reason(reason));
            defformatstring(discmsg)("client (%s) disconnected because: %s\n", ci->hostname(), disc_reason_msg);
            printf("%s",discmsg);
            sendservmsg(discmsg);
        }
        else
        {
            defformatstring(discmsg)("disconnected client (%s)",ci->hostname());
            puts(discmsg);
        }
        
        if(ci->connected)
        {
            if(ci->privilege)
            {
                if(currentmaster == n) setmaster(ci, false);
                else cleanup_masterstate(ci);
            }
            
            if(smode) smode->leavegame(ci, true);
            
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
            ci->state.disconnecttime = totalmillis;
            
            savescore(ci);
            
            sendf(-1, 1, "ri2", N_CDIS, n);
            
            clients.removeobj(ci);
            aiman::removeai(ci);
            
            maxclients -= reservedslots_use > 0;
            reservedslots_use -= reservedslots_use > 0;
            reservedslots += ci->using_reservedslot;
            
            signal_disconnect(n, disc_reason_msg);
            
            if(clients.empty()) noclients();
            else aiman::dorefresh = true;
        }
        else
        {
            signal_failedconnect(ci->hostname(), disc_reason_msg);
            connects.removeobj(ci);
        }
    }

    int reserveclients() { return 3; }

    int allowconnect(clientinfo *ci, const char *pwd)
    {
        if(ci->local) return DISC_NONE;
        if(!m_mp(gamemode)) return DISC_PRIVATE;
        int clientcount = numclients(-1, false, true);

        bool is_reserved = slotpass[0] && checkpassword(ci, slotpass, pwd);
        
        if(signal_connecting(ci->clientnum, ci->hostname(), ci->name, pwd, is_reserved) == -1)
        {
            return DISC_IPBAN;
        }
        
        bool is_admin = ci->privilege == PRIV_ADMIN;
        
        
        if((is_admin || is_reserved) && reservedslots > 0) 
        {
            if(clientcount >= maxclients)
            {
                reservedslots_use++;
                maxclients++;
            }
            reservedslots--;
            ci->using_reservedslot = true;
            return DISC_NONE;
        }
        
        if(clientcount >= maxclients) return DISC_MAXCLIENTS;
        
        if(serverpass[0])
        {
            if(!checkpassword(ci, serverpass, pwd)) return DISC_PRIVATE;
            return DISC_NONE;
        }
        
        uint ip = getclientip(ci->clientnum);
        
        if(mastermode>=MM_PRIVATE && allowedips.find(ip)<0) return DISC_PRIVATE;
        
        return DISC_NONE;
    }

    bool allowbroadcast(int n)
    {
        clientinfo *ci = getinfo(n);
        return ci && ci->connected && !ci->is_delayed_spectator();
    }
    
    void tryauth(clientinfo *ci, const char * domain, const char * user)
    {
        signal_authreq(ci->clientnum, user, domain);
    }

    void answerchallenge(clientinfo *ci, uint id, char *val)
    {
        for(char *s = val; *s; s++)
        {
            if(!isxdigit(*s)) { *s = '\0'; break; }
        }
        signal_authrep(ci->clientnum, id, val);
    }

    void receivefile(int sender, uchar *data, int len)
    {
        if(!m_edit || len > 1024*1024) return;
        clientinfo *ci = getinfo(sender);
        if(ci->state.state==CS_SPECTATOR && !ci->privilege && !ci->local) return;
        if(mapdata) DELETEP(mapdata);
        if(!len) return;
        mapdata = opentempfile("mapdata", "w+b");
        if(!mapdata) { sendf(sender, 1, "ris", N_SERVMSG, "failed to open temporary file for map"); return; }
        mapdata->write(data, len);
        defformatstring(msg)("[%s uploaded map to server, \"/getmap\" to receive it]", colorname(ci));
        sendservmsg(msg);
    }

    void sendclipboard(clientinfo *ci)
    {
        if(!ci->lastclipboard || !ci->clipboard) return;
        bool flushed = false;
        loopv(clients)
        {
            clientinfo &e = *clients[i];
            if(e.clientnum != ci->clientnum && e.needclipboard >= ci->lastclipboard) 
            {
                if(!flushed) { flushserver(true); flushed = true; }
                sendpacket(e.clientnum, 1, ci->clipboard);
            }
        }
    }

    void setspectator(clientinfo * spinfo, bool val)
    {
        if(!spinfo || (spinfo->state.state==CS_SPECTATOR ? val : !val) || spinfo->state.aitype != AI_NONE) return;
        
        if(spinfo->state.state!=CS_SPECTATOR && val)
        {
            if(smode) smode->leavegame(spinfo);
            spinfo->state.state = CS_SPECTATOR;
            spinfo->specmillis = totalmillis;
            spinfo->state.timeplayed += lastmillis - spinfo->state.lasttimeplayed;
            if(!spinfo->local && !spinfo->privilege) aiman::removeai(spinfo);
        }
        else if(spinfo->state.state==CS_SPECTATOR && !val)
        {
            spinfo->state.state = CS_DEAD;
            spinfo->state.respawn();
            spinfo->state.lasttimeplayed = lastmillis;
            aiman::addclient(spinfo);
        }
        sendf(-1, 1, "ri3", N_SPECTATOR, spinfo->clientnum, val);
        
        if(spectator_delay)
        {
            sendf(spinfo->clientnum, 1, "ri3", N_SPECTATOR, spinfo->clientnum, val);
            sendf(spinfo->clientnum, 1, "ri2", N_TIMEUP, (gamelimit - gamemillis + spectator_delay)/1000);
        }
        
        signal_spectator(spinfo->clientnum, val);
    }

    void parsepacket(int sender, int chan, packetbuf &p)     // has to parse exactly each byte of the packet
    {
        timer parsepacket_time;
        
        if(sender<0) return;
        char text[MAXTRANS];
        int type;
        clientinfo *ci = sender>=0 ? getinfo(sender) : NULL, *cq = ci, *cm = ci;
        if(ci && !ci->connected)
        {
            if(chan==0) return;
            else if(chan!=1 || getint(p)!=N_CONNECT) { disconnect_client(sender, DISC_TAGT); return; }
            else
            {
                getstring(text, p);
                filtertext(text, text, false, MAXNAMELEN);
                if(!text[0]) copystring(text, "unnamed");
                copystring(ci->name, text, MAXNAMELEN+1);

                getstring(text, p);
                int disc = allowconnect(ci, text);
                if(disc)
                {
                    disconnect_client(sender, disc);
                    return;
                }

                ci->playermodel = getint(p);
                ci->playerid = get_player_id(ci->name, getclientip(ci->clientnum));
                
                if(m_demo) enddemoplayback();
                
                connects.removeobj(ci);
                clients.add(ci);

                ci->connected = true;
                ci->needclipboard = totalmillis;
                bool restoredscore = restorescore(ci);
                bool was_playing = restoredscore && ci->state.state != CS_SPECTATOR && ci->state.disconnecttime > mastermode_mtime;
                
                if(mastermode>=MM_LOCKED && !was_playing) 
                {
                    ci->state.state = CS_SPECTATOR;
                    ci->specmillis = totalmillis;
                }
                else ci->state.state = CS_DEAD;
                
                if(currentmaster>=0) masterupdate = true; //FIXME send N_CURRENTMASTER packet directly to client
                ci->state.lasttimeplayed = lastmillis;

                const char *worst = m_teammode ? chooseworstteam(text, ci) : NULL;
                copystring(ci->team, worst ? worst : "good", MAXTEAMLEN+1);
                
                if(clients.length() == 1 && mapreload) selectnextgame();
                
                sendwelcome(ci);
                if(restoredscore) sendresume(ci);
                sendinitclient(ci);

                aiman::addclient(ci);

                if(m_demo) setupdemoplayback();
                
                signal_connect(ci->clientnum);
            }
        }
        else if(chan==2)
        {
            receivefile(sender, p.buf, p.maxlen);
            return;
        }

        if(p.packet->flags&ENET_PACKET_FLAG_RELIABLE) reliablemessages = true;
        #define QUEUE_AI clientinfo *cm = cq;
        #define QUEUE_MSG { if(cm && (!cm->local || demorecord || hasnonlocalclients())) while(curmsg<p.length()) cm->messages.add(p.buf[curmsg++]); }
        #define QUEUE_BUF(body) { \
            if(cm && (!cm->local || demorecord || hasnonlocalclients())) \
            { \
                curmsg = p.length(); \
                { body; } \
            } \
        }
        #define QUEUE_INT(n) QUEUE_BUF(putint(cm->messages, n))
        #define QUEUE_UINT(n) QUEUE_BUF(putuint(cm->messages, n))
        #define QUEUE_STR(text) QUEUE_BUF(sendstring(text, cm->messages))
        int curmsg;
        while((curmsg = p.length()) < p.maxlen) switch(type = checktype(getint(p), ci))
        {
            case N_POS:
            {
                int pcn = getuint(p); 
                p.get(); 
                uint flags = getuint(p);
                clientinfo *cp = getinfo(pcn);
                if(cp && pcn != sender && cp->ownernum != sender) cp = NULL;
                vec pos;
                loopk(3)
                {
                    int n = p.get(); n |= p.get()<<8; if(flags&(1<<k)) { n |= p.get()<<16; if(n&0x800000) n |= -1<<24; }
                    pos[k] = n/DMF;
                }
                loopk(3) p.get();
                int mag = p.get(); if(flags&(1<<3)) mag |= p.get()<<8;
                int dir = p.get(); dir |= p.get()<<8;
                vec vel = vec((dir%360)*RAD, (clamp(dir/360, 0, 180)-90)*RAD).mul(mag/DVELF);
                if(flags&(1<<4))
                {
                    p.get(); if(flags&(1<<5)) p.get();
                    if(flags&(1<<6)) loopk(2) p.get();
                }
                if(cp)
                {
                    if((!ci->local || demorecord || hasnonlocalclients()) && (cp->state.state==CS_ALIVE || cp->state.state==CS_EDITING))
                    {
                        if(!ci->local && !m_edit && max(vel.magnitude2(), (float)fabs(vel.z)) >= 180)
                            cp->setexceeded();
                        cp->position.setsize(0);
                        while(curmsg<p.length()) cp->position.add(p.buf[curmsg++]);
                    }
                    if(smode && cp->state.state==CS_ALIVE) smode->moved(cp, cp->state.o, cp->gameclip, pos, (flags&0x80)!=0);
                    cp->state.o = pos;
                    cp->gameclip = (flags&0x80)!=0;
                    
                    if(cp->state.state==CS_ALIVE && !cp->maploaded && cp->state.aitype == AI_NONE)
                    {
                        cp->lastposupdate = totalmillis - 30;
                        cp->maploaded = true;
                        signal_maploaded(cp->clientnum);
                    }
                    
                    if(cp->maploaded)
                    {
                        cp->lag = (std::max(30,cp->lag)*10 + (totalmillis - cp->lastposupdate))/12;
                        cp->lastposupdate = totalmillis;
                    }
                }
                break;
            }

            case N_TELEPORT:
            {
                int pcn = getint(p), teleport = getint(p), teledest = getint(p);
                clientinfo *cp = getinfo(pcn);
                if(cp && pcn != sender && cp->ownernum != sender) cp = NULL;
                if(cp && (!ci->local || demorecord || hasnonlocalclients()) && (cp->state.state==CS_ALIVE || cp->state.state==CS_EDITING))
                {
                    flushclientposition(*cp);
                    sendf(-1, 0, "ri4x", N_TELEPORT, pcn, teleport, teledest, cp->ownernum); 
                }
                break;
            }

            case N_JUMPPAD:
            {
                int pcn = getint(p), jumppad = getint(p);
                clientinfo *cp = getinfo(pcn);
                if(cp && pcn != sender && cp->ownernum != sender) cp = NULL;
                if(cp && (!ci->local || demorecord || hasnonlocalclients()) && (cp->state.state==CS_ALIVE || cp->state.state==CS_EDITING))
                {
                    cp->setpushed();
                    flushclientposition(*cp);
                    sendf(-1, 0, "ri3x", N_JUMPPAD, pcn, jumppad, cp->ownernum);
                }
                break;
            }
            
            case N_FROMAI:
            {
                int qcn = getint(p);
                if(qcn < 0) cq = ci;
                else
                {
                    cq = getinfo(qcn);
                    if(cq && qcn != sender && cq->ownernum != sender) cq = NULL;
                }
                break;
            }
            
            case N_EDITMODE:
            {
                int val = getint(p);
                if(!ci->local && !m_edit) break;
                if(val ? ci->state.state!=CS_ALIVE && ci->state.state!=CS_DEAD : ci->state.state!=CS_EDITING) break;
                if(smode)
                {
                    if(val) smode->leavegame(ci);
                    else smode->entergame(ci);
                }
                if(val)
                {
                    ci->state.editstate = ci->state.state;
                    ci->state.state = CS_EDITING;
                    ci->events.setsize(0);
                    ci->state.rockets.reset();
                    ci->state.grenades.reset();
                }
                else ci->state.state = ci->state.editstate;
                QUEUE_MSG;
                break;
            }

            case N_MAPCRC:
            {
                getstring(text, p);
                int crc = getint(p);
                if(!ci) break;
                if(strcmp(text, smapname))
                {
                    if(ci->clientmap[0])
                    {
                        ci->clientmap[0] = '\0';
                        ci->mapcrc = 0;
                    }
                    else if(ci->mapcrc > 0) ci->mapcrc = 0;
                    break;
                }
                copystring(ci->clientmap, text);
                ci->mapcrc = text[0] ? crc : 1;
                signal_mapcrc(ci->clientnum, text, crc);
                break;
            }

            case N_CHECKMAPS:
                signal_checkmaps(sender);
                break;

            case N_TRYSPAWN:
            #if 0
                if(!ci || !cq || cq->state.state!=CS_DEAD || cq->state.lastspawn>=0 || (smode && !smode->canspawn(cq))) break;
                if(!ci->clientmap[0] && !ci->mapcrc) 
                {
                    ci->mapcrc = -1;
                    checkmaps();
                }
                if(cq->state.lastdeath)
                {
                    flushevents(cq, cq->state.lastdeath + DEATHMILLIS);
                    cq->state.respawn();
                }
                cleartimedevents(cq);
                sendspawn(cq);
           #endif
                try_respawn(ci, cq);
                break;

            case N_GUNSELECT:
            {
                int gunselect = getint(p);
                if(!cq || cq->state.state!=CS_ALIVE || gunselect<GUN_FIST || gunselect>GUN_PISTOL) break;
                cq->state.gunselect = gunselect;
                QUEUE_AI;
                QUEUE_MSG;
                break;
            }

            case N_SPAWN:
            {
                int ls = getint(p), gunselect = getint(p);
                if(!cq || (cq->state.state!=CS_ALIVE && cq->state.state!=CS_DEAD) || ls!=cq->state.lifesequence || cq->state.lastspawn<0) break;
                if(cq->mapcrc == 0 && cq->state.aitype == AI_NONE)
                {
                    cq->mapcrc = 1;
                    signal_mapcrc(cq->clientnum, smapname, cq->mapcrc);
                }
                cq->state.lastspawn = -1;
                cq->state.state = CS_ALIVE;
                cq->state.gunselect = gunselect;
                cq->exceeded = 0;
                if(smode) smode->spawned(cq);
                QUEUE_AI;
                QUEUE_BUF({
                    putint(cm->messages, N_SPAWN);
                    sendstate(cq->state, cm->messages);
                });
                signal_spawn(cq->clientnum);
                break;
            }
            

            case N_SUICIDE:
            {
                if(cq) cq->addevent(new suicideevent);
                break;
            }

            case N_SHOOT:
            {
                shotevent *shot = new shotevent;
                shot->id = getint(p);
                shot->millis = cq ? cq->geteventmillis(gamemillis, shot->id) : 0;
                shot->gun = getint(p);
                loopk(3) shot->from[k] = getint(p)/DMF;
                loopk(3) shot->to[k] = getint(p)/DMF;
                int hits = getint(p);
                loopk(hits)
                {
                    if(p.overread()) break;
                    hitinfo &hit = shot->hits.add();
                    hit.target = getint(p);
                    hit.lifesequence = getint(p);
                    hit.dist = getint(p)/DMF;
                    hit.rays = getint(p);
                    loopk(3) hit.dir[k] = getint(p)/DNF;
                }
                if(cq) 
                {
                    cq->addevent(shot);
                    cq->setpushed();
                }
                else delete shot;
                break;
            }

            case N_EXPLODE:
            {
                explodeevent *exp = new explodeevent;
                int cmillis = getint(p);
                exp->millis = cq ? cq->geteventmillis(gamemillis, cmillis) : 0;
                exp->gun = getint(p);
                exp->id = getint(p);
                int hits = getint(p);
                loopk(hits)
                {
                    if(p.overread()) break;
                    hitinfo &hit = exp->hits.add();
                    hit.target = getint(p);
                    hit.lifesequence = getint(p);
                    hit.dist = getint(p)/DMF;
                    hit.rays = getint(p);
                    loopk(3) hit.dir[k] = getint(p)/DNF;
                }
                if(cq) cq->addevent(exp);
                else delete exp;
                break;
            }


            case N_ITEMPICKUP:
            {
                int n = getint(p);
                if(!cq) break;
                pickupevent *pickup = new pickupevent;
                pickup->ent = n;
                cq->addevent(pickup);
                break;
            }

            case N_TEXT:
            {
                getstring(text, p);
                filtertext(text, text);
                
                if(ci && (ci->privilege == PRIV_ADMIN || !ci->check_flooding(ci->sv_text_hit,"sending text")))
                {
                    if(text[0] == '#' && !strcmp(text+1, "reload") && ci->privilege == PRIV_ADMIN)
                    {
                        reload_hopmod();
                        break;
                    }
                    
                    if(signal_text(ci->clientnum, text) != -1 && !ci->is_delayed_spectator())
                    {
                        QUEUE_AI;
                        QUEUE_INT(N_TEXT);
                        QUEUE_STR(text);
                    }
                }
                break;
            }
            
            case N_SAYTEAM:
            {
                getstring(text, p);
                if(!ci || !cq || (ci->state.state==CS_SPECTATOR && !ci->privilege) || !m_teammode || !cq->team[0] || ci->check_flooding(ci->sv_sayteam_hit,"sending text")) break;
                if(signal_sayteam(ci->clientnum,text)!=-1)
                {
                    loopv(clients)
                    {
                        clientinfo *t = clients[i];
                        if(t==cq || t->state.state==CS_SPECTATOR || t->state.aitype != AI_NONE || strcmp(cq->team, t->team)) continue;
                        sendf(t->clientnum, 1, "riis", N_SAYTEAM, ci->clientnum, text);
                    }
                }
                break;
            }

            case N_SWITCHNAME:
            {
                getstring(text, p);
                filtertext(text, text, false, MAXNAMELEN);
                if(!text[0]) copystring(text, "unnamed");
		        
                string oldname;
                copystring(oldname, ci->name);
                
                bool allow_rename = strcmp(ci->name, text) && signal_allow_rename(ci->clientnum, text) != -1;
                
                if(allow_rename)
                {
                    copystring(ci->name, text);
                    
                    int futureId = get_player_id(text, getclientip(ci->clientnum));
                    signal_renaming(ci->clientnum, futureId);
                    ci->playerid = futureId;
                    
                    signal_rename(ci->clientnum, oldname, ci->name);
                    
                    QUEUE_INT(N_SWITCHNAME);
                    QUEUE_STR(ci->name);
                }
                else 
                {
                    if (strcmp(ci->name, text))
                        player_rename(ci->clientnum, oldname, false);
                }
                
                break;
            }


            case N_SWITCHMODEL:
            {
                ci->playermodel = getint(p);
                QUEUE_MSG;
                break;
            }

            case N_SWITCHTEAM:
            {
                getstring(text, p);
                filtertext(text, text, false, MAXTEAMLEN);
                if(strcmp(ci->team, text) && m_teammode)
                {
                    bool cancel = ci->check_flooding(ci->sv_switchteam_hit,"switching teams") || 
                        (smode && !smode->canchangeteam(ci, ci->team, text)) ||
                        signal_chteamrequest(ci->clientnum, ci->team, text) == -1;
                    
                    if(!cancel)
                    {
                        if(ci->state.state==CS_ALIVE) suicide(ci);
                        string oldteam;
                        copystring(oldteam, ci->team);
                        copystring(ci->team, text);
                        aiman::changeteam(ci);
                        sendf(-1, 1, "riisi", N_SETTEAM, sender, ci->team, ci->state.state==CS_SPECTATOR ? -1 : 0);
                        signal_reteam(ci->clientnum, oldteam, text);
                    }
                    else sendf(-1, 1, "riisi", N_SETTEAM, sender, ci->team, ci->state.state==CS_SPECTATOR ? -1 : 0);
                }
                break;
            }

            case N_MAPVOTE:
            {
                getstring(text, p);
                filtertext(text, text);
                int reqmode = getint(p);
                if(!ci->local && !m_mp(reqmode)) reqmode = 0;
                if(!ci->check_flooding(ci->sv_mapvote_hit,"map voting") &&
                    signal_mapvote(ci->clientnum, text, modename(reqmode,"unknown")) != -1) vote(text, reqmode, sender);
                break;
            }
            
            case N_MAPCHANGE:
            {
                getstring(text, p);
                filtertext(text, text, false);
                int reqmode = getint(p);
                if(type!=N_MAPVOTE && !mapreload) break;
                if(!ci->local && !m_mp(reqmode)) reqmode = 0;
                vote(text, reqmode, sender);
                break;
            }

            case N_ITEMLIST:
            {
                if((ci->state.state==CS_SPECTATOR && !ci->privilege && !ci->local) || !notgotitems || strcmp(ci->clientmap, smapname)) { while(getint(p)>=0 && !p.overread()) getint(p); break; }
                int n;
                while((n = getint(p))>=0 && n<MAXENTS && !p.overread())
                {
                    server_entity se = { NOTUSED, 0, false };
                    while(sents.length()<=n) sents.add(se);
                    sents[n].type = getint(p);
                    sents_type_index[sents[n].type] = n;
                    if(canspawnitem(sents[n].type))
                    {
                        if(m_mp(gamemode) && delayspawn(sents[n].type)) sents[n].spawntime = spawntime(sents[n].type);
                        else sents[n].spawned = true;
                    }
                }
                notgotitems = false;
                break;
            }

            case N_EDITENT:
            {
                int i = getint(p);
                loopk(3) getint(p);
                int type = getint(p);
                loopk(5) getint(p);
                if(!ci || ci->state.state==CS_SPECTATOR) break;
                QUEUE_MSG;
                bool canspawn = canspawnitem(type);
                if(i<MAXENTS && (sents.inrange(i) || canspawnitem(type)))
                {
                    server_entity se = { NOTUSED, 0, false };
                    while(sents.length()<=i) sents.add(se);
                    sents[i].type = type;
                    if(canspawn ? !sents[i].spawned : (sents[i].spawned || sents[i].spawntime))
                    {
                        sents[i].spawntime = canspawn ? 1 : 0;
                        sents[i].spawned = false;
                    }
                }
                break;
            }

            case N_EDITVAR:
            {
                int type = getint(p);
                getstring(text, p);
                switch(type)
                {
                    case ID_VAR: getint(p); break;
                    case ID_FVAR: getfloat(p); break;
                    case ID_SVAR: getstring(text, p);
                }
                if(ci && ci->state.state!=CS_SPECTATOR) QUEUE_MSG;
                break;
            }

            case N_PING:
	    {
                if(!ci->maploaded && totalmillis - ci->connectmillis > 2000)
                {
                    ci->maploaded = true;
                    signal_maploaded(ci->clientnum);
                }
                if(ci) ci->lastpingupdate = totalmillis; 
                sendf(sender, 1, "i2", N_PONG, getint(p));
                break;
            }

            case N_CLIENTPING:
            {
                int ping = getint(p);
                if(ci) 
                {
                    ci->ping = ping;
                    loopv(ci->bots) ci->bots[i]->ping = ping;
                }
                QUEUE_MSG;
                break;
            }

            case N_MASTERMODE:
            {
                update_mastermask();
                
                int mm = getint(p);
                if((ci->privilege || ci->local) && mm>=MM_OPEN && mm<=MM_PRIVATE)
                {
                    if((ci->privilege>=PRIV_ADMIN || ci->local) || (mastermask&(1<<mm)))
                    {
                        if(signal_setmastermode_request(ci->clientnum, mastermodename(mastermode), mastermodename(mm)) == -1) break;
                        signal_setmastermode(ci->clientnum, mastermodename(mastermode), mastermodename(mm));
                        mastermode = mm;
                        mastermode_owner = ci->clientnum;
                        mastermode_mtime = totalmillis;
                        allowedips.shrink(0);
                        if(mm>=MM_PRIVATE)
                        {
                            loopv(clients) allowedips.add(getclientip(clients[i]->clientnum));
                        }
                        sendf(-1, 1, "rii", N_MASTERMODE, mastermode);
                        //defformatstring(s)("mastermode is now %s (%d)", mastermodename(mastermode), mastermode);
                        //sendservmsg(s);
                    }
                    else
                    {
                        defformatstring(s)("mastermode %d is disabled on this server", mm);
                        sendf(sender, 1, "ris", N_SERVMSG, s);
                    }
                }
                break;
            }

            case N_CLEARBANS:
            {
                if(ci->privilege) signal_clearbans_request();
                break;
            }

            case N_KICK:
            {
                int victim = getint(p);
                if(ci->privilege && ci->clientnum != victim && getclientinfo(victim))
                {
                    if(ci->privilege < PRIV_ADMIN && ci->check_flooding(ci->sv_kick_hit, "kicking")) break;
                    signal_kick_request(ci->clientnum, ci->name, 14400, victim, "");
                }
                break;
            }

            case N_SPECTATOR:
            {
                int spectator = getint(p), val = getint(p);
                bool self = spectator == sender;
                if(!ci->privilege && (!self || (ci->state.state==CS_SPECTATOR && mastermode>=MM_LOCKED) || (ci->state.state == CS_SPECTATOR && !val && !ci->allow_self_unspec) || ci->check_flooding(ci->sv_spec_hit, "switching"))) break;
                clientinfo *spinfo = (clientinfo *)getclientinfo(spectator); // no bots
                if(!spinfo) break;
                if(val && spinfo != ci && spinfo->privilege && ci->privilege < PRIV_ADMIN)
                {
                    ci->sendprivtext(RED "You cannot spec that player because they have an above normal privilege.");
                    break;
                }
                //spinfo->allow_self_unspec = self && spinfo->state.state!=CS_SPECTATOR && val;
                spinfo->allow_self_unspec = self && val;
                setspectator(spinfo, val);
                break;
            }

            case N_SETTEAM:
            {
                int who = getint(p);
                getstring(text, p);
                filtertext(text, text, false, MAXTEAMLEN);
                if(!ci->privilege && !ci->local) break;
                clientinfo *wi = getinfo(who);
                if(!wi || !strcmp(wi->team, text)) break;
                if((!smode || smode->canchangeteam(wi, wi->team, text)) && 
                    signal_chteamrequest(wi->clientnum,wi->team,text) != -1)
                {
                    if(smode && wi->state.state==CS_ALIVE) suicide(wi);
                    signal_reteam(wi->clientnum, wi->team, text);
                    copystring(wi->team, text, MAXTEAMLEN+1);
                }
                aiman::changeteam(wi);
                sendf(-1, 1, "riisi", N_SETTEAM, who, wi->team, 1);
                break;
            }

            case N_FORCEINTERMISSION:
                if(ci->local && !hasnonlocalclients()) startintermission();
                break;

            case N_RECORDDEMO:
            {
                int val = getint(p);
                if(ci->privilege<PRIV_ADMIN && !ci->local) break;
                demonextmatch = val!=0;
                defformatstring(msg)("demo recording is %s for next match", demonextmatch ? "enabled" : "disabled");
                sendservmsg(msg);
                break;
            }

            case N_STOPDEMO:
            {
                if(ci->privilege<PRIV_ADMIN && !ci->local) break;
                stopdemo();
                break;
            }

            case N_CLEARDEMOS:
            {
                int demo = getint(p);
                if(ci->privilege<PRIV_ADMIN && !ci->local) break;
                cleardemos(demo);
                break;
            }

            case N_LISTDEMOS:
                if(!ci->privilege && !ci->local && ci->state.state==CS_SPECTATOR) break;
                listdemos(sender);
                break;

            case N_GETDEMO:
            {
                int n = getint(p);
                if(!ci->privilege && !ci->local && ci->state.state==CS_SPECTATOR) break;
                senddemo(ci, n);
                break;
            }

            case N_GETMAP:
                if(!mapdata) sendf(sender, 1, "ris", N_SERVMSG, "no map to send");
                else if(ci->getmap) sendf(sender, 1, "ris", N_SERVMSG, "already sending map");
                else
                {
                    sendf(sender, 1, "ris", N_SERVMSG, "server sending map...");
                    if((ci->getmap = sendfile(sender, 2, mapdata, "ri", N_SENDMAP)))
                        ci->getmap->freeCallback = freegetmap;
                    ci->needclipboard = totalmillis;
                }
                break;

            case N_NEWMAP:
            {
                int size = getint(p);
                if(!ci->privilege && !ci->local && ci->state.state==CS_SPECTATOR) break;
                if(ci->check_flooding(ci->sv_newmap_hit, "newmapping")) break;
                if(size>=0)
                {
                    smapname[0] = '\0';
                    resetitems();
                    notgotitems = false;
                    if(smode) smode->reset(true);
                }
                QUEUE_MSG;
                break;
            }

            case N_SETMASTER:
            {
                int val = getint(p);
                getstring(text, p);
                setmaster(ci, val!=0, text);
                // don't broadcast the master password
                break;
            }

            case N_ADDBOT:
            {
                aiman::reqadd(ci, getint(p));
                break;
            }

            case N_DELBOT:
            {
                aiman::reqdel(ci);
                break;
            }

            case N_BOTLIMIT:
            {
                int limit = getint(p);
                if(ci) aiman::setbotlimit(ci, limit);
                break;
            }
            
            case N_BOTBALANCE:
            {
                int balance = getint(p);
                if(ci) aiman::setbotbalance(ci, balance!=0);
                break;
            }

            case N_AUTHTRY:
            {
                string desc, name;
                getstring(desc, p, sizeof(desc));
                getstring(name, p, sizeof(name));
                tryauth(ci, desc, name);
                break;
            }

            case N_AUTHANS:
            {
                string desc, ans;
                getstring(desc, p, sizeof(desc)); // unused for now
                uint id = (uint)getint(p);
                getstring(ans, p, sizeof(ans));
                answerchallenge(ci, id, ans);
                break;
            }

            case N_PAUSEGAME:
            {
                int val = getint(p);
                if(ci->privilege<PRIV_ADMIN && !ci->local) break;
                pausegame(val > 0);
                pausegame_owner = ci->clientnum;
                break;
            }

            case N_COPY:
                ci->cleanclipboard();
                ci->lastclipboard = totalmillis;
                goto genericmsg;

            case N_PASTE:
                if(ci->state.state!=CS_SPECTATOR) sendclipboard(ci);
                goto genericmsg;

            case N_CLIPBOARD:
            {
                int unpacklen = getint(p), packlen = getint(p); 
                ci->cleanclipboard(false);
                if(ci->state.state==CS_SPECTATOR)
                {
                    if(packlen > 0) p.subbuf(packlen);
                    break;
                }
                if(packlen <= 0 || packlen > (1<<16) || unpacklen <= 0) 
                {
                    if(packlen > 0) p.subbuf(packlen);
                    packlen = unpacklen = 0;
                }
                packetbuf q(32 + packlen, ENET_PACKET_FLAG_RELIABLE);
                putint(q, N_CLIPBOARD);
                putint(q, ci->clientnum);
                putint(q, unpacklen);
                putint(q, packlen); 
                if(packlen > 0) p.get(q.subbuf(packlen).buf, packlen);
                ci->clipboard = q.finalize();
                ci->clipboard->referenceCount++;
                break;
            } 

            #define PARSEMESSAGES 1
            #include "capture.h"
            #include "ctf.h"
            #undef PARSEMESSAGES
            
            case -1:
                disconnect_client(sender, DISC_TAGT);
                return;
            
            case -2:
                disconnect_client(sender, DISC_OVERFLOW);
                return;
            
            default: genericmsg:
            {
                int size = server::msgsizelookup(type);
                if(size<=0) { disconnect_client(sender, DISC_TAGT); return; }
                loopi(size-1) getint(p);
                if(ci && cq && (ci != cq || ci->state.state!=CS_SPECTATOR)) { QUEUE_AI; QUEUE_MSG; }
                break;
            }
        }
        
        timer::time_diff_t e = parsepacket_time.usec_elapsed();
        if(e >= timer_alarm_threshold) std::cout<<"parsepacket timing: "<<e<<" usecs."<<std::endl;
    }
    
    int laninfoport() { return SAUERBRATEN_LANINFO_PORT; }
    int serverinfoport(int servport) { return servport < 0 ? SAUERBRATEN_SERVINFO_PORT : servport+1; }
    int serverport(int infoport) { return infoport < 0 ? SAUERBRATEN_SERVER_PORT : infoport-1; }
    const char *defaultmaster() { return "sauerbraten.org"; } 
    int masterport() { return SAUERBRATEN_MASTER_PORT; } 
    int numchannels() { return 3; }

    #include "extinfo.h"

    void serverinforeply(ucharbuf &req, ucharbuf &p)
    {
        if(!getint(req))
        {
            if(!enable_extinfo) return;
            extserverinforeply(req, p);
            return;
        }
        
        int clients = numclients(-1, false, true);
        
        putint(p, clients);
        putint(p, 5);                   // number of attrs following
        putint(p, PROTOCOL_VERSION);    // a // generic attributes, passed back below
        putint(p, gamemode);            // b
        putint(p, max((gamelimit - gamemillis)/1000, 0));
        putint(p, (clients <= maxclients ? maxclients : clients));
        putint(p, serverpass[0] ? MM_PASSWORD : (!m_mp(gamemode) ? MM_PRIVATE : (mastermode || display_open ? mastermode : MM_AUTH) ));
        sendstring(smapname, p);
        sendstring(serverdesc, p);
        sendserverinforeply(p);
    }

    bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np)
    {
        return attr.length() && attr[0]==PROTOCOL_VERSION;
    }
    
    #define INCLUDE_EXTSERVER_CPP
    #include "extserver.cpp"
    #undef INCLUDE_EXTSERVER_CPP
    
    #include "aiman.h"
} //namespace server
