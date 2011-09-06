// capture.h: client and server state for capture gamemode
#ifndef PARSEMESSAGES

struct captureservmode : servmode
{
    static const int CAPTURERADIUS = 64;
    static const int CAPTUREHEIGHT = 24;
    static const int OCCUPYBONUS = 1;
    static const int OCCUPYPOINTS = 1;
    static const int OCCUPYENEMYLIMIT = 28;
    static const int OCCUPYNEUTRALLIMIT = 14;
    static const int SCORESECS = 10;
    static const int AMMOSECS = 15;
    static const int REGENSECS = 1;
    static const int REGENHEALTH = 10;
    static const int REGENARMOUR = 10;
    static const int REGENAMMO = 20;
    static const int MAXAMMO = 5;
    static const int REPAMMODIST = 32;
    static const int RESPAWNSECS = 5;
    static const int MAXBASES = 100;

    struct baseinfo
    {
        vec o;
        string owner, enemy;
        int ammogroup, ammotype, ammo, owners, enemies, converted, capturetime;

        baseinfo() { reset(); }

        void noenemy()
        {
            enemy[0] = '\0';
            enemies = 0;
            converted = 0;
        }

        void reset()
        {
            noenemy();
            owner[0] = '\0';
            capturetime = -1;
            ammogroup = 0;
            ammotype = 0;
            ammo = 0;
            owners = 0;
        }

        bool enter(const char *team)
        {
            if(!strcmp(owner, team))
            {
                owners++;
                return false;
            }
            if(!enemies)
            {
                if(strcmp(enemy, team))
                {
                    converted = 0;
                    copystring(enemy, team);
                }
                enemies++;
                return true;
            }
            else if(strcmp(enemy, team)) return false;
            else enemies++;
            return false;
        }

        bool steal(const char *team)
        {
            return !enemies && strcmp(owner, team);
        }

        bool leave(const char *team)
        {
            if(!strcmp(owner, team) && owners > 0)
            {
                owners--;
                return false;
            }
            if(strcmp(enemy, team) || enemies <= 0) return false;
            enemies--;
            return !enemies;
        }

        int occupy(const char *team, int units)
        {
            if(strcmp(enemy, team)) return -1;
            converted += units;
            if(units<0)
            {
                if(converted<=0) noenemy();
                return -1;
            }
            else if(converted<(owner[0] ? int(OCCUPYENEMYLIMIT) : int(OCCUPYNEUTRALLIMIT))) return -1;
            if(owner[0]) { owner[0] = '\0'; converted = 0; copystring(enemy, team); return 0; }
            else { copystring(owner, team); ammo = 0; capturetime = 0; owners = enemies; noenemy(); return 1; }
        }

        bool addammo(int i)
        {
            if(ammo>=MAXAMMO) return false;
            ammo = min(ammo+i, int(MAXAMMO));
            return true;
        }

        bool takeammo(const char *team)
        {
            if(strcmp(owner, team) || ammo<=0) return false;
            ammo--;
            return true;
        }
    };

    vector<baseinfo> bases;

    struct score
    {
        string team;
        int total;
    };

    vector<score> scores;

    int captures;

    void resetbases()
    {
        bases.shrink(0);
        scores.shrink(0);
        captures = 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(const char *team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(!strcmp(cs.team, team)) return cs.total;
        }
        return 0;
    }

    void getteamscores(vector<teamscore> &teamscores)
    {
        loopv(scores) teamscores.add(teamscore(scores[i].team, scores[i].total));
    }

    score &findscore(const char *team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(!strcmp(cs.team, team)) return cs;
        }
        score &cs = scores.add();
        copystring(cs.team, team);
        cs.total = 0;
        return cs;
    }

    void addbase(int ammotype, const vec &o)
    {
        if(bases.length() >= MAXBASES) return;
        baseinfo &b = bases.add();
        b.ammogroup = min(ammotype, 0);
        b.ammotype = ammotype > 0 ? ammotype : rnd(5)+1;
        b.o = o;

        if(b.ammogroup)
        {
            loopi(bases.length()-1) if(b.ammogroup == bases[i].ammogroup)
            {
                b.ammotype = bases[i].ammotype;
                return;
            }
            int uses[5] = { 0, 0, 0, 0, 0 };
            loopi(bases.length()-1) if(bases[i].ammogroup)
            {
                loopj(i) if(bases[j].ammogroup == bases[i].ammogroup) goto nextbase;
                uses[bases[i].ammotype-1]++;
                nextbase:;
            }
            int mintype = 0;
            loopi(5) if(uses[i] < uses[mintype]) mintype = i;
            int numavail = 0, avail[5];
            loopi(5) if(uses[i] == uses[mintype]) avail[numavail++] = i+1;
            b.ammotype = avail[rnd(numavail)];
        }
    }

    void initbase(int i, int ammotype, const char *owner, const char *enemy, int converted, int ammo)
    {
        if(!bases.inrange(i)) return;
        baseinfo &b = bases[i];
        b.ammotype = ammotype;
        copystring(b.owner, owner);
        copystring(b.enemy, enemy);
        b.converted = converted;
        b.ammo = ammo;
    }

    bool hasbases(const char *team)
    {
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.owner[0] && !strcmp(b.owner, team)) return true;
        }
        return false;
    }

    float disttoenemy(baseinfo &b)
    {
        float dist = 1e10f;
        loopv(bases)
        {
            baseinfo &e = bases[i];
            if(e.owner[0] && strcmp(b.owner, e.owner))
                dist = min(dist, b.o.dist(e.o));
        }
        return dist;
    }

    bool insidebase(const baseinfo &b, const vec &o)
    {
        float dx = (b.o.x-o.x), dy = (b.o.y-o.y), dz = (b.o.z-o.z);
        return dx*dx + dy*dy <= CAPTURERADIUS*CAPTURERADIUS && fabs(dz) <= CAPTUREHEIGHT;
    }

