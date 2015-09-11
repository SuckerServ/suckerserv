#ifdef BOOST_BUILD_PCH_ENABLED
#include "hopmod/pch.hpp"
#endif

#include "hopmod/hopmod.hpp"
#include "game.h"

#include "hopmod/server_functions.hpp"
lua::event_environment & event_listeners();

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

namespace game
{         
    void parseoptions(vector<const char *> &args)
    {   
        loopv(args)
            if(!server::serveroption(args[i]))
                printf("unknown command-line option: %s", args[i]);
    }
    const char *gameident() { return "fps"; }
}

extern ENetAddress masteraddress;


namespace server
{
    struct server_entity            // server side version of "entity" type
    {
        int type;
        int spawntime;
        bool spawned;
        int lastpickup;
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

    bool ctftkpenalty = true;
    
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
        vec o, cam;
        int state, editstate;
        int lastdeath, deadflush, lastspawn, lifesequence;
        int lastshot;
        projectilestate<8> rockets, grenades;
        int frags, flags, deaths, suicides, teamkills, shotdamage, damage, explosivedamage, tokens, hits, misses, shots;
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
            frags = flags = deaths = suicides = teamkills = shotdamage = explosivedamage = damage = hits = misses = shots = tokens = 0;
            
            lastdeath = 0;

            respawn();
        }

