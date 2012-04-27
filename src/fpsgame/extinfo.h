
#define EXT_ACK                         -1
#define EXT_VERSION                     105
#define EXT_NO_ERROR                    0
#define EXT_ERROR                       1
#define EXT_PLAYERSTATS_RESP_IDS        -10
#define EXT_PLAYERSTATS_RESP_STATS      -11
#define EXT_UPTIME                      0
#define EXT_PLAYERSTATS                 1
#define EXT_TEAMSCORE                   2

#define EXT_HOPMOD                      -2 // Case to identify Hopmod based servers

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
        putint(q, (currentmaster == ci->clientnum ? ci->privilege : PRIV_NONE)); 
        putint(q, ci->state.state);
        uint ip = getclientip(ci->clientnum);
        /* hopmod extension */
        putint(q, EXT_HOPMOD);
        putint(q, ci->state.suicides);
        putint(q, ci->state.shotdamage);
        putint(q, ci->state.damage);
        putint(q, ci->state.explosivedamage);
        putint(q, ci->state.hits);
        putint(q, ci->state.misses);
        putint(q, ci->state.shots);
        q.put((uchar*)&ip, ext_admin_client ? 4 : 3);
        if(ext_admin_client)
        {
            putint(q, (totalmillis - ci->connectmillis)/1000);
            q.put(mcrc != (uint)ci->mapcrc ? 1 : 0);
            q.put(ci->spy ? 1 : 0);
        }
        sendserverinforeply(q);
    }

    void extinfoteams(ucharbuf &p)
    {
        putint(p, m_teammode ? 0 : 1);
        putint(p, gamemode);
        putint(p, max((gamelimit - gamemillis)/1000, 0));
        if(!m_teammode) return;

        vector<teamscore> scores;

        //most taken from scoreboard.h
        if(smode && smode->hidefrags()) 
        {
            smode->getteamscores(scores);
            loopv(clients) if(clients[i]->team[0])
            {
                clientinfo *ci = clients[i];
                teamscore *ts = NULL;
                loopvj(scores) if(!strcmp(scores[j].team, ci->team)) { ts = &scores[j]; break; }
                if(!ts) scores.add(teamscore(ci->team, 0));
            }
        }
        else
        {
            loopv(clients) if(clients[i]->team[0])
            {
                clientinfo *ci = clients[i];
                teamscore *ts = NULL;
                loopvj(scores) if(!strcmp(scores[j].team, ci->team)) { ts = &scores[j]; break; }
                if(!ts) scores.add(teamscore(ci->team, ci->state.frags));
                else ts->score += ci->state.frags;
            }
        }

        loopv(scores)
        {
            sendstring(scores[i].team, p);
            putint(p, scores[i].score);

            if(!smode || !smode->extinfoteam(scores[i].team, p))
                putint(p,-1); //no bases follow
        }
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
                putint(p, 0);
                putint(p, EXT_HOPMOD);
                putint(p, revision());
                sendstring(version(), p);
                break;
            }

            case EXT_HOPMOD:
            {
                putint(p, EXT_NO_ERROR);
                putint(p, revision());
                sendstring(version(), p);
                break;
            }	

            case EXT_PLAYERSTATS:
            {
                int cn = getint(req); //a special player, -1 for all
                
                /* hopmod extension */
                if(ext_admin_pass[0] && req.remaining())
                {
                    char text[MAXSTRLEN];
                    getstring(text, req, MAXSTRLEN);
                    ext_admin_client = !strcmp(ext_admin_pass, text);
                }

                clientinfo *ci = NULL;
                if(cn >= 0)
                {
                    loopv(clients) if(clients[i]->clientnum == cn) { ci = clients[i]; break; }
                    if(!ci || (ci->spy && !ext_admin_client))
                    {
                        putint(p, EXT_ERROR); //client requested by id was not found
                        sendserverinforeply(p);
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