#ifdef SERVMODE
    bool notgotbases;

    captureservmode() : captures(0), notgotbases(false) {}

    void reset(bool empty)
    {
        resetbases();
        notgotbases = !empty;
    }

    void cleanup()
    {
        reset(false);
    }

    void setup()
    {
        reset(false);
        if(notgotitems || ments.empty()) return;
        loopv(ments)
        {
            entity &e = ments[i];
            if(e.type != BASE) continue;
            int ammotype = e.attr1;
            addbase(ammotype>=GUN_SG && ammotype<=GUN_PISTOL ? ammotype : min(ammotype, 0), e.o); 
        }
        notgotbases = false;
        sendbases();
        loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
    }

    void newmap()
    {
        reset(true);
    }

    void stealbase(int n, const char *team)
    {
        baseinfo &b = bases[n];
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_ALIVE && ci->team[0] && !strcmp(ci->team, team) && insidebase(b, ci->state.o))
                b.enter(ci->team);
        }
        sendbaseinfo(n);
    }

    void replenishammo(clientinfo *ci)
    {
        if(m_noitems || notgotbases || ci->state.state!=CS_ALIVE || !ci->team[0]) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.ammotype>0 && b.ammotype<=I_CARTRIDGES-I_SHELLS+1 && insidebase(b, ci->state.o) && !ci->state.hasmaxammo(b.ammotype-1+I_SHELLS) && b.takeammo(ci->team))
            {
                sendbaseinfo(i);
                sendf(-1, 1, "riii", N_REPAMMO, ci->clientnum, b.ammotype);
                ci->state.addammo(b.ammotype);
                break;
            }
        }
    }

    void movebases(const char *team, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip)
    {
        if(!team[0] || gamemillis>=gamelimit) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            bool leave = !oldclip && insidebase(b, oldpos),
                 enter = !newclip && insidebase(b, newpos);
            if(leave && !enter && b.leave(team)) sendbaseinfo(i);
            else if(enter && !leave && b.enter(team)) sendbaseinfo(i);
            else if(leave && enter && b.steal(team)) stealbase(i, team);
        }
    }

    void leavebases(const char *team, const vec &o)
    {
        movebases(team, o, false, vec(-1e10f, -1e10f, -1e10f), true);
    }

    void enterbases(const char *team, const vec &o)
    {
        movebases(team, vec(-1e10f, -1e10f, -1e10f), true, o, false);
    }

    void addscore(int base, const char *team, int n)
    {
        if(!n) return;
        score &cs = findscore(team);
        cs.total += n;
        sendf(-1, 1, "riisi", N_BASESCORE, base, team, cs.total);
        event_scoreupdate(event_listeners(), boost::make_tuple(team, cs.total));
    }

    void regenowners(baseinfo &b, int ticks)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_ALIVE && ci->team[0] && !strcmp(ci->team, b.owner) && insidebase(b, ci->state.o))
            {
                bool notify = false;
                if(ci->state.health < ci->state.maxhealth)
                {
                    ci->state.health = min(ci->state.health + ticks*REGENHEALTH, ci->state.maxhealth);
                    notify = true;
                }
                if(ci->state.armour < itemstats[I_GREENARMOUR-I_SHELLS].max)
                {
                    ci->state.armour = min(ci->state.armour + ticks*REGENARMOUR, itemstats[I_GREENARMOUR-I_SHELLS].max);
                    notify = true;
                }
                if(b.ammotype>0)
                {
                    int ammotype = b.ammotype-1+I_SHELLS;
                    if(ammotype<=I_CARTRIDGES && !ci->state.hasmaxammo(ammotype))
                    {
                        ci->state.addammo(b.ammotype, ticks*REGENAMMO, 100);
                        notify = true;
                    }
                }
                if(notify)
                    sendf(-1, 1, "ri6", N_BASEREGEN, ci->clientnum, ci->state.health, ci->state.armour, b.ammotype, b.ammotype>0 ? ci->state.ammo[b.ammotype] : 0);
            }
        }
    }

    void update()
    {
        if(gamemillis>=gamelimit) return;
        endcheck();
        int t = gamemillis/1000 - (gamemillis-curtime)/1000;
        if(t<1) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.enemy[0])
            {
                if(!b.owners || !b.enemies) b.occupy(b.enemy, OCCUPYBONUS*(b.enemies ? 1 : -1) + OCCUPYPOINTS*(b.enemies ? b.enemies : -(1+b.owners))*t);
                sendbaseinfo(i);
            }
            else if(b.owner[0])
            {
                b.capturetime += t;

                int score = b.capturetime/SCORESECS - (b.capturetime-t)/SCORESECS;
                if(score) addscore(i, b.owner, score);

                if(m_regencapture)
                {
                    int regen = b.capturetime/REGENSECS - (b.capturetime-t)/REGENSECS;
                    if(regen) regenowners(b, regen);
                }
                else
                {
                    int ammo = b.capturetime/AMMOSECS - (b.capturetime-t)/AMMOSECS;
                    if(ammo && b.addammo(ammo)) sendbaseinfo(i);
                }
            }
        }
    }

    void sendbaseinfo(int i)
    {
        baseinfo &b = bases[i];
        sendf(-1, 1, "riissii", N_BASEINFO, i, b.owner, b.enemy, b.enemy[0] ? b.converted : 0, b.owner[0] ? b.ammo : 0);
    }

    void sendbases()
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        initclient(NULL, p, false);
        sendpacket(-1, 1, p.finalize());
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        if(connecting)
        {
            loopv(scores)
            {
                score &cs = scores[i];
                putint(p, N_BASESCORE);
                putint(p, -1);
                sendstring(cs.team, p);
                putint(p, cs.total);
            }
        }
        putint(p, N_BASES);
        putint(p, bases.length());
        loopv(bases)
        {
            baseinfo &b = bases[i];
            putint(p, min(max(b.ammotype, 1), I_CARTRIDGES+1));
            sendstring(b.owner, p);
            sendstring(b.enemy, p);
            putint(p, b.converted);
            putint(p, b.ammo);
        }
    }

    void endcheck()
    {
        const char *lastteam = NULL;

        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.owner[0])
            {
                if(!lastteam) lastteam = b.owner;
                else if(strcmp(lastteam, b.owner))
                {
                    lastteam = NULL;
                    break;
                }
            }
            else
            {
                lastteam = NULL;
                break;
            }
        }

        if(!lastteam) return;
        findscore(lastteam).total = 10000;
        sendf(-1, 1, "riisi", N_BASESCORE, -1, lastteam, 10000);
        event_scoreupdate(event_listeners(), boost::make_tuple(lastteam, 10000));
        startintermission();
    }

    void entergame(clientinfo *ci)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        enterbases(ci->team, ci->state.o);
    }

    void spawned(clientinfo *ci)
    {
        if(notgotbases || ci->gameclip) return;
        enterbases(ci->team, ci->state.o);
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        leavebases(ci->team, ci->state.o);
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(ci->team, ci->state.o);
    }

    void moved(clientinfo *ci, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip)
    {
        if(notgotbases) return;
        movebases(ci->team, oldpos, oldclip, newpos, newclip);
    }

    void changeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(oldteam, ci->state.o);
        enterbases(newteam, ci->state.o);
    }

    void parsebases(ucharbuf &p, bool commit, int sender=-1)
    {
        int numbases = getint(p);
        
        if (anti_cheat_enabled && !notgotbases && sender > -1)
        {
            bool modified = false;
            loopi(numbases)
            {
                int ammotype = getint(p);
                vec o;
                loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
                if(p.overread()) break;
                if (!bases.inrange(i) || bases[i].ammotype != ammotype || !vec_equal(bases[i].o, o)) modified = true;
            }
            if (modified)
            {
                clientinfo *ci = getinfo(sender);
                if (ci) ci->ac.modified_capture_bases();
            }
            return;
        }

        loopi(numbases)
        {
            int ammotype = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(commit && notgotbases) addbase(ammotype>=GUN_SG && ammotype<=GUN_PISTOL ? ammotype : min(ammotype, 0), o);
        }
        if(commit && notgotbases)
        {
            notgotbases = false;
            sendbases();
            loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
        }
    }

    bool extinfoteam(const char *team, ucharbuf &p)
    {
        int numbases = 0;
        loopvj(bases) if(!strcmp(bases[j].owner, team)) numbases++;
        putint(p, numbases);
        loopvj(bases) if(!strcmp(bases[j].owner, team)) putint(p, j);
        return true;
    }
};

#endif

#elif SERVMODE

case N_BASES:
    if(smode==&capturemode) capturemode.parsebases(p, (ci->state.state!=CS_SPECTATOR || ci->privilege || ci->local) && !strcmp(ci->clientmap, smapname), sender);
    break;

case N_REPAMMO:
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&capturemode) capturemode.replenishammo(cq);
    break;

#endif
