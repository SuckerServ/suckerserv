#ifndef PARSEMESSAGES

#define ctfteamflag(s) (!strcmp(s, "good") ? 1 : (!strcmp(s, "evil") ? 2 : 0))
#define ctfflagteam(i) (i==1 ? "good" : (i==2 ? "evil" : NULL))

struct ctfservmode : servmode
{
    static const int BASERADIUS = 64;
    static const int BASEHEIGHT = 24;
    static const int MAXFLAGS = 20;
    static const int FLAGRADIUS = 16;
    static const int FLAGLIMIT = 10;
    static const int MAXHOLDSPAWNS = 100;
    static const int HOLDSECS = 20;
    static const int HOLDFLAGS = 1;

    struct flag
    {
        int id, version, spawnindex;
        vec droploc, spawnloc;
        int team, droptime, owntime;
        int owner, dropper, invistime;
        int tmillis, drops;

        flag() { reset(); }

        void reset()
        {
            version = 0;
            spawnindex = -1;
            droploc = spawnloc = vec(0, 0, 0);
            owner = dropper = -1;
            invistime = owntime = 0;
            team = 0;
            droptime = owntime = 0;
            tmillis = -1;
            drops = 0;
        }
    };

    struct holdspawn
    {
        vec o;
    };
    vector<holdspawn> holdspawns;
    vector<flag> flags;
    int scores[2];

    void resetflags()
    {
        holdspawns.shrink(0);
        flags.shrink(0);
        loopk(2) scores[k] = 0;
    }

    bool addflag(int i, const vec &o, int team, int invistime = 0)
    {
        if(i<0 || i>=MAXFLAGS) return false;
        while(flags.length()<=i) flags.add();
        flag &f = flags[i];
        f.reset();
        f.id = i;
        f.team = team;
        f.spawnloc = o;
        f.invistime = invistime;
        return true;
    }

    bool addholdspawn(const vec &o)
    {
        if(holdspawns.length() >= MAXHOLDSPAWNS) return false;
        holdspawn &h = holdspawns.add();
        h.o = o;
        return true;
    }

    void ownflag(int i, int owner, int owntime)
    {
        flag &f = flags[i];
        f.owner = owner;
        f.owntime = owntime;
        f.dropper = -1;
        f.invistime = 0;
    }

    void dropflag(int i, const vec &o, int droptime, int dropper = -1)
    {
        flag &f = flags[i];
        f.droploc = o;
        f.droptime = droptime;
        f.dropper = dropper;
        f.owner = -1;
        f.invistime = 0;
        f.tmillis = -1;
        f.drops++;
    }

    void returnflag(int i, int invistime = 0)
    {
        flag &f = flags[i];
        f.droptime = 0;
        f.owner = f.dropper = -1;
        f.invistime = invistime;
        f.tmillis = -1;
        f.drops = 0;
    }

    int totalscore(int team)
    {
        return team >= 1 && team <= 2 ? scores[team-1] : 0;
    }

    int setscore(int team, int score)
    {
        if(team >= 1 && team <= 2) return scores[team-1] = score;
        return 0;
    }

    int addscore(int team, int score)
    {
        if(team >= 1 && team <= 2) return scores[team-1] += score;
        return 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(const char *team)
    {
        return totalscore(ctfteamflag(team));
    }

    void getteamscores(vector<teamscore> &tscores)
    {
        loopk(2) if(scores[k]) tscores.add(teamscore(ctfflagteam(k+1), scores[k]));
    }

    bool insidebase(const flag &f, const vec &o)
    {
        float dx = (f.spawnloc.x-o.x), dy = (f.spawnloc.y-o.y), dz = (f.spawnloc.z-o.z);
        return dx*dx + dy*dy <= BASERADIUS*BASERADIUS && fabs(dz) <= BASEHEIGHT;
    }

    static const int RESETFLAGTIME = 10000;
    static const int INVISFLAGTIME = 20000;

    bool notgotflags;

    ctfservmode() : notgotflags(false) {}

    void reset(bool empty)
    {
        resetflags();
        notgotflags = !empty;
    }

    void cleanup()
    {
        reset(false);
    }

    void setupholdspawns()
    {
        if(!m_hold || holdspawns.empty()) return;
        while(flags.length() < HOLDFLAGS)
        {
            int i = flags.length();
            if(!addflag(i, vec(0, 0, 0), 0, 0)) break;
            flag &f = flags[i];
            spawnflag(i);
            sendf(-1, 1, "ri6", N_RESETFLAG, i, ++f.version, f.spawnindex, 0, 0);
            event_resetflag(event_listeners(), boost::make_tuple(ctfflagteam(f.team)));
        }
    }

    void setup()
    {
        reset(false);
        if(notgotitems || ments.empty()) return;
        if(m_hold) 
        {
            loopv(ments)
            {   
                entity &e = ments[i];
                if(e.type != BASE) continue;
                if(!addholdspawn(e.o)) break;
            }
            setupholdspawns();
        }
        else loopv(ments)
        {
            entity &e = ments[i];
            if(e.type != FLAG || e.attr2 < 1 || e.attr2 > 2) continue;
            if(!addflag(flags.length(), e.o, e.attr2, m_protect ? lastmillis : 0)) break;
        }
        notgotflags = false;
    }

    void newmap()
    {
        reset(true);
    }

    void dropflag(clientinfo *ci, clientinfo *dropper = NULL)
    {
        if(notgotflags) return;
        loopv(flags) if(flags[i].owner==ci->clientnum)
        {
            flag &f = flags[i];
            if(m_protect && insidebase(f, ci->state.o))
            {
                returnflag(i);
                sendf(-1, 1, "ri4", N_RETURNFLAG, ci->clientnum, i, ++f.version);
                event_returnflag(event_listeners(), boost::make_tuple(ci->clientnum, ctfflagteam(f.team)));
            }
            else
            {
                ivec o(vec(ci->state.o).mul(DMF));
                sendf(-1, 1, "ri7", N_DROPFLAG, ci->clientnum, i, ++f.version, o.x, o.y, o.z);
                dropflag(i, ci->state.o, lastmillis, dropper ? dropper->clientnum : ci->clientnum);
                event_dropflag(event_listeners(), boost::make_tuple(ci->clientnum, ctfflagteam(f.team)));
            }
        }
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        dropflag(ci);
        loopv(flags) if(flags[i].dropper == ci->clientnum) flags[i].dropper = -1;
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        dropflag(ci, ctftkpenalty && actor && actor != ci && isteam(actor->team, ci->team) ? actor : NULL);
        loopv(flags) if(flags[i].dropper == ci->clientnum)
        { 
            flags[i].droploc = ci->state.o;
            flags[i].dropper = -1;
        }
    }

    bool canchangeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        return ctfteamflag(newteam) > 0;
    }

