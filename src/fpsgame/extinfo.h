
#define EXT_ACK                         -1
#define EXT_VERSION                     105
#define EXT_NO_ERROR                    0
#define EXT_ERROR                       1
#define EXT_PLAYERSTATS_RESP_IDS        -10
#define EXT_PLAYERSTATS_RESP_STATS      -11
#define EXT_UPTIME                      0
#define EXT_PLAYERSTATS                 1
#define EXT_TEAMSCORE                   2

#define EXT_SUCKERSERV                  -5 // Case to identify SuckerServ based servers
#define EXT_SUCKERSERV_VERSION          1  // bump when changing SuckerServ related extensions

/*
    Client:
    -----
    A: 0 EXT_UPTIME
    B: 0 EXT_PLAYERSTATS cn #a client number or -1 for all players#
    C: 0 EXT_TEAMSCORE

    Server:  
    --------
    A: 0 EXT_UPTIME EXT_ACK EXT_VERSION uptime #in seconds#
    B: 0 EXT_PLAYERSTATS cn #send by client# EXT_ACK EXT_VERSION 0 or 1 #error, if cn was > -1 and client does not exist# ...
         EXT_PLAYERSTATS_RESP_IDS pid(s) #1 packet#
         EXT_PLAYERSTATS_RESP_STATS pid playerdata #1 packet for each player#
    C: 0 EXT_TEAMSCORE EXT_ACK EXT_VERSION 0 or 1 #error, no teammode# remaining_time gamemode loop(teamdata [numbases bases] or -1)

    Errors:
    --------------
    B:C:default: 0 command EXT_ACK EXT_VERSION EXT_ERROR
*/
    static bool ext_admin_client = false;
    static bool ext_hopmod_request = false;
    extern string ext_admin_pass;

    void extinfoplayer(ucharbuf &p, clientinfo *ci)
    {
        ucharbuf q = p;
        putint(q, EXT_PLAYERSTATS_RESP_STATS); // send player stats following
        putint(q, ci->clientnum); //add player id
        putint(q, ci->ping);
        sendstring(ci->name, q);
        sendstring(ci->team, q);
        putint(q, ci->state.frags);
        putint(q, ci->state.flags);
        putint(q, ci->state.deaths);
        putint(q, ci->state.teamkills);
        putint(q, ci->state.damage*100/max(ci->state.shotdamage,1));
        putint(q, ci->state.health);
        putint(q, ci->state.armour);
        putint(q, ci->state.gunselect);
        putint(q, ci->privilege);
        putint(q, ci->state.state);
        uint ip = getclientip(ci->clientnum);
        q.put((uchar*)&ip, 3);
        /* hopmod extension */
        if(ext_admin_client || ext_hopmod_request)
            putint(q, !ext_admin_client ? -1 : (ip >> 24) & 0xFF); // send last byte as signed integer, -1 on error
        if(ext_hopmod_request)
        {
            putint(q, EXT_SUCKERSERV);
            putint(q, ci->state.suicides);
            putint(q, ci->state.shotdamage);
            putint(q, ci->state.damage);
            putint(q, ci->state.explosivedamage);
            putint(q, ci->state.hits);
            putint(q, ci->state.misses);
            putint(q, ci->state.shots);
            if(ext_admin_client)
            {
                putint(q, (totalmillis - ci->connectmillis)/1000);
                q.put(mcrc != (uint)ci->mapcrc ? 1 : 0);
                q.put(ci->spy ? 1 : 0);
            }
        }
        sendserverinforeply(q);
    }
    
    static inline void extinfoteamscore(ucharbuf &p, const char *team, int score)
    {
        sendstring(team, p);
        putint(p, score);
        if(!smode || !smode->extinfoteam(team, p))
            putint(p,-1); //no bases follow
    }

    void extinfoteams(ucharbuf &p)
    {
        putint(p, m_teammode ? 0 : 1);
        putint(p, gamemode);
        putint(p, max((gamelimit - gamemillis)/1000, 0));
        if(!m_teammode) return;

        vector<teamscore> scores;
        if(smode && smode->hidefrags()) smode->getteamscores(scores);
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state!=CS_SPECTATOR && ci->team[0] && scores.htfind(ci->team) < 0)
            {
                if(smode && smode->hidefrags()) scores.add(teamscore(ci->team, 0));
                else { teaminfo *ti = teaminfos.access(ci->team); scores.add(teamscore(ci->team, ti ? ti->frags : 0)); }
            }
        }
        loopv(scores) extinfoteamscore(p, scores[i].team, scores[i].score);
    }

    static inline const char *buildtime()
    {
        static string buf = {0};
        if(!buf[0]) formatstring(buf, "%s %s", hopmod::build_date(), hopmod::build_time());
        return buf;
    }

    void extserverinforeply(ucharbuf &req, ucharbuf &p)
    {
        int extcmd = getint(req); // extended commands  

        //Build a new packet
        putint(p, EXT_ACK); //send ack
        putint(p, EXT_VERSION); //send version of extended info

        switch(extcmd)
        {
            case EXT_UPTIME:
            {
                putint(p, totalsecs); //in seconds
                /* hopmod extension */
                if(req.remaining() && req.get() > 0)
                {
                    putint(p, EXT_SUCKERSERV);
                    putint(p, EXT_SUCKERSERV_VERSION);
                    putint(p, hopmod::revision());
                    sendstring(buildtime(), p);
                }
                break;
            }

            /* hopmod extension */
            case EXT_SUCKERSERV:
            {
                putint(p, EXT_NO_ERROR);
                putint(p, hopmod::revision());
                sendstring(buildtime(), p);
                break;
            }

            case EXT_PLAYERSTATS:
            {
                int cn = getint(req); //a special player, -1 for all
                
                /* hopmod extension */
                if(req.remaining())
                {
                    ext_hopmod_request = getint(req) > 0;
                    if(ext_admin_pass[0] && req.remaining())
                    {
                        char text[MAXSTRLEN];
                        getstring(text, req, MAXSTRLEN);
                        ext_admin_client = !strcmp(ext_admin_pass, text);
                    }
                }
                
                clientinfo *ci = NULL;
                if(cn >= 0)
                {
                    loopv(clients) if(clients[i]->clientnum == cn) { ci = clients[i]; break; }
                    if(!ci || (ci->spy && !ext_admin_client))
                    {
                        putint(p, EXT_ERROR); //client requested by id was not found
                        sendserverinforeply(p);
                        ext_admin_client = false; //hopmod extension
                        ext_hopmod_request = false; //hopmod extension
                        return;
                    }
                }

                putint(p, EXT_NO_ERROR); //so far no error can happen anymore
                
                ucharbuf q = p; //remember buffer position
                putint(q, EXT_PLAYERSTATS_RESP_IDS); //send player ids following
                if(ci) putint(q, ci->clientnum);
                else loopv(clients) if(!clients[i]->spy || (clients[i]->spy && ext_admin_client)) putint(q, clients[i]->clientnum);
                sendserverinforeply(q);
            
                if(ci) extinfoplayer(p, ci);
                else loopv(clients) if(!clients[i]->spy || (clients[i]->spy && ext_admin_client)) extinfoplayer(p, clients[i]);
                ext_admin_client = false; //hopmod extension
                ext_hopmod_request = false; //hopmod extension
                return;
            }

            case EXT_TEAMSCORE:
            {
                extinfoteams(p);
                break;
            }

            default:
            {
                putint(p, EXT_ERROR);
                break;
            }
        }
        sendserverinforeply(p);
    }