        void respawn()
        {
            fpsstate::respawn();
            o = vec(-1e10f, -1e10f, -1e10f);
            deadflush = 0;
            lastspawn = -1;
            lastshot = 0;
            tokens = 0;
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
            gs.disconnecttime = disconnecttime;
        }
    };
    
    extern int gamemillis, nextexceeded;

	#define anticheat_class
    #include "anticheat.h"
	#undef anticheat_class

    struct clientinfo
    {
        int n;
        int clientnum, ownernum, connectmillis, specmillis, sessionid, overflow, playerid;
        string name, team, mapvote;
        int playermodel;
        int modevote;
        int privilege;
        bool hide_privilege;
        bool connected, local, timesync;
        int gameoffset, lastevent, pushed, exceeded;
        gamestate state;
        vector<gameevent *> events;
        vector<uchar> position, messages;
        uchar *wsdata;
        int wslen;
        vector<clientinfo *> bots;
        int ping, lastpingupdate, lastposupdate, lag, aireinit;
        string clientmap;
        int mapcrc;
        int no_spawn;
        bool warned, gameclip;
        ENetPacket *getdemo, *getmap, *clipboard;
        int lastclipboard, needclipboard;
        int connectauth;
        string authname, authdesc;
        int authkickvictim;
        char *authkickreason;

        int clientmillis;
        int timetrial;

        anticheat ac;

        bool spy;
        int last_lag;
        
        ullong n_text_millis;
        ullong n_sayteam_millis;
        ullong n_mapvote_millis;
        ullong n_switchname_millis;
        ullong n_switchteam_millis;
        ullong n_kick_millis;
        ullong n_remip_millis;
        ullong n_newmap_millis;
        ullong n_spec_millis;
        
        std::string disconnect_reason;
        int maploaded;
        int rank;
        bool using_reservedslot;
        bool allow_self_unspec;
        
        clientinfo():
            getdemo(NULL),
            getmap(NULL),
            clipboard(NULL),
            authkickreason(NULL),
            n_text_millis(0),
            n_sayteam_millis(0),
            n_mapvote_millis(0),
            n_switchname_millis(0),
            n_switchteam_millis(0),
            n_kick_millis(0),
            n_remip_millis(0),
            n_newmap_millis(0),
            n_spec_millis(0) 
            
        { reset(); }
        
        ~clientinfo() { events.deletecontents(); cleanclipboard(); }
        
        void addevent(gameevent *e)
        {
            if(state.state==CS_SPECTATOR || events.length()>100) delete e;
            else events.add(e);
        }

        enum
        {
            PUSHMILLIS = 3000
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
            modevote = INT_MAX;
            state.reset();
            events.deletecontents();
            overflow = 0;
            timesync = false;
            lastevent = 0;
            exceeded = 0;
            pushed = 0;
            maploaded = 0;
            clientmap[0] = '\0';
            mapcrc = 0;
            warned = false;
            gameclip = false;
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
        
        void cleanauth(bool full = true)
        {
            //authreq = 0;
            //if(authchallenge) { freechallenge(authchallenge); authchallenge = NULL; }
            if(full) cleanauthkick();
        }

        void cleanauthkick()
        {
            authkickvictim = -1;
            DELETEA(authkickreason);
        }
        
        void reset()
        {
            name[0] = team[0] = 0;
            playermodel = -1;
            privilege = PRIV_NONE;
            hide_privilege = false;
            connected = local = false;
            connectauth = 0;
            position.setsize(0);
            messages.setsize(0);
            ping = 0;
            lastpingupdate = 0;
            lastposupdate = 0;
            lag = 0;

            ac.reset();

            clientmillis = 0;
            last_lag = 0;
            spy = false;
            timetrial = 0;
            no_spawn = 0;

            aireinit = 0;
            using_reservedslot = false;
            needclipboard = 0;
            cleanclipboard();
            cleanauth();
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
        int uses, len;
        uchar *data;

        worldstate() : uses(0), len(0), data(NULL) {}

        void setup(int n) { len = n; data = new uchar[n]; }
        void cleanup() { DELETEA(data); len = 0; }
        bool contains(const uchar *p) const { return p >= data && p < &data[len]; }
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
    int gamemillis = 0, gamelimit = 0, nextexceeded = 0, next_timeupdate = 0, gamespeed = 100;
    bool gamepaused = false, shouldstep = true;
    int pausegame_owner = -1;
    bool reassignteams = true;
    
    bool display_open = false;
    bool allow_mm_veto = false;
    bool allow_mm_locked = false;
    bool allow_mm_private = false;
    bool allow_mm_private_reconnect = false;
    bool reset_mm = true;
    bool allow_item[11] = {true, true, true, true, true, true, true, true, true, true, true};
    
    string next_gamemode = "";
    string next_mapname = "";
    int next_gametime = -1;

    int reservedslots = 0;
    int reservedslots_use = 0;

    int intermtime = 10000;

    string serverdesc = "", serverpass = "", serverauth = "";
    string smapname = "";
    int interm = 0;
    bool mapreload = false;
    enet_uint32 lastsend = 0;
    int mastermode = MM_OPEN, mastermask = MM_PRIVSERV;
    string adminpass = "";
    string slotpass = "";
    stream *mapdata = NULL;
    
    bool restrictpausegame = false;
    bool restrictgamespeed = false;
    
    int hide_and_seek = 0; //MOD
    
    vector<uint> allowedips;
    
    vector<clientinfo *> connects, clients, bots;
    vector<worldstate> worldstates;
    
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
    #define MAXDEMOSIZE (16*1024*1024)
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
        if(n >= spycn)
        {
            extern vector<clientinfo *> spies;
            loopv(spies) if(spies[i]->clientnum == n) return spies[i];
        }
        if(n < MAXCLIENTS) return (clientinfo *)getclientinfo(n);
        n -= MAXCLIENTS;
        return bots.inrange(n) ? bots[n] : NULL;
    }
    
    clientinfo * get_ci(int cn)
    {
        clientinfo * ci = getinfo(cn);
        if(!ci) luaL_error(get_lua_state(), "invalid cn");
        return ci;
    }

    bool spec_slots = false;

    uint mcrc = 0;
    vector<entity> ments;
    vector<server_entity> sents;
    int jmpds[MAXENTTYPES];
    int tps[MAXENTTYPES];
    int sents_type_index[MAXENTTYPES];
    vector<savedscore> scores;

    #define anticheat_helper_func
    #include "anticheat.h"
    #undef anticheat_helper_func
    
    hashtable<uint, vector<int> > disc_record;

    extern void setspectator(clientinfo * spinfo, bool val, bool broadcast=true);
    
    #define MAXSPIES 5
    
    int spycn = rnd(INT_MAX - MAXCLIENTS - MAXBOTS - MAXSPIES) + 1;    
    vector<clientinfo *> spies;
    void real_cn(int &n) { n = spies[n-spycn]->n; }
    
    void sendresume(clientinfo *ci);
    void sendinitclient(clientinfo *ci);
    void sendservinfo(clientinfo *ci);
    bool restorescore(clientinfo *ci);
    
    void set_spy(int cn, bool val)
    {
        clientinfo *ci = getinfo(cn);
        if(!ci || ci->spy == val) return;

        if(val)
        {
            if(spies.length() >= MAXSPIES)
            {
                ci->sendprivtext(RED "You can't enter the spy-mode at this time, limit of maximum spies reached.");
                if(!ci->connected) disconnect_client(cn, DISC_MAXCLIENTS);
                return;
            }
            if(ci->messages.length() > 0) ci->messages.shrink(0);
            if(ci->connected)
            {
                setspectator(ci, val, false);
                sendf(-1, 1, "ri2", N_CDIS, cn);
                ci->sendprivtext(RED "You've entered the spy-mode.");
            }
            defformatstring(admin_info, RED "ADMIN-INFO: %s joined the spy-mode.", ci->name);
            loopv(clients) if(clients[i] != ci && clients[i]->privilege >= PRIV_ADMIN) clients[i]->sendprivtext(admin_info);
            ci->spy = true;
            ci->privilege = PRIV_ADMIN;
            ci->hide_privilege = true;
            ci->clientnum = spycn + spies.length();
            spies.add(ci);
            sendservinfo(ci);
        }
        else
        {
            if(ci->messages.length() > 0) ci->messages.shrink(0);
            ci->spy = false;
            ci->hide_privilege = false;
            spies.removeobj(ci);
            ci->clientnum = ci->n;
            sendservinfo(ci);
            sendinitclient(ci);
            if(restorescore(ci)) sendresume(ci);
            event_connect(event_listeners(), boost::make_tuple(ci->clientnum, ci->spy));
            ci->connectmillis = totalmillis;
            ci->sendprivtext(RED "You've left the spy-mode.");
            if(mastermode <= 1) setspectator(ci, 0);
            else sendf(-1, 1, "ri3", N_SPECTATOR, ci->clientnum, 1);
            sendf(-1, 1, "riisi", N_SETTEAM, cn, ci->team, -1);
            defformatstring(admin_info, RED "ADMIN-INFO: %s left the spy-mode.", ci->name);
            loopv(clients) if(clients[i] != ci && clients[i]->privilege >= PRIV_ADMIN) clients[i]->sendprivtext(admin_info);
        }
    }
    
    inline int spy_cn(clientinfo *ci)
    {
        if(ci && ci->spy) return ci->clientnum;
        return -1;
    }

    int spec_count()
    {
        int n = 0;
        loopv(clients) if (clients[i]->state.state == CS_SPECTATOR && !clients[i]->spy) n++;
        return n;
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
        virtual void cleanup() {}
        virtual void setup() {}
        virtual void newmap() {}
        virtual void intermission() {}
        virtual bool hidefrags() { return false; }
        virtual int getteamscore(const char *team) { return 0; }
        virtual void getteamscores(vector<teamscore> &scores) {}
        virtual bool extinfoteam(const char *team, ucharbuf &p) { return false; }
    };

    #define SERVMODE 1
    #include "capture.h"
    #include "ctf.h"
    #include "collect.h"

    captureservmode capturemode;
    ctfservmode ctfmode;
    collectservmode collectmode;
    servmode *smode = NULL;
    
    bool canspawnitem(int type) { return !m_noitems && (type>=I_SHELLS && type<=I_QUAD && (!m_noammo || type<I_SHELLS || type>I_CARTRIDGES)); }

    int numclients(int exclude, bool nospec, bool noai, bool priv);

    int spawntime(int type)
    {
        if(m_classicsp) return INT_MAX;
        int np = numclients(-1, true, false, false);
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
            case I_GREENARMOUR: sec = 20; break;
            case I_YELLOWARMOUR: sec = 30; break;
            case I_BOOST: sec = 60; break;
            case I_QUAD: sec = 70; break;
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

    int msgsizelookup(int msg)
    {
        static int sizetable[NUMMSG] = { -1 };
        if(sizetable[0] < 0)
        {
            memset(sizetable, -1, sizeof(sizetable));
            for(const int *p = msgsizes; *p >= 0; p += 2) sizetable[p[0]] = p[1];
        }
        return msg >= 0 && msg < NUMMSG ? sizetable[msg] : -1;
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
            case PRIV_ADMIN: return "\fs\f6admin\fr";
            case PRIV_AUTH: return "\fs\f1auth\fr";
            case PRIV_MASTER: return "\fs\f0master\fr";
            case PRIV_NONE: return "\fs\ffnone\fr";
            default: return "unknown";
        }
    }

    void sendservmsg(const char *s) { sendf(-1, 1, "ris", N_SERVMSG, s); }
    void sendservmsgf(const char *fmt, ...)
    {
        defvformatstring(s, fmt, fmt);
        sendf(-1, 1, "ris", N_SERVMSG, s);
    }

    void resetitems() 
    { 
        mcrc = 0;
        ments.setsize(0);
        sents.setsize(0);
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

    void cleanup_fpsgame(int shutdown_type)
    {
        aiman::deleteai();
        
        loopv(demos) delete[] demos[i].data;
        demos.shrink(0);
    }

    void serverinit()
    {
        smapname[0] = '\0';
        resetitems();
        
        init_hopmod();
    
        signal_shutdown.connect(cleanup_fpsgame);
    }
    
    int numclients(int exclude = -1, bool nospec = true, bool noai = true, bool priv = false)
    {
        int n = 0;
        
        loopv(clients) 
        {
            clientinfo *ci = clients[i];
            if(ci->clientnum!=exclude && (!nospec || ci->state.state!=CS_SPECTATOR || (priv && (ci->privilege || ci->local))) && (!noai || ci->state.aitype == AI_NONE)) n++;
        }
        
        n -= spies.length();
        
        return n;
    }
    
    bool duplicatename(clientinfo *ci, const char *name)
    {
        if(!name) name = ci->name;
        loopv(clients) if(clients[i]!=ci && (!strcmp(name, clients[i]->name) && !clients[i]->spy)) return true;
        return false;
    }

    const char *colorname(clientinfo *ci, const char *name = NULL)
    {
        if(!name) name = ci->name;
        if(name[0] && !duplicatename(ci, name) && ci->state.aitype == AI_NONE) return name;
        static string cname[3];
        static int cidx = 0;
        cidx = (cidx+1)%3;
        formatstring(cname[cidx], ci->state.aitype == AI_NONE ? "%s \fs\f5(%d)\fr" : "%s \fs\f5[%d]\fr", name, ci->clientnum);
        return cname[cidx];
    }

    bool pickup(int i, int sender)         // server side item pickup, acknowledge first client that gets it
    {
        if((m_timed && gamemillis>=gamelimit) || !sents.inrange(i) || !sents[i].spawned) return false;
        clientinfo *ci = getinfo(sender);
        if(!ci || (!ci->local && !ci->state.canpickup(sents[i].type))) return false;
        sents[i].spawned = false;
        sents[i].spawntime = spawntime(sents[i].type);
        sents[i].lastpickup = totalmillis;
        sendf(-1, 1, "ri3", N_ITEMACC, i, sender);
        ci->state.pickup(sents[i].type);
        return true;
    }
    
    static hashset<teaminfo> teaminfos;
    
    void clearteaminfo()
    {
        teaminfos.clear();
    }

    bool teamhasplayers(const char *team) { loopv(clients) if(!strcmp(clients[i]->team, team)) return true; return false; }
    
    bool pruneteaminfo()
    {
        int oldteams = teaminfos.numelems;
        enumerate(teaminfos, teaminfo, old,
            if(!old.frags && !teamhasplayers(old.team)) teaminfos.remove(old.team);
        );
        return teaminfos.numelems < oldteams;
    }
    
    teaminfo *addteaminfo(const char *team)
    {
        teaminfo *t = teaminfos.access(team);
        if(!t)
        {
            if(teaminfos.numelems >= MAXTEAMS && !pruneteaminfo()) return NULL;
            t = &teaminfos[team];
            copystring(t->team, team, sizeof(t->team));
            t->frags = 0;
        }
        return t;
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
        static const char * const teamnames[2] = {"good", "evil"};
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
            addteaminfo(teamnames[i]);
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

        int len = (int)min(demotmp->size(), stream::offset(MAXDEMOSIZE));
        if(demos.length()>=MAXDEMOS)
        {
            delete[] demos[0].data;
            demos.remove(0);
        }
        demofile &d = demos.add();
        time_t t = time(NULL);
        char *timestr = ctime(&t), *trim = timestr + strlen(timestr);
        while(trim>timestr && isspace(*--trim)) *trim = '\0';
        formatstring(d.info, "%s: %s, %s, %.2f%s", timestr, modename(gamemode), smapname, len > 1024*1024 ? len/(1024*1024.f) : len/1024.0f, len > 1024*1024 ? "MB" : "kB");
        sendservmsgf("demo \"%s\" recorded", d.info);
        d.data = new uchar[len];
        d.len = len;
        demotmp->seek(0, SEEK_SET);
        demotmp->read(d.data, len);
        DELETEP(demotmp);
        
        event_endrecord(event_listeners(), boost::make_tuple(demo_id, len));
    }

    int welcomepacket(packetbuf &p, clientinfo *ci);
    void sendwelcome(clientinfo *ci);

    void setupdemorecord(bool broadcast = true, const char * filename = NULL)
    {
        if(!m_mp(gamemode) || m_edit) return;
        
        string defaultfilename;
        
        if(!filename || filename[0]=='\0')
        {
            char ftime[32];
            ftime[0]='\0';
            time_t now = time(NULL);
            strftime(ftime,sizeof(ftime),"%0e%b%Y_%H:%M",localtime(&now));
            formatstring(defaultfilename, "log/demo/%s_%s.dmo",ftime,smapname);
            filename = defaultfilename;
        }
        
        demotmp = openfile(filename,"w+b");
        if(!demotmp) return;
        
        stream *f = opengzfile(NULL, "wb", demotmp);
        if(!f) { DELETEP(demotmp); return; } 

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

        event_beginrecord(event_listeners(), boost::make_tuple(0, filename));
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
            sendservmsgf("cleared demo %d", n);
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
        defformatstring(file, "%s.dmo", smapname);
        demoplayback = opengzfile(file, "rb");
        if(!demoplayback) formatstring(msg, "could not read demo \"%s\"", file);
        else if(demoplayback->read(&hdr, sizeof(demoheader))!=sizeof(demoheader) || memcmp(hdr.magic, DEMO_MAGIC, sizeof(hdr.magic)))
            formatstring(msg, "\"%s\" is not a demo file", file);
        else 
        { 
            lilswap(&hdr.version, 2);
            if(hdr.version!=DEMO_VERSION) formatstring(msg, "demo \"%s\" requires an %s version of Cube 2: Sauerbraten", file, hdr.version<DEMO_VERSION ? "older" : "newer");
            else if(hdr.protocol!=PROTOCOL_VERSION) formatstring(msg, "demo \"%s\" requires an %s version of Cube 2: Sauerbraten", file, hdr.protocol<PROTOCOL_VERSION ? "older" : "newer");
        }
        if(msg[0])
        {
            DELETEP(demoplayback);
            sendservmsg(msg);
            return;
        }

        sendservmsgf("playing demo \"%s\"", file);

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
        if(!demoplayback) return;
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
            ENetPacket *packet = enet_packet_create(NULL, len+1, 0);
            if(!packet || demoplayback->read(packet->data+1, len)!=size_t(len))
            {
                if(packet) enet_packet_destroy(packet);
                enddemoplayback();
                return;
            }
            packet->data[0] = N_DEMOPACKET;
            sendpacket(-1, chan, packet);
            if(!packet->referenceCount) enet_packet_destroy(packet);
            if(!demoplayback) break;
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
        pausegame(val, NULL);
    }
    
    void pausegame(bool val, clientinfo * ci)
    {
        pausegame_owner = -1;
        if(gamepaused==val) return;
        gamepaused = val;
        sendf(-1, 1, "riii", N_PAUSEGAME, gamepaused ? 1 : 0, ci ? ci->clientnum : -1);
        if(gamepaused)
        {
            event_gamepaused(event_listeners(), boost::make_tuple());
        }
        else
        {
            event_gameresumed(event_listeners(), boost::make_tuple());
        }
    }
    
    void checkpausegame()
    {
        if(!gamepaused) return;
        int admins = 0;
        loopv(clients) if(clients[i]->privilege >= (restrictpausegame ? PRIV_ADMIN : PRIV_MASTER) || clients[i]->local) admins++;
        if(!admins) pausegame(false);
    }
    
    void forcepaused(bool paused)
    {
        pausegame(paused);
    }
    
    bool ispaused() { return gamepaused; }
    
    void changegamespeed(int val, clientinfo *ci = NULL)
    {
        val = clamp(val, 10, 1000);
        if(gamespeed==val) return;
        gamespeed = val;
        sendf(-1, 1, "riii", N_GAMESPEED, gamespeed, ci ? ci->clientnum : -1);
    }
    
    void forcegamespeed(int speed)
    {
        changegamespeed(speed);
    }

    int scaletime(int t) { return t*gamespeed; }
    
    void hashpassword(int cn, int sessionid, const char *pwd, char *result, int maxlen)
    {
        char buf[2*sizeof(string)];
        formatstring(buf, "%d %d ", cn, sessionid);
        concatstring(buf, pwd, sizeof(buf));
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

    bool setmaster(clientinfo *ci, bool request_claim_master, const char * hashed_password = "", const char *authname = NULL)
    {
        assert(!authname);
        update_mastermask();
        event_setmaster(event_listeners(), boost::make_tuple(ci->clientnum, hashed_password, request_claim_master));
        if(reset_mm) checkpausegame();
        return true;
    }
    
    bool trykick(clientinfo *ci, int victim, const char *reason = NULL, const char *authname = NULL, const char *authdesc = NULL, int authpriv = PRIV_NONE, bool trial = false)
    {
        int priv = ci->privilege;
        if(authname)
        {
            if(priv >= authpriv || ci->local) authname = authdesc = NULL;
            else priv = authpriv;
        }
        clientinfo *vinfo = (clientinfo *)getclientinfo(victim);
        if(ci->privilege && vinfo && vinfo != ci && !vinfo->spy && vinfo->privilege < PRIV_ADMIN)
        {
            if(trial) return true;
            if(ci->privilege < PRIV_ADMIN && message::limit(ci, &ci->n_kick_millis, message::resend_time::kick, "kick")) return false;
            string kicker;
            if(authname)
            {
                if(authdesc && authdesc[0]) formatstring(kicker, "%s as '\fs\f5%s\fr' [\fs\f0%s\fr]", colorname(ci), authname, authdesc);
                else formatstring(kicker, "%s as '\fs\f5%s\fr'", colorname(ci), authname);
            }
            else copystring(kicker, colorname(ci));
            if(reason && reason[0]) sendservmsgf("%s kicked %s because: %s", kicker, colorname(vinfo), reason);
            else sendservmsgf("%s kicked %s", kicker, colorname(vinfo));
            convert2utf8 utf8name(ci->name);
            convert2utf8 utf8text(reason);
            event_kick_request(event_listeners(), boost::make_tuple(ci->clientnum, utf8name.str(), 14400, victim, utf8text.str()));
        }
        return false;
     }
    
    savedscore *findscore(clientinfo *ci, bool insert)
    {
        uint ip = getclientip(ci->clientnum);
        if(!ip && !ci->local) return 0;
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
                    return &curscore;
                }
            }
        }
        loopv(scores)
        {
            savedscore &sc = scores[i];
            if(sc.ip == ip && !strcmp(sc.name, ci->name)) return &sc;
        }
        if(!insert) return 0;
        savedscore &sc = scores.add();
        sc.ip = ip;
        copystring(sc.name, ci->name);
        return &sc;
    }

    void savescore(clientinfo *ci)
    {
        savedscore * sc = findscore(ci, true);
        if(sc) sc->save(ci->state);
    }

    static struct msgfilter
    {
        uchar msgmask[NUMMSG];

        msgfilter(int msg, ...)
        {
            memset(msgmask, 0, sizeof(msgmask));
            va_list msgs;
            va_start(msgs, msg);
            for(uchar val = 1; msg < NUMMSG; msg = va_arg(msgs, int))
            {
                if(msg < 0) val = uchar(-msg);
                else msgmask[msg] = val;
            }
            va_end(msgs);
        }

        uchar operator[](int msg) const { return msg >= 0 && msg < NUMMSG ? msgmask[msg] : 0; }
    } msgfilter(-1, N_CONNECT, N_SERVINFO, N_INITCLIENT, N_WELCOME, N_MAPCHANGE, N_SERVMSG, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_DIED, N_SPAWNSTATE, N_FORCEDEATH, N_TEAMINFO, N_ITEMACC, N_ITEMSPAWN, N_TIMEUP, N_CDIS, N_CURRENTMASTER, N_PONG, N_RESUME, N_BASESCORE, N_BASEINFO, N_BASEREGEN, N_ANNOUNCE, N_SENDDEMOLIST, N_SENDDEMO, N_DEMOPLAYBACK, N_SENDMAP, N_DROPFLAG, N_SCOREFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_CLIENT, N_AUTHCHAL, N_INITAI, N_EXPIRETOKENS, N_DROPTOKENS, N_STEALTOKENS, N_DEMOPACKET, -2, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, -3, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_EDITVAR, -4, N_POS, NUMMSG),
      connectfilter(-1, N_CONNECT, -2, N_AUTHANS, -3, N_PING, NUMMSG);

    #define anticheat_parsepacket
    #include "anticheat.h"
    #undef anticheat_parsepacket

    int checktype(int type, clientinfo *ci, clientinfo *cq, packetbuf &p) //NEW (thomas): clientinfo *cq, packetbuf &p)
    {
        if(ci)
        {
            anti_cheat_parsepacket(type, ci, cq, p);

            if(!ci->connected) switch(connectfilter[type])
            {
                // allow only before authconnect
                case 1: return !ci->connectauth ? type : -1;
                // allow only during authconnect
                case 2: return ci->connectauth ? type : -1;
                // always allow
                case 3: return type;
                // never allow
                default: return -1;
            }
            if(ci->local) return type;
        }
        switch(msgfilter[type])
        {
            // server-only messages
            case 1: return ci ? -1 : type;
            // only allowed in coop-edit
            case 2: if(m_edit) break; return -1;
            // only allowed in coop-edit, no overflow check
            case 3: return m_edit ? type : -1;
            // no overflow check
            case 4: return type;
        }
        if(ci && ++ci->overflow >= 200) return -2;

        return type;
    }
    
    void cleanworldstate(ENetPacket *packet)
    {
        loopv(worldstates)
        {
            worldstate &ws = worldstates[i];
            if(!ws.contains(packet->data)) continue;
            ws.uses--;
            if(ws.uses <= 0)
            {
                ws.cleanup();
                worldstates.removeunordered(i);
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

    static void sendpositions(worldstate &ws, ucharbuf &wsbuf)
    {
        if(wsbuf.empty()) return;
        int wslen = wsbuf.length();
        recordpacket(0, wsbuf.buf, wslen);
        wsbuf.put(wsbuf.buf, wslen);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            uchar *data = wsbuf.buf;
            int size = wslen;
            if(ci.wsdata >= wsbuf.buf) { data = ci.wsdata + ci.wslen; size -= ci.wslen; }
            if(size <= 0) continue;
            ENetPacket *packet = enet_packet_create(data, size, ENET_PACKET_FLAG_NO_ALLOCATE);
            sendpacket(ci.clientnum, 0, packet);
            if(packet->referenceCount) { ws.uses++; packet->freeCallback = cleanworldstate; }
            else enet_packet_destroy(packet);
        }
        wsbuf.offset(wsbuf.length());
    }
    
    static inline void addposition(worldstate &ws, ucharbuf &wsbuf, int mtu, clientinfo &bi, clientinfo &ci)
    {
        if(bi.position.empty()) return;
        if(wsbuf.length() + bi.position.length() > mtu) sendpositions(ws, wsbuf);
        int offset = wsbuf.length();
        wsbuf.put(bi.position.getbuf(), bi.position.length());
        bi.position.setsize(0);
        int len = wsbuf.length() - offset;
        if(ci.wsdata < wsbuf.buf) { ci.wsdata = &wsbuf.buf[offset]; ci.wslen = len; }
        else ci.wslen += len;
    }
    
    static void sendmessages(worldstate &ws, ucharbuf &wsbuf)
    {
        if(wsbuf.empty()) return;
        int wslen = wsbuf.length();
        recordpacket(1, wsbuf.buf, wslen);
        wsbuf.put(wsbuf.buf, wslen);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            uchar *data = wsbuf.buf;
            int size = wslen;
            if(ci.wsdata >= wsbuf.buf) { data = ci.wsdata + ci.wslen; size -= ci.wslen; }
            if(size <= 0) continue;
            ENetPacket *packet = enet_packet_create(data, size, (reliablemessages ? ENET_PACKET_FLAG_RELIABLE : 0) | ENET_PACKET_FLAG_NO_ALLOCATE);
            sendpacket(ci.clientnum, 1, packet);
            if(packet->referenceCount) { ws.uses++; packet->freeCallback = cleanworldstate; }
            else enet_packet_destroy(packet);
        }
        wsbuf.offset(wsbuf.length());
    }
    
    static inline void addmessages(worldstate &ws, ucharbuf &wsbuf, int mtu, clientinfo &bi, clientinfo &ci)
    {
        if(bi.messages.empty()) return;
        if(wsbuf.length() + 10 + bi.messages.length() > mtu) sendmessages(ws, wsbuf);
        int offset = wsbuf.length();
        putint(wsbuf, N_CLIENT);
        putint(wsbuf, bi.clientnum);
        putuint(wsbuf, bi.messages.length());
        wsbuf.put(bi.messages.getbuf(), bi.messages.length());
        bi.messages.setsize(0);
        int len = wsbuf.length() - offset;
        if(ci.wsdata < wsbuf.buf) { ci.wsdata = &wsbuf.buf[offset]; ci.wslen = len; }
        else ci.wslen += len;
    }
    
    bool buildworldstate()
    {
        int wsmax = 0;
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            ci.overflow = 0;
            ci.wsdata = NULL;
            wsmax += ci.position.length();
            if(ci.messages.length()) wsmax += 10 + ci.messages.length();
        }
        if(wsmax <= 0)
        {
            reliablemessages = false;
            return false;
        }
        worldstate &ws = worldstates.add();
        ws.setup(2*wsmax);
        int mtu = getservermtu() - 100;
        if(mtu <= 0) mtu = ws.len;
        ucharbuf wsbuf(ws.data, ws.len);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            addposition(ws, wsbuf, mtu, ci, ci);
            loopvj(ci.bots) addposition(ws, wsbuf, mtu, *ci.bots[j], ci);
        }
        sendpositions(ws, wsbuf);
        loopv(clients)
        {
            clientinfo &ci = *clients[i];
            if(ci.state.aitype != AI_NONE) continue;
            addmessages(ws, wsbuf, mtu, ci, ci);
            loopvj(ci.bots) addmessages(ws, wsbuf, mtu, *ci.bots[j], ci);
        }
        sendmessages(ws, wsbuf);
        reliablemessages = false;
        if(ws.uses) return true;
        ws.cleanup();
        worldstates.drop();
        return false;
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
            int specs = 0;
            loopv(clients)
            {
                clientinfo &ci = *clients[i];
                if(!ci.is_delayed_spectator() || ci.specmillis + spectator_delay > totalmillis) continue;
                sendpacket(ci.clientnum, delayed_sendpackets[0].channel, packet);
                specs++;
            }
            if(!specs) free(delayed_sendpackets[0].data);
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
            if(!ci->connected || ci->clientnum == exclude || ci->spy) continue;

            putinitclient(ci, p);
        }
    }
    
    bool hasmap(clientinfo *ci)
    {
        return (m_edit && (clients.length() > 0 || ci->local)) ||
               (smapname[0] && (!m_timed || gamemillis < gamelimit || (ci->state.state==CS_SPECTATOR && !ci->privilege && !ci->local) || numclients(ci->clientnum, true, true, true)));
    }

    int welcomepacket(packetbuf &p, clientinfo *ci)
    {
        putint(p, N_WELCOME);
        putint(p, N_MAPCHANGE);
        sendstring(smapname, p);
        putint(p, gamemode);
        putint(p, notgotitems ? 1 : 0);
        if(!ci || (m_timed && smapname[0]))
        {
            putint(p, N_TIMEUP);
            putint(p, gamemillis < gamelimit && !interm ? max((gamelimit - gamemillis)/1000, 1) : 0);
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
        bool hasmaster = false;
        if(mastermode != MM_OPEN)
        {
            putint(p, N_CURRENTMASTER);
            putint(p, mastermode);
            hasmaster = true;
        }
        loopv(clients) if(clients[i]->privilege >= PRIV_MASTER)
        {
            if(!hasmaster)
            {
                putint(p, N_CURRENTMASTER);
                putint(p, mastermode);
                hasmaster = true;
            }
            if(clients[i]->privilege > PRIV_NONE && !clients[i]->hide_privilege) {
                putint(p, clients[i]->clientnum);
                putint(p, clients[i]->privilege);
            }
        }
        if(hasmaster) putint(p, -1);
        if(gamepaused)
        {
            putint(p, N_PAUSEGAME);
            putint(p, 1);
            putint(p, -1);
        }
        if(gamespeed != 100)
        {
            putint(p, N_GAMESPEED);
            putint(p, gamespeed);
            putint(p, -1);
        }
        if(m_teammode)
        {
            putint(p, N_TEAMINFO);
            enumerate(teaminfos, teaminfo, t,
                if(t.frags) { sendstring(t.team, p); putint(p, t.frags); }
            );
            sendstring("", p);
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
        savedscore * sc = findscore(ci, false);
        if(sc)
        {
            sc->restore(ci->state);
            return true;
        }
        return false;
    }
    
    void sendresume(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        sendf(ci->ownernum, 1, "ri3i9vi", N_RESUME, ci->clientnum,
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
    
    #include "ents.h"

    void loaditems()
    {
        resetitems();
        notgotitems = true;
        if(m_edit || !e::loadents(smapname, ments, mcrc))
        {
            convert2utf8 utf8mapname(smapname);
            std::cerr << "failed to load mapents for map " << utf8mapname.str() << std::endl;
            mcrc = 0;
            return;
        }
        loopv(ments) if(canspawnitem(ments[i].type))
        {
            server_entity se = { NOTUSED, 0, false, -1 };
            while(sents.length()<=i) sents.add(se);
            sents[i].type = ments[i].type;
            if(m_mp(gamemode) && delayspawn(sents[i].type)) sents[i].spawntime = spawntime(sents[i].type);
            else sents[i].spawned = true;
        }
        notgotitems = false;
    }

    void changemap(const char *s, int mode,int mins = -1)
    {
        calc_player_ranks();
        event_finishedgame(event_listeners(), boost::make_tuple());
        
        stopdemo();
        pausegame(false);
        changegamespeed(100);
        if(smode) smode->cleanup();
        aiman::clearai();
        
        mapreload = false;
        gamemode = mode;
        gamemillis = 0;
        gamelimit = (mins == -1 ? (m_overtime ? 15 : 10) : mins) * 60000;
        interm = 0;
        nextexceeded = 0;
        copystring(smapname, s);
        loaditems();
        scores.shrink(0);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
        }

        if(!m_mp(gamemode)) kicknonlocalclients(DISC_PRIVATE);
        
        sendf(-1, 1, "risii", N_MAPCHANGE, smapname, gamemode, 1);

        clearteaminfo();
        if(m_teammode)
        {
            if(reassignteams) autoteam();
            else
            {
                loopv(clients) addteaminfo(clients[i]->team);
            }
        }

        if(m_capture) smode = &capturemode;
        else if(m_ctf) smode = &ctfmode;
        else if(m_collect) smode = &collectmode;
        else smode = NULL;

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
        
        if(smode) smode->setup();
        
        loopv(clients) clients[i]->ac.reset();
        
        convert2utf8 utf8mapname(smapname);
        event_mapchange(event_listeners(), boost::make_tuple(utf8mapname.str(), modename(gamemode,"unknown")));
        
        next_timeupdate = 0; //as soon as possible
    }
    
    bool rotatemap()
    {
        event_setnextgame(event_listeners(), boost::make_tuple());
        if(next_gamemode[0] && next_mapname[0])
        {
            int next_gamemode_code = modecode(next_gamemode);
            if(m_mp(next_gamemode_code))
            {
                mapreload = false;
                changemap(next_mapname, next_gamemode_code, next_gametime);
                next_gamemode[0] = '\0';
                next_mapname[0] = '\0';
                next_gametime = -1;
            }
            else
            {
                std::cerr << next_gamemode << " game mode is unrecognised." << std::endl;
                //sendf(-1, 1, "ri", N_MAPRELOAD);
            }
            return true;
        }else return false;
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
            if(!m_valid(oi->modevote)) continue;
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
                convert2utf8 utf8mapname(best->map);
                event_votepassed(event_listeners(), boost::make_tuple(utf8mapname.str(), modename(best->mode)));
                changemap(best->map, best->mode);
            }
            else rotatemap();
        }
    }

    void forcemap(const char *map, int mode)
    {
        stopdemo();
        if(!map[0] && !m_check(mode, M_EDIT))
        {
            if(smapname[0]) map = smapname;
            else rotatemap();
        }
        if(hasnonlocalclients()) sendservmsgf("local player forced %s on map %s", modename(mode), map[0] ? map : "[new map]");
        changemap(map, mode);
    }

    void vote(const char *map, int reqmode, int sender)
    {
        clientinfo *ci = getinfo(sender);
        if(!ci || (ci->state.state==CS_SPECTATOR && !ci->privilege && !ci->local) || (!ci->local && !m_mp(reqmode))) return;
        strncpy(ci->mapvote, map, 64);
        ci->modevote = reqmode;
        if(!ci->mapvote[0]) return;
        if(ci->local || mapreload || (ci->privilege && mastermode>=MM_VETO))
        {
            if(demorecord) enddemorecord();
            if((!ci->local || hasnonlocalclients()) && !mapreload && !ci->spy)
            {
                sendservmsgf("%s %s forced %s on map %s", ci->privilege && mastermode>=MM_VETO ? privname(ci->privilege) : "local player", colorname(ci), modename(ci->modevote), ci->mapvote);
            }
            changemap(ci->mapvote, ci->modevote);
        }
        else
        {
            if (!ci->spy) sendservmsgf("%s suggests %s on map %s (select map to vote)", colorname(ci), modename(reqmode), map);
            checkvotes();
        }
    }

    void checkintermission()
    {
        if(gamemillis >= gamelimit && !interm)
        {
            event_timeupdate(event_listeners(), boost::make_tuple(0, 0));
            if(gamemillis < gamelimit) return;
            
            sendf(-1, 1, "ri2", N_TIMEUP, 0);
            interm = gamemillis+intermtime;
            calc_player_ranks();
            event_intermission(event_listeners(), boost::make_tuple());
        }
    }

    void startintermission() { gamelimit = min(gamelimit, gamemillis); checkintermission(); }

    void dodamage(clientinfo *target, clientinfo *actor, int damage, int gun, const vec &hitpush = vec(0, 0, 0))
    {
        if(event_damage(event_listeners(), boost::make_tuple(target->clientnum, actor->clientnum, damage, gun, hitpush[0], hitpush[1], hitpush[2])))
        {
            return;
        }

        if (anti_cheat_enabled)
        {
            //anticheat *ac_a = &actor->ac;
            anticheat *ac_t = &target->ac;
            
            ac_t->damage();

            float st_dist = distance(actor->state.o, target->state.o);
            if ((int)st_dist > (guns[gun].range + 50/*tolerance*/))
            {
                defformatstring(cheatinfo, "GUN: %s GUN-RANGE: %i DISTANCE: %.2f", "%s", guns[gun].range, st_dist);
                actor->ac.out_of_gun_distance_range(gun, cheatinfo);
                return;
            }
        }
         
        gamestate &ts = target->state;
        ts.dodamage(damage);
        if(target!=actor && !isteam(target->team, actor->team)) actor->state.damage += damage;
        sendf(-1, 1, "ri6", N_DAMAGE, target->clientnum, actor->clientnum, damage, ts.armour, ts.health); 
        if(target==actor) target->setpushed();
        else if(!hitpush.iszero())
        {
            ivec v(vec(hitpush).rescale(DNF));
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
                event_teamkill(event_listeners(), boost::make_tuple(actor->clientnum, target->clientnum));
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
            teaminfo *t = m_teammode ? teaminfos.access(actor->team) : NULL;
            if(t) t->frags += fragvalue;
            event_frag(event_listeners(), boost::make_tuple(target->clientnum, actor->clientnum));
            sendf(-1, 1, "ri5", N_DIED, target->clientnum, actor->clientnum, actor->state.frags, t ? t->frags : 0);
            target->position.setsize(0);
            if(smode) smode->died(target, actor);
            ts.state = CS_DEAD;
            ts.lastdeath = gamemillis;
            ts.deadflush = ts.lastdeath + DEATHMILLIS;
            // don't issue respawn yet until DEATHMILLIS has elapsed
            // ts.respawn();
        }
    }
    
    void suicide(clientinfo *ci)
    {
        gamestate &gs = ci->state;
        if(gs.state!=CS_ALIVE) return;
        int fragvalue = smode ? smode->fragvalue(ci, ci) : -1;
        ci->state.frags += fragvalue;
        ci->state.deaths++;
        ci->state.suicides++;
        teaminfo *t = m_teammode ? teaminfos.access(ci->team) : NULL;
        if(t) t->frags += fragvalue;
        sendf(-1, 1, "ri5", N_DIED, ci->clientnum, ci->clientnum, gs.frags, t ? t->frags : 0);
        ci->position.setsize(0);
        if(smode) smode->died(ci, NULL);
        gs.state = CS_DEAD;
        gs.lastdeath = gamemillis;
        gs.respawn();
        event_suicide(event_listeners(), boost::make_tuple(ci->clientnum));
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
            if(!target || target->state.state!=CS_ALIVE || h.lifesequence!=target->state.lifesequence || h.dist<0 || h.dist>guns[gun].exprad) continue;
            
            bool dup = false;
            loopj(i) if(hits[j].target==h.target) { dup = true; break; }
            if(dup) continue;
            
            gs.hits += (ci != target ? 1 : 0);
            
            int damage = guns[gun].damage;
            if(gs.quadmillis) damage *= 4;        
            damage = int(damage*(1-h.dist/EXP_DISTSCALE/guns[gun].exprad));
            if(target==ci) damage /= EXP_SELFDAMDIV;
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
        gs.shotdamage += guns[gun].damage*(gs.quadmillis ? 4 : 1)*guns[gun].rays;
        
        gs.shots++;
        int old_hits = gs.hits; 
        
        switch(gun)
        {
            case GUN_RL: gs.rockets.add(id); break;
            case GUN_GL: gs.grenades.add(id); break;
            default:
            {
                int totalrays = 0, maxrays = guns[gun].rays;
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
        
        event_shot(event_listeners(), boost::make_tuple(ci->clientnum, gun, gs.hits - old_hits));
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

    void serverupdate()
    {
        timer serverupdate_time;
        
        if(shouldstep && !gamepaused)
        {
            gamemillis += curtime;

            if(m_demo) readdemo();
            else if(!m_timed || gamemillis < gamelimit)
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
        }
        
        loopv(connects) if(totalmillis-connects[i]->connectmillis>15000) disconnect_client(connects[i]->clientnum, DISC_TIMEOUT);
        
        if(nextexceeded && gamemillis > nextexceeded && (!m_timed || gamemillis < gamelimit))
        {
            nextexceeded = 0;
            loopvrev(clients)
            {
                clientinfo &c = *clients[i];
                if(c.state.aitype != AI_NONE) continue;
                if(c.checkexceeded())
                {
                    if (anti_cheat_enabled)
                    {
                        anticheat *ac = &c.ac;
                        if (!ac->ignore_exceed || totalmillis > ac->ignore_exceed)
                        {
                            ac->player_position_exceeded();
                            ac->ignore_exceed = totalmillis + 2500;
                        }
                    }
                    else disconnect_client(c.clientnum, DISC_MSGERR);
                }
                else c.scheduleexceeded();
            }
        }
        
        if(gamemillis > next_timeupdate)
        {
            event_timeupdate(event_listeners(), boost::make_tuple(get_minutes_left(), get_seconds_left()));
            next_timeupdate = gamemillis + 60000;
        }
        
        if(shouldstep && !gamepaused)
        {
            if(m_timed && smapname[0] && gamemillis-curtime>0) checkintermission();
            
            if(interm > 0 && gamemillis > interm + spectator_delay && delayed_sendpackets.length() == 0)
            {
                spectator_delay = 0;
                if(demorecord) enddemorecord();
                interm = -1;
                checkvotes(true);
            }
        }
        
        anti_cheat_serverupdate();
        
        timer::time_diff_t elapsed = serverupdate_time.usec_elapsed();
        if(elapsed >= timer_alarm_threshold)
        {
            int timeleft = gamemillis < gamelimit && !interm ? max((gamelimit - gamemillis)/1000, 1) : 0;
            
            std::cerr<<"Detected slowdown in serverupdate(): "
                     <<elapsed<<" usecs, "
                     <<numclients(-1, false, true)<<" clients, "
                     <<timeleft<< " minutes left"
                     <<std::endl;
        }
           
        shouldstep = clients.length() > 0;

        static vector<uint> disc_topurge;

        enumeratekt(disc_record, uint, ip, vector<int>, millis, {
            int toremove = 0;
            loopv(millis) if(totalmillis-millis[i] < message::disc_window)
            {
                toremove = i;
                break;
            }
            if(toremove)
            {
                millis.remove(0, toremove);
                if(millis.empty()) disc_topurge.add(ip);
            }
        });

        loopv(disc_topurge) disc_record.remove(disc_topurge[i]);
        disc_topurge.setsize(0);
    }

    struct crcinfo 
    { 
        int crc, matches; 

        crcinfo() {}
        crcinfo(int crc, int matches) : crc(crc), matches(matches) {}

        static bool compare(const crcinfo &x, const crcinfo &y) { return x.matches > y.matches; }
    };

    void sendservinfo(clientinfo *ci)
    {
        sendf(ci->clientnum, 1, "ri5ss", N_SERVINFO, ci->clientnum, PROTOCOL_VERSION, ci->sessionid, serverpass[0] ? 1 : 0, serverdesc, serverauth);
    }

    void noclients()
    {
        event_clearbans_request(event_listeners(), boost::make_tuple(-1));
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
        ci->clientnum = ci->ownernum = ci->n = n;
        ci->connectmillis = totalmillis;
        ci->sessionid = (rnd(0x1000000)*((totalmillis%10000)+1))&0xFFFFFF;
        
        connects.add(ci);
        if(!m_mp(gamemode)) return DISC_PRIVATE;
        sendservinfo(ci);
        
        return DISC_NONE;
    }

    void clientdisconnect(int n,int reason) 
    {
        clientinfo *ci = (clientinfo *)getinfo(n);
        if(ci->spy) spies.remove(ci->clientnum - spycn);

        const char * disc_reason_msg = "normal";
        if(reason != DISC_NONE || ci->disconnect_reason.length())
        {
            disc_reason_msg = (ci->disconnect_reason.length() ? ci->disconnect_reason.c_str() : disconnect_reason(reason));
            convert2cube disc_reason_msg_cubeenc(disc_reason_msg);
            defformatstring(discmsg, "client (%s) disconnected because: %s", ci->hostname(), disc_reason_msg_cubeenc.str());
            printf("%s\n", discmsg);
            if (!ci->spy)
            {
                bool senddiscmsg = true;
                if(message::disc_window > 0 && message::disc_msgs > 0)
                {
                    vector<int>& millis = disc_record[getclientip(n)];
                    millis.put(totalmillis);
                    senddiscmsg = millis.length() <= message::disc_msgs;
                }
                if(senddiscmsg) sendservmsg(discmsg);
            }
        }
        else
        {
            defformatstring(discmsg, "disconnected client (%s)", ci->hostname());
            puts(discmsg);
        }
        
        if(ci->connected)
        {
            if(ci->privilege) setmaster(ci, false);
            
            if(smode) smode->leavegame(ci, true);
            
            ci->state.timeplayed += lastmillis - ci->state.lasttimeplayed;
            ci->state.disconnecttime = totalmillis;
            
            savescore(ci);
            
            if (!ci->spy) sendf(-1, 1, "ri2", N_CDIS, n);
            
            clients.removeobj(ci);
            aiman::removeai(ci);
            
            maxclients -= reservedslots_use > 0;
            reservedslots_use -= reservedslots_use > 0;
            reservedslots += ci->using_reservedslot;
            
            event_disconnect(event_listeners(), boost::make_tuple(n, disc_reason_msg));
            
            if(clients.empty()) noclients();
            else aiman::dorefresh = true;
            
            if(reset_mm) checkpausegame();
        }
        else
        {
            event_failedconnect(event_listeners(), boost::make_tuple(ci->hostname(), disc_reason_msg));
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

        ci->spy = false;
        
        convert2utf8 utf8name(ci->name);
        if(event_connecting(event_listeners(), boost::make_tuple(ci->clientnum, ci->hostname(), utf8name.str(), pwd, is_reserved)))
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

        int spy_count = spies.length();
        int maxclients_ = maxclients;
        if (spec_slots) maxclients_ += spec_count();
        if(ci->spy) spy_count++; // allow connect as "spy" when server is full
        if(clientcount >= maxclients_ + spy_count) return DISC_MAXCLIENTS;
        
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
    
    bool tryauth(clientinfo *ci, const char * user, const char * domain, int kickcn = -1)
    {
        convert2utf8 utf8user(user);
        event_authreq(event_listeners(), boost::make_tuple(ci->clientnum, utf8user.str(), domain, kickcn));
        return true;
    }
    
    void answerchallenge(clientinfo *ci, uint id, char *val, const char * desc)
    {
        for(char *s = val; *s; s++)
        {
            if(!isxdigit(*s)) { *s = '\0'; break; }
        }
        event_authrep(event_listeners(), boost::make_tuple(ci->clientnum, id, val));
        if(ci->privilege >= PRIV_AUTH && ci->authkickvictim >= 0)
        {
            trykick(ci, ci->authkickvictim, ci->authkickreason, ci->authname, ci->authdesc, ci->privilege);
        }
    }

    void receivefile(int sender, uchar *data, int len)
    {
        if(!m_edit || len > 4*1024*1024) return;
        clientinfo *ci = getinfo(sender);
        if(ci->state.state==CS_SPECTATOR && !ci->privilege && !ci->local) return;
        if(event_editpacket(event_listeners(), boost::make_tuple(ci->clientnum))) return;
        if(mapdata) DELETEP(mapdata);
        if(!len) return;
        mapdata = opentempfile("mapdata", "w+b");
        if(!mapdata) { sendf(sender, 1, "ris", N_SERVMSG, "failed to open temporary file for map"); return; }
        mapdata->write(data, len);
        sendservmsgf("[%s sent a map to server, \"/getmap\" to receive it]", colorname(ci));
    }

    void sendclipboard(clientinfo *ci)
    {
        if(!ci->lastclipboard || !ci->clipboard) return;
        if(event_editpacket(event_listeners(), boost::make_tuple(ci->clientnum))) return;
        bool flushed = false;
        loopv(clients)
        {
            clientinfo &e = *clients[i];
            if(e.clientnum != ci->clientnum && e.needclipboard - ci->lastclipboard >= 0)
            {
                if(!flushed) { flushserver(true); flushed = true; }
                sendpacket(e.clientnum, 1, ci->clipboard);
            }
        }
    }

    void setspectator(clientinfo * spinfo, bool val, bool broadcast)
    {
        if(!spinfo || !spinfo->connected || (spinfo->state.state==CS_SPECTATOR ? val : !val) || spinfo->state.aitype != AI_NONE || spinfo->spy) return;
        
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
        sendf(broadcast ? -1 : spinfo->clientnum, 1, "ri3", N_SPECTATOR, spinfo->clientnum, val);
        
        if(spectator_delay)
        {
            sendf(spinfo->clientnum, 1, "ri3", N_SPECTATOR, spinfo->clientnum, val);
            sendf(spinfo->clientnum, 1, "ri2", N_TIMEUP, (gamelimit - gamemillis + spectator_delay)/1000);
        }
        
        event_spectator(event_listeners(), boost::make_tuple(spinfo->clientnum, val));
    }
    
    void connected(clientinfo *ci)
    {
        if(m_demo) enddemoplayback();
        
        connects.removeobj(ci);
        clients.add(ci);
        
        ci->connected = true;
        ci->needclipboard = totalmillis ? totalmillis : 1;
        if(mastermode>=MM_LOCKED) ci->state.state = CS_SPECTATOR;
        ci->state.lasttimeplayed = lastmillis;
        
        const char *worst = m_teammode ? chooseworstteam(NULL, ci) : NULL;
        copystring(ci->team, worst ? worst : "good", MAXTEAMLEN+1);
        
        if(clients.length() == 1 && mapreload) rotatemap();
        
        sendwelcome(ci);
        if(restorescore(ci)) sendresume(ci);
        sendinitclient(ci);
        
        aiman::addclient(ci);
        
        if(m_demo) setupdemoplayback();
        
        if(clients.length() == 1 && mapreload) rotatemap();
        
        event_connect(event_listeners(), boost::make_tuple(ci->clientnum, ci->spy));
    }

    void parsepacket(int sender, int chan, packetbuf &p)     // has to parse exactly each byte of the packet
    {
        timer parsepacket_time;
        
        if(sender<0 || p.packet->flags&ENET_PACKET_FLAG_UNSEQUENCED || chan > 2) return;
        char text[MAXTRANS];
        int type;
        clientinfo *ci = sender>=0 ? getinfo(sender) : NULL, *cq = ci, *cm = ci;
        if(ci && !ci->connected)
        {
            if(chan==0) return;
            else if(chan!=1) { disconnect_client(sender, DISC_MSGERR); return; }
            else while(p.length() < p.maxlen) switch(checktype(getint(p), ci, NULL, p))
            {
                case N_CONNECT:
                {
                    getstring(text, p);
                    filtertext(text, text, false, false, MAXNAMELEN);
                    if(!text[0]) copystring(text, "unnamed");
                    copystring(ci->name, text, MAXNAMELEN+1);
                    ci->playermodel = getint(p);
                    
                    ci->ac.reset(sender);
                    ci->state.lastdeath = -5000;
                    
                    string password, authdesc, authname;
                    getstring(password, p, sizeof(password));
                    getstring(authdesc, p, sizeof(authdesc));
                    getstring(authname, p, sizeof(authname));
                    int disc = allowconnect(ci, password);
                    if(disc)
                    {
                        if(!serverauth[0] || strcmp(serverauth, authdesc) || !tryauth(ci, authname, authdesc))
                        {
                            disconnect_client(sender, disc);
                            return;
                        }
                        ci->connectauth = disc;
                    }
                    else connected(ci);
                    break;
                }
                
                case N_AUTHANS:
                {
                    string desc, ans;
                    getstring(desc, p, sizeof(desc));
                    uint id = (uint)getint(p);
                    getstring(ans, p, sizeof(ans));
                    answerchallenge(ci, id, ans, desc);
                    break;
                }

                case N_PING:
                    getint(p);
                    break;

                default:
                    disconnect_client(sender, DISC_MSGERR);
                    return;
            }
            return;
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
        while((curmsg = p.length()) < p.maxlen) switch(type = checktype(getint(p), ci, cq, p))
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

                vec cam = vec(p.get(), p.get(), p.get());// hopmod

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
                        float pos_z = max(vel.magnitude2(), (float)fabs(vel.z));
                        if(!ci->local && !m_edit && pos_z >= 180)
                        {
                            cp->setexceeded();
                            if (anti_cheat_enabled) cq->ac.exceed_position = pos_z;
                        }
                        cp->position.setsize(0);
                        while(curmsg<p.length()) cp->position.add(p.buf[curmsg++]);
                    }
                    if(smode && cp->state.state==CS_ALIVE) smode->moved(cp, cp->state.o, cp->gameclip, pos, (flags&0x80)!=0);

                    cp->state.o = pos;
                    cp->state.cam = cam;
                    cp->gameclip = (flags&0x80)!=0;
                    
                    if(cp->state.state==CS_ALIVE && !cp->maploaded && cp->state.aitype == AI_NONE)
                    {
                        cp->lastposupdate = totalmillis - 30;
                        cp->maploaded = gamemillis;
                        event_maploaded(event_listeners(), boost::make_tuple(cp->clientnum));
                    }
                    
                    if(ci->maploaded)
                    {
                        cp->lag = (std::max(30,cp->lag)*10 + (totalmillis - cp->lastposupdate))/12;
                        cp->last_lag = totalmillis - cp->lastposupdate;
                        cp->lastposupdate = totalmillis;
                    }
                }
                break;
            }

            case N_TELEPORT:
            {
                int pcn = getint(p), teleport = getint(p), teledest = getint(p);
                clientinfo *cp = getinfo(pcn);
                if(!cq) break;
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
                if(!cq) break;
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
                event_editmode(event_listeners(), boost::make_tuple(ci->clientnum, val));
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

                if(!m_edit && mcrc && (uint)mcrc != (uint)ci->mapcrc)
                {
                    convert2utf8 utf8map(ci->clientmap);
                    event_modmap(event_listeners(), boost::make_tuple(ci->clientnum, utf8map.str(), ci->mapcrc));
                    sendservmsgf("%s is using a modified map", colorname(ci, ci->name));
                }
                break;
            }

            case N_CHECKMAPS:
                if(!mcrc) break;
                loopv(clients)
                {
                    clientinfo *ci = clients[i];
                    if(ci->mapcrc && (uint)mcrc != (uint)ci->mapcrc)
                    {
                        convert2utf8 utf8map(ci->clientmap);
                        event_modmap(event_listeners(), boost::make_tuple(ci->clientnum, utf8map.str(), ci->mapcrc));
                        sendservmsgf("%s is using a modified map", colorname(ci, ci->name));
                    }
                }
                break;

            case N_TRYSPAWN:
                try_respawn(ci, cq);
                break;

            case N_GUNSELECT:
            {
                int gunselect = getint(p);
                if(!cq || cq->state.state!=CS_ALIVE) break;
				if(gunselect<GUN_FIST || gunselect>GUN_PISTOL) break;
                cq->state.gunselect = gunselect;
                QUEUE_AI;
                QUEUE_MSG;
                break;
            }

            case N_SPAWN:
            {
                int ls = getint(p), gunselect = getint(p);
                if(gunselect<GUN_FIST || gunselect>GUN_PISTOL) break;
                if(!cq || (cq->state.state!=CS_ALIVE && cq->state.state!=CS_DEAD && cq->state.state!=CS_EDITING) || ls!=cq->state.lifesequence || cq->state.lastspawn<0) break;
                if(!cq->mapcrc && cq->state.aitype == AI_NONE)
                {
                    convert2utf8 utf8map(ci->clientmap);
                    event_modmap(event_listeners(), boost::make_tuple(cq->clientnum, utf8map.str(), cq->mapcrc));
                    sendservmsgf("%s is using a modified map", colorname(cq, cq->name));
                    cq->mapcrc = 1;
                }
                if (ci->spy)
                {
                    ci->spy = false;
                    ci->state.state = CS_DEAD;
                    setspectator(ci, true, false);
                    ci->spy = true;
                    ci->sendprivtext(RED "You've entered the spy-mode.");
                    break;
                }
                cq->state.lastspawn = -1;
                cq->state.state = CS_ALIVE;
                cq->state.gunselect = gunselect;
                cq->exceeded = 0;
                cq->lastposupdate = 0;
                if(smode) smode->spawned(cq);
                QUEUE_AI;
                QUEUE_BUF({
                    putint(cm->messages, N_SPAWN);
                    sendstate(cq->state, cm->messages);
                });
                event_spawn(event_listeners(), boost::make_tuple(cq->clientnum));
                cq->state.lastdeath = 0;
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
                filtertext(text, text, true, true);
                
                if(ci && (ci->privilege == PRIV_ADMIN || !message::limit(ci, &ci->n_text_millis, message::resend_time::text, "text")))
                {
                    bool is_command = text[0] == '#';

                    if(is_command && !strcmp(text+1, "reload") && ci->privilege == PRIV_ADMIN)
                    {
                        reload_hopmod();
                        break;
                    }

                    if (ci->spy && !is_command)
                    {
                        sendservmsgf("\f3REMOTE ADMIN \f0(\f8%s\f0)\f3: \f8%s", ci->name, text);
                        break;
                    }
                    
                    convert2utf8 utf8text(text);
                    if( event_text(event_listeners(), boost::make_tuple(ci->clientnum, utf8text.str())) == false &&
                        !ci->is_delayed_spectator())
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
                if(!ci || !cq || (ci->state.state==CS_SPECTATOR && !ci->privilege) || !m_teammode || !cq->team[0] || message::limit(ci, &ci->n_sayteam_millis, message::resend_time::sayteam, "team chat") || ci->spy) break;
                filtertext(text, text, true, true);
                convert2utf8 utf8text(text);
                if(event_sayteam(event_listeners(), boost::make_tuple(ci->clientnum, utf8text.str())) == false)
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
                filtertext(text, text, false, false, MAXNAMELEN);
                if(!text[0]) copystring(text, "unnamed");
                convert2utf8 newnameutf8(text);
          
                string oldname;
                copystring(oldname, ci->name);
                
                bool allow_rename = !ci->spy && strcmp(ci->name, text) &&
                    !message::limit(ci, &ci->n_switchname_millis, message::resend_time::switchname, "name change") &&
                    event_allow_rename(event_listeners(), boost::make_tuple(ci->clientnum, newnameutf8.str())) == false;
                
                if(allow_rename)
                {
                    copystring(ci->name, text);
                    
                    event_renaming(event_listeners(), boost::make_tuple(ci->clientnum, 0));
                    
                    convert2utf8 oldnameutf8(oldname);
                    event_rename(event_listeners(), boost::make_tuple(ci->clientnum, oldnameutf8.str(), newnameutf8.str()));
                    
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
                int model = getint(p);
                if(model<0 || model>4) break;
                ci->playermodel = model;
                if (ci->spy) break;
                QUEUE_MSG;
                break;
            }

            case N_MAPVOTE:
            {
                getstring(text, p);
                filtertext(text, text, false);
                int reqmode = getint(p);
                if(!ci->local && !m_mp(reqmode)) reqmode = 0;
                convert2utf8 utf8text(text);
                if(!message::limit(ci, &ci->n_mapvote_millis, message::resend_time::mapvote, "mapvote") &&
                   event_mapvote(event_listeners(), boost::make_tuple(ci->clientnum, utf8text.str(), modename(reqmode, "unknown"))) == false)
                {
                    vote(text, reqmode, sender);
                }
                break;
            }
            
            case N_SWITCHTEAM:
            {
                getstring(text, p);
                filtertext(text, text, false, false, MAXTEAMLEN);
                convert2utf8 newteamutf8(text);
                convert2utf8 oldteamutf8(ci->team);
                
                bool allow = m_teammode && text[0] && strcmp(ci->team, text) && 
                    (!smode || smode->canchangeteam(ci, ci->team, text)) &&
                    !message::limit(ci, &ci->n_switchteam_millis, message::resend_time::switchteam, "team change") &&
                    event_chteamrequest(event_listeners(), boost::make_tuple(ci->clientnum, oldteamutf8.str(), newteamutf8.str(), sender)) == false;
                
                if(allow && addteaminfo(text))
                {
                    if(ci->state.state==CS_ALIVE) suicide(ci);
                    copystring(ci->team, text);
                    aiman::changeteam(ci);
                    event_reteam(event_listeners(), boost::make_tuple(ci->clientnum, oldteamutf8.str(), newteamutf8.str()));
                    if(!ci->spy) sendf(-1, 1, "riisi", N_SETTEAM, sender, ci->team, ci->state.state==CS_SPECTATOR ? -1 : 0);
                }
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
                int n;
                
                if((ci->state.state==CS_SPECTATOR && !ci->privilege && !ci->local) || !notgotitems || strcmp(ci->clientmap, smapname)) { while(getint(p)>=0 && !p.overread()) getint(p); break; }
                while((n = getint(p))>=0 && n<MAXENTS && !p.overread())
                {
                    server_entity se = { NOTUSED, 0, false, -1 };
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
                if(event_editpacket(event_listeners(), boost::make_tuple(ci->clientnum))) return;
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
                if(event_editpacket(event_listeners(), boost::make_tuple(ci->clientnum))) return;
                if(ci && ci->state.state!=CS_SPECTATOR) QUEUE_MSG;
                break;
            }

            case N_PING:
            {
                int clientmillis = getint(p);

                if(!ci->maploaded && totalmillis - ci->connectmillis > 2000)
                {
                    ci->maploaded = gamemillis;
                    event_maploaded(event_listeners(), boost::make_tuple(ci->clientnum));
                }

				ci->lastpingupdate = totalmillis;
				ci->clientmillis = clientmillis;
                
                sendf(sender, 1, "i2", N_PONG, clientmillis);
                break;
            }

            case N_CLIENTPING:
            {
                int ping = getint(p);
                if (ping > 0 && ping < 15000)
                {
                    if(ci) 
                    {
                        ci->ac._ping = ci->ping = ping;
                        loopv(ci->bots) ci->bots[i]->ping = ping;
                    }
                    if (!ci->spy) QUEUE_MSG;
                }
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
                        if(event_setmastermode_request(event_listeners(), boost::make_tuple(ci->clientnum, mastermodename(mastermode), mastermodename(mm))))
                        {
                            break;
                        }
                        set_mastermode_cn(mm, ci->clientnum);
                    }
                    else
                    {
                        defformatstring(s, "mastermode %d is disabled on this server", mm);
                        sendf(sender, 1, "ris", N_SERVMSG, s);
                    }
                }
                break;
            }

            case N_CLEARBANS:
            {
                if(ci->privilege)
                {
                    event_clearbans_request(event_listeners(), boost::make_tuple(ci->clientnum));
                }
                break;
            }

            case N_KICK:
            {
                int victim = getint(p);
                getstring(text, p);
                filtertext(text, text);
                trykick(ci, victim, text);
                break;
            }

            case N_SPECTATOR:
            {
                int spectator = getint(p), val = getint(p);
                bool self = spectator == sender;
                if(!ci->privilege && (!self || (ci->state.state==CS_SPECTATOR && mastermode>=MM_LOCKED) || (ci->state.state == CS_SPECTATOR && !val && !ci->allow_self_unspec) || message::limit(ci, &ci->n_spec_millis, message::resend_time::spec, "spectator status change"))) break;
                clientinfo *spinfo = (clientinfo *)getclientinfo(spectator); // no bots
                if(!spinfo || (spinfo != ci && spinfo->spy)) break;
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
                filtertext(text, text, false, false, MAXTEAMLEN);
                if(!ci->privilege && !ci->local) break;
                clientinfo *wi = getinfo(who);
                if(!m_teammode || !text[0] || !wi || !wi->connected || !strcmp(wi->team, text)) break;
                convert2utf8 newteamutf8(text);
                convert2utf8 oldteamutf8(wi->team);
                if((!smode || smode->canchangeteam(wi, wi->team, text)) && 
                    !message::limit(ci, &ci->n_switchname_millis, message::resend_time::switchteam, "N_SWITCHTEAM") &&
                    event_chteamrequest(event_listeners(), boost::make_tuple(wi->clientnum, oldteamutf8.str(), newteamutf8.str(), sender)) == false &&
                    addteaminfo(text))
                {
                    if(smode && wi->state.state==CS_ALIVE) suicide(wi);
                    event_reteam(event_listeners(), boost::make_tuple(wi->clientnum, oldteamutf8.str(), newteamutf8.str()));
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
                sendservmsgf("demo recording is %s for next match", demonextmatch ? "enabled" : "disabled");
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
                    sendservmsgf("[%s is getting the map]", colorname(ci));
                    if((ci->getmap = sendfile(sender, 2, mapdata, "ri", N_SENDMAP)))
                        ci->getmap->freeCallback = freegetmap;
                    ci->needclipboard = totalmillis ? totalmillis : 1;
                }
                break;

            case N_NEWMAP:
            {
                int size = getint(p);
                if(!ci->privilege && !ci->local && ci->state.state==CS_SPECTATOR) break;
                if(message::limit(ci, &ci->n_newmap_millis, message::resend_time::newmap, "map change")) break;
                if(event_editpacket(event_listeners(), boost::make_tuple(ci->clientnum))) return;
                if(size>=0)
                {
                    smapname[0] = '\0';
                    resetitems();
                    notgotitems = false;
                    if(smode) smode->newmap();
                }
                QUEUE_MSG;
                break;
            }

            case N_SETMASTER:
            {
                int mn = getint(p), val = getint(p);
                getstring(text, p);
                
                if(mn != ci->clientnum)
                {
                    if(!ci->privilege && !ci->local) break;
                    clientinfo *minfo = (clientinfo *)getclientinfo(mn);
                    if(!minfo || !minfo->connected || (!ci->local && minfo->privilege >= ci->privilege) || (val && minfo->privilege)) break;
                    set_player_master(mn);
                }
                else setmaster(ci, val!=0, text);
                
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
                tryauth(ci, name, desc);
                break;
            }
            
            case N_AUTHKICK:
            {
                string desc, name;
                getstring(desc, p, sizeof(desc));
                getstring(name, p, sizeof(name));
                int victim = getint(p);
                getstring(text, p);
                filtertext(text, text);

                tryauth(ci, name, desc, victim);

                break;
            }


            case N_AUTHANS:
            {
                string desc, ans;
                getstring(desc, p, sizeof(desc));
                uint id = (uint)getint(p);
                getstring(ans, p, sizeof(ans));
                answerchallenge(ci, id, ans, desc);
                break;
            }

            case N_PAUSEGAME:
            {
                int val = getint(p);
                if(ci->privilege < (restrictpausegame ? PRIV_ADMIN : PRIV_MASTER) && !ci->local) break;
                pausegame(val > 0, ci);
                pausegame_owner = ci->clientnum;
                break;
            }
            
            case N_GAMESPEED:
            {
                int val = getint(p);
                if(ci->privilege < (restrictgamespeed ? PRIV_ADMIN : PRIV_MASTER) && !ci->local) break;
                changegamespeed(val, ci);
                break;
            }

            case N_COPY:
            {
                ci->cleanclipboard();
                ci->lastclipboard = totalmillis ? totalmillis : 1;
                goto genericmsg;
            }

            case N_PASTE:
            {
                if(ci->state.state!=CS_SPECTATOR) sendclipboard(ci);
                goto genericmsg;
            }

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

            case N_SERVCMD:
            {
                getstring(text, p);
                convert2utf8 utf8command(text);
                event_servcmd(event_listeners(), boost::make_tuple(ci->clientnum, utf8command.str()));
                break;
            }
            
            case N_CONNECT:
            {
                loopi(2) getstring(text, p);
                getint(p);
                break;
            }

            case N_REMIP:
            case N_EDITF:
            case N_EDITT:
            case N_EDITM:
            case N_FLIP:
            case N_ROTATE:
            case N_REPLACE:
            case N_DELCUBE:
            {
                int size = server::msgsizelookup(type);
                if(size<=0) {
                    if (anti_cheat_enabled) ci->ac.unknown_packet(type);
                    else disconnect_client(sender, DISC_MSGERR);
                    return;
                }
                loopi(size-1) getint(p);
                if(event_editpacket(event_listeners(), boost::make_tuple(ci->clientnum))) return;
                if(ci && cq && (ci != cq || ci->state.state!=CS_SPECTATOR)) { QUEUE_AI; QUEUE_MSG; }
                break;
            }

            #define PARSEMESSAGES 1
            #include "capture.h"
            #include "ctf.h"
            #include "collect.h"
            #undef PARSEMESSAGES
            
            case -1:
            {
                if (anti_cheat_enabled) ci->ac.unknown_packet(-1);
                else disconnect_client(sender, DISC_MSGERR);
                return;
            }
            
            case -2:
                disconnect_client(sender, DISC_OVERFLOW);
                return;
            
            default: genericmsg:
            {
                int size = server::msgsizelookup(type);
                if(size<=0) { 
                    if (anti_cheat_enabled) ci->ac.unknown_packet(type);
                    else disconnect_client(sender, DISC_MSGERR);
                    return;
                }
                loopi(size-1) getint(p);
                if(ci) switch(msgfilter[type])
                {
                    case 2: case 3: if(ci->state.state != CS_SPECTATOR) QUEUE_MSG; break;
                    default: if(cq && (ci != cq || ci->state.state!=CS_SPECTATOR)) { QUEUE_AI; QUEUE_MSG; } break;
                }
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
        if(req.remaining() && !getint(req))
        {
            if(!enable_extinfo) return;
            extserverinforeply(req, p);
            return;
        }
        
        int clients = numclients(-1, false, true);
        int maxclients_ = maxclients;
        if (spec_slots) maxclients_ += spec_count();
        
        putint(p, clients);
        putint(p, gamepaused || gamespeed != 100 ? 7 : 5);                   // number of attrs following
        putint(p, PROTOCOL_VERSION);    // a // generic attributes, passed back below
        putint(p, gamemode);            // b
        putint(p, m_timed ? max((gamelimit - gamemillis)/1000, 0) : 0);
        putint(p, (clients <= maxclients_ ? maxclients_ : clients));
        putint(p, serverpass[0] ? MM_PASSWORD : (!m_mp(gamemode) ? MM_PRIVATE : (mastermode || display_open ? mastermode : MM_AUTH) ));
        if(gamepaused || gamespeed != 100)
        {
            putint(p, gamepaused ? 1 : 0);
            putint(p, gamespeed);
        }
        sendstring(smapname, p);
        sendstring(serverdesc, p);
        sendserverinforeply(p);
    }

    bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np)
    {
        return attr.length() && attr[0]==PROTOCOL_VERSION;
    }
    
    #include "hopmod/server_functions.cpp"
    
    #include "aiman.h"
} //namespace server