    void changeteam(clientinfo *ci, const char *oldteam, const char *newteam)
    {
        dropflag(ci);
    }

    void spawnflag(int i)
    {
        if(holdspawns.empty()) return;
        int spawnindex = flags[i].spawnindex;
        loopj(4)
        {
            spawnindex = rnd(holdspawns.length());
            if(spawnindex != flags[i].spawnindex) break;
        }
        flags[i].spawnindex = spawnindex;
    }

    void scoreflag(clientinfo *ci, int goal, int relay = -1)
    {
        int flagIndex = relay >= 0 ? relay : goal;
        int timetrial = flags[flagIndex].tmillis > -1 ? totalmillis - flags[flagIndex].tmillis : -1;

        if (!ci->timetrial || timetrial < ci->timetrial) ci->timetrial = timetrial;

        returnflag(relay >= 0 ? relay : goal, m_protect ? lastmillis : 0);
        ci->state.flags++;
        int team = ctfteamflag(ci->team), score = addscore(team, 1);
        if(m_hold) spawnflag(goal);
        sendf(-1, 1, "rii9", N_SCOREFLAG, ci->clientnum, relay, relay >= 0 ? ++flags[relay].version : -1, goal, ++flags[goal].version, flags[goal].spawnindex, team, score, ci->state.flags);
        event_scoreflag(event_listeners(), boost::make_tuple(ci->clientnum, ci->team, score, timetrial));
        if(score == FLAGLIMIT)
        {
            startintermission();
        }
        else if(anti_cheat_enabled && score > FLAGLIMIT)
        {
            cheat(ci->clientnum, 1, score);
        }
    }

    void takeflag(clientinfo *ci, int i, int version)
    {
        if(notgotflags || !flags.inrange(i) || ci->state.state!=CS_ALIVE || !ci->team[0]) return;

        if(ci->ac.is_player_invisible()) return;

        flag &f = flags[i];

        vec flag_location = vec(0, 0, 0);

        bool flag_dropped = f.dropper > -1;

        if (m_ctf) flag_location = flag_dropped ? f.droploc : f.spawnloc;
        if ((m_hold || m_protect) && holdspawns.inrange(f.spawnindex))
            flag_location = flag_dropped ? f.droploc : holdspawns[f.spawnindex].o;

        if((m_hold ? f.spawnindex < 0 : !ctfflagteam(f.team)) || f.owner>=0 || f.version != version || (f.droptime && f.dropper == ci->clientnum)) return;
        int team = ctfteamflag(ci->team);
        if(m_hold || m_protect == (f.team==team))
        {
            loopvj(flags) if(flags[j].owner==ci->clientnum) return;
            ownflag(i, ci->clientnum, lastmillis);
            sendf(-1, 1, "ri4", N_TAKEFLAG, ci->clientnum, i, ++f.version);
            if (!f.drops) f.tmillis = totalmillis;
            event_takeflag(event_listeners(), boost::make_tuple(ci->clientnum, ctfflagteam(f.team)));
        }
        else if(m_protect)
        {
            if(!f.invistime) scoreflag(ci, i);
            return;
        }
        else if(f.droptime)
        {
            returnflag(i);
            sendf(-1, 1, "ri4", N_RETURNFLAG, ci->clientnum, i, ++f.version);
            event_returnflag(event_listeners(), boost::make_tuple(ci->clientnum, ctfflagteam(f.team)));
            return;
        }
        else
        {
            loopvj(flags) if(flags[j].owner==ci->clientnum) { scoreflag(ci, i, j); break; }
        }
        
        if (anti_cheat_enabled) 
        {
        /*
            float flag_dist = distance(flag_location, ci->state.o); 

            if (flag_dist >= 250)
            {
                defformatstring(debug)("%s -> flag location (%.2f/%.2f/%.2f) / player location (%.2f/%.2f/%.2f) / distance: %.2f / dropped: %s / team flag: %s",
                    ci->name,
                    flag_location.x, 
                    flag_location.y, 
                    flag_location.z,
                    ci->state.o.x,
                    ci->state.o.y,
                    ci->state.o.z,
                    flag_dist,
                    flag_dropped ? "Yes" : "No",
                    m_hold || m_protect || !strcmp(ci->team, ctfflagteam(f.team)) ? "Yes" : "No"
                );
                sendservmsg(debug);
                cheat(ci->clientnum, 10, (int)flag_dist);
            }
            */
            ci->ac.check_get_flag(distance(flag_location, ci->state.o));
        }
    }

    void update()
    {
        if(gamemillis>=gamelimit || notgotflags) return;
        loopv(flags)
        {
            flag &f = flags[i];
            if(f.owner<0 && f.droptime && lastmillis - f.droptime >= RESETFLAGTIME)
            {
                returnflag(i, m_protect ? lastmillis : 0);
                if(m_hold) spawnflag(i);
                sendf(-1, 1, "ri6", N_RESETFLAG, i, ++f.version, f.spawnindex, m_hold ? 0 : f.team, m_hold ? 0 : addscore(f.team, m_protect ? -1 : 0));
                event_resetflag(event_listeners(), boost::make_tuple(ctfflagteam(f.team)));
            }
            if(f.invistime && lastmillis - f.invistime >= INVISFLAGTIME)
            {
                f.invistime = 0;
                sendf(-1, 1, "ri3", N_INVISFLAG, i, 0);
            }
            if(m_hold && f.owner>=0 && lastmillis - f.owntime >= HOLDSECS*1000)
            {
                clientinfo *ci = getinfo(f.owner);
                if(ci) scoreflag(ci, i);
                else
                {
                    spawnflag(i);
                    sendf(-1, 1, "ri6", N_RESETFLAG, i, ++f.version, f.spawnindex, 0, 0);
                    event_resetflag(event_listeners(), boost::make_tuple(ctfflagteam(f.team)));
                }
            }
        }
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        putint(p, N_INITFLAGS);
        loopk(2) putint(p, scores[k]);
        putint(p, flags.length());
        loopv(flags)
        {
            flag &f = flags[i];
            putint(p, f.version);
            putint(p, f.spawnindex);
            putint(p, f.owner);
            putint(p, f.invistime ? 1 : 0);
            if(f.owner<0)
            {
                putint(p, f.droptime ? 1 : 0);
                if(f.droptime)
                {
                    putint(p, int(f.droploc.x*DMF));
                    putint(p, int(f.droploc.y*DMF));
                    putint(p, int(f.droploc.z*DMF));
                }
            }
        }
    }

    void parseflags(ucharbuf &p, bool commit, int sender=-1)
    {
        int numflags = getint(p);
        
        if (anti_cheat_enabled && !notgotflags && sender > -1)
        {
            bool modified = false;
            loopi(numflags)
            {
                int team = getint(p);
                vec o;
                loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
                if(p.overread()) break;
                if(true)
                {
                    if(m_hold) 
                    {
                        if (!holdspawns.inrange(i) || !vec_equal(holdspawns[i].o, o))
                        {
                            modified = true;
                        }
                    }
                    else 
                    {
                        if (!flags.inrange(i) || flags[i].team != team || !vec_equal(flags[i].spawnloc, o))
                        {
                            modified = true;
                        }
                    }
                }
            }
            if (modified)
            {
                clientinfo *ci = getinfo(sender);
                if (ci) ci->ac.modified_map_flags();
            }           
            return;
        }
        
        loopi(numflags)
        {
            int team = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(commit && notgotflags)
            {
                if(m_hold) addholdspawn(o);
                else addflag(i, o, team, m_protect ? lastmillis : 0);
            }
        }
        if(commit && notgotflags)
        {
            if(m_hold) setupholdspawns();
            notgotflags = false;
        }
    }
};

#elif SERVMODE

case N_TRYDROPFLAG:
{
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&ctfmode) ctfmode.dropflag(cq);
    break;
}

case N_TAKEFLAG:
{
    int flag = getint(p), version = getint(p);
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&ctfmode) ctfmode.takeflag(cq, flag, version);
    break;
}

case N_INITFLAGS:
    if(smode==&ctfmode) ctfmode.parseflags(p, (ci->state.state!=CS_SPECTATOR || ci->privilege || ci->local) && !strcmp(ci->clientmap, smapname), sender);
    break;

#endif
