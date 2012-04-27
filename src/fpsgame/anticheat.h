/*
 * Cheat Detection / Anticheat System (c) 2011 by Thomas
 * 
 * TODO: Check Jumppads / Teleporter IDs
 *
 */

#ifdef anticheat_helper_func

bool anti_cheat_enabled = true;
bool anti_cheat_add_log_to_demo = true;
int anti_cheat_system_rev = 3;
	
void cheat(int cn, int cheat, int info1, const char *info2)
{
	if(!anti_cheat_enabled) return;
    
    if(anti_cheat_add_log_to_demo && demorecord)
    {
        clientinfo *ci = getinfo(cn);
        if(ci)
        {
            char cheatinfo_demo[MAXTRANS];
            
            sprintf(cheatinfo_demo, 
               "\f3CHEAT DETECTED \f1-> \f3NAME: \f1%s \f3IP: \f1%s \f3TYPE: \f1%i \f3INFO1: \f1%i \f3INFO2: \f3%s",
                ci->name,
                ci->hostname(),
                cheat,
                info1,
                info2
            );
            
            const char *cheatinfo_demo_cc = cheatinfo_demo;
            
            vector<uchar> demoinfo;
            putint(demoinfo, N_SETTEAM);
            putint(demoinfo, 2048); // fake CN so the normal client ignores the packet
            while(*cheatinfo_demo_cc) putint(demoinfo, *cheatinfo_demo_cc++);
            putint(demoinfo, 0);
            putint(demoinfo, -1);
            recordpacket(1, demoinfo.getbuf(), demoinfo.length());
            
            /*
             * I am using N_SETTEAM cause I don't want "normal" clients to display such messages.
             * To see the messages, you need to change the N_SETTEAM case in client.cpp a little bit:
             *
             * if(wn == 2048 && reason == -1)
             * {
             *     conoutf(text);
             *     break;
             * }
             */
             
        }
    }
    
    event_cheat(event_listeners(), boost::make_tuple(cn, cheat, info1, info2));

}
    
int is_invisible(int cn)
{
    clientinfo *ci = getinfo(cn);
    if(!anti_cheat_enabled || !ci || (ci->state.state != CS_ALIVE && ci->state.state != CS_LAGGED)|| ci->ping >= 7500) 
        return false;
     
    int lag = totalmillis - ci->lastposupdate;
    if(lag == totalmillis) lag = -1;
    
    if(ci->state.o == vec(-1e10f, -1e10f, -1e10f) || ci->state.o == vec(0, 0, 0))
        return 2; // weird position hack
    
    if(lag >= 7500 || lag == -1)
        return 1;
    
    return 0;
}

float distance(vec a, vec b)
{
    int x = a.x - b.x, y = a.y - b.y, z = a.z - b.z;
    return sqrt(x*x + y*y + z*z);
}
    
bool vec_equal(vec a, vec b)
{
    loopi(3) if((int)a[i] != (int)b[i]) return false; 
    return true;
}

#endif 
#ifdef anticheat_class
 
extern bool anti_cheat_enabled;
extern int is_invisible(int);
extern void cheat(int cn, int cheat, int info1, const char *info2="");
extern vector<server_entity> sents;
extern clientinfo *getinfo(int n);

class anticheat
{
    public:

    int clientnum;
    int pingupdates, lastpingupdates, lastpingsnapshot, speedhack, speedhack_updates;
    int speedhack2, speedhack2_lastreset, speedhack2_strangedist_c;
    int positionhack;
    float speedhack2_dist, speedhack2_strangedist;
	bool lagged;
    int last_lag;
    int timetrial;
    int lastshoot, lastjumppad, lastteleport, lastdamage, lastspawn, spawns;
    int lastjump, lastgun;
    int jumphack, jumppads, jumphack_dist;
    int invisiblehack_count, invisible_first, invisible_last;
    float exceed_position; int ignore_exceed;
    int _ping;
    float last_fall_loc;
    float speedhack2_lastdist, jumpdist, lastjumphack_dist;
    bool was_falling;
    vec pos;
    bool initialized;
    

    void reset(int cn=-1)
    {
        if(!anti_cheat_enabled) 
        {
            initialized = false;
            return;
        }
        
        if(cn > -1) clientnum = cn;

        jumpdist = 0;
        last_fall_loc = 0;
        spawns = 0;
        lastgun = -1;
        exceed_position = 0;
        ignore_exceed = 0;
        
        reset_speedhack_dist();
        reset_speedhack_ping();
        reset_last_action();
        reset_jumphack();
        reset_invisible();
        reset_lag();
        
        initialized = true;
    }
    
    /*
     * Player Actions
     */

    void no_falling() { jumpdist = 0; try_decrease_jumphack(); }
    void shoot() { lastshoot = totalmillis; }
    void damage() { lastdamage = totalmillis; }
    void jumppad(int n) { lastjumppad = totalmillis; jumppads++; }
    void teleport(int n) { if(speedhack2_strangedist_c) { speedhack2_strangedist_c--; speedhack2_strangedist -= speedhack2_lastdist; } lastteleport = totalmillis; }
    void spawn() { reset_invisible(); reset_speedhack_dist(); reset_last_action(); reset_jumphack(); fix_items(); lastspawn = totalmillis; spawns++; }
    void ping() { check_ping(); }
    
    /*
     * Invisible 
     */
     
    bool is_player_invisible()
    {
        if(!initialized || (lastspawn > -1 && totalmillis - lastspawn < 2000)) return false;
        int is_invis = is_invisible(clientnum);
        
        switch (is_invis)
        {
            case 1:
                invisiblehack_count++;
                if(invisible_first == -1) invisible_first = totalmillis;
                invisible_last = totalmillis;
                if(invisiblehack_count >= 7) 
                {
                    int inv_time = invisible_last - invisible_first;
                    if(inv_time >= 2000) 
                    {
                        cheat(clientnum, 9, inv_time); 
                    }
                    reset_invisible();
                }
                break;
            case 2:
                positionhack++;
                if(positionhack >= 4) cheat(clientnum, 24, -1);   
                break;
            default:;    
        }

        return is_invis > 0;
    }
    
    /*
     * Speed Hack Position
     */

    void check_speedhack_dist(float dist, int lag)
    {
        if(!initialized) return;
        if(!lagged && dist >= 5.5) // DON'T CHANGE THIS VALUE!!!
        {          
            if(dist < 50) // < 12.5x
            {
                speedhack2++;
                speedhack2_dist += dist;
                speedhack2_lastdist = dist;
            }
            else
            {
                speedhack2_strangedist += dist;
                speedhack2_strangedist_c++;
            }
            
            int speed = is_speedhack_dist();
            /*if(speed  && speed / 100000 < 4 && _ping > 2000) 
            {
                reset_speedhack_dist();
                speed = 0;
            }
            else*/
            if(totalmillis - speedhack2_lastreset >= 45000)
            {
                reset_speedhack_dist();
            }
            
            if(speed) cheat(clientnum, 16, speed);
        }
    }
    
    /*
     * Jumphack
     */
    
    void check_jumphack(float dist, float fall_loc)
    {     
        if(!initialized) return;
        int last_action = last_player_action();
        
        if((last_action < 1500 && last_action != -1) || (lastjump > -1 && totalmillis - lastjump < 500)) 
        {
            jumpdist = 0;
            return;
        }

        if(dist > 0)
        {
            jumpdist = !was_falling ? (int)dist : jumpdist + (int)dist;
        }
        else  // falling down
        {
            if(jumpdist > 20) 
            {
                jumphack++;
                lastjumphack_dist = jumpdist;
                jumphack_dist += jumpdist;
                
                //defformatstring(debug)("jumphack %i %2.f (%d) %ix", clientnum, jumpdist, last_action, jumphack);
                //sendservmsg(debug);
                    
                jumpdist = 0;
                try_decrease_jumphack(); // big jumppads 
                
                int jdist = is_jumphack();
                if(jdist > 0)
                {
                    cheat(clientnum, 17, jdist);
                    reset_jumphack();
                }
                
                last_fall_loc = fall_loc;
            }
            lastjump = totalmillis;
        }
    }
    
    /*
     * Edit Mode
     */

    void edit_packet(int type)
    {
        cheat(clientnum, 2, type);
    }
    
    /*
     * Modified Map Items
     */
    
    void modified_map_items()
    {
        cheat(clientnum, 11, 0);
    }
    
    /*
     * Modified CTF Flags
     */
     
    void modified_map_flags()
    {
        cheat(clientnum, 12, 0);
    }
    
    /*
     * Modified Capture Bases
     */
    
    void modified_capture_bases()
    {
        cheat(clientnum, 13, 0);
    }
    
    /*
     * Weapon out of bounds
     */   

    void unknown_weapon(int gun)
    {
        cheat(clientnum, 4, gun);
    }
    
    /*
     * Unknown Sound
     */  

    void unknown_sound(int sound)
    {
        cheat(clientnum, 8, sound);
    }
    
    /*
     * Out of Gun Distance Range
     */  
    
    void out_of_gun_distance_range(int gun, const char *info)
    {
        cheat(clientnum, 14, gun, info);
    }
    
    /*
     * Unknown Network Traffic
     */
     
    void unknown_packet(int type)
    {
        cheat(clientnum, 3, type);
    }
    
    /*
     * Get Flag
     */
     
    void get_flag(float dist, bool score_flag, bool flag_owned)
    {
        cheat(clientnum, 10, (int)dist, flag_owned ? "stealflag" : score_flag ? "scoreflag" : "getflag");
    }
    
    /*
     * Unknown Item
     */    
    
    void unknown_item(int item, int len)
    {
        if(initialized && lastspawn > -1 && totalmillis - lastspawn >= 2000)
        {
            //if(!is_item_mode() && impossible(1, item)) return;
            defformatstring(info)("TRIED ITEM: %i ITEMLIST LENGTH: %i", item, len);
            cheat(clientnum, 19, 0, info);
        }
    }
    
    /*
     * Item Not Spawned
     */    
    
    void item_not_spawned(int item, int spawntime)
    {
        if(initialized && lastspawn > -1 && totalmillis - lastspawn >= 2000)
        {
            //if(!is_item_mode() && impossible(0, item)) return; // There are item pick ups in insta. You just don't see them.
            defformatstring(info)("TRIED ITEM: %i SPAWNTIME: %i", item, spawntime);
            cheat(clientnum, 20, 0, info);
        }
    }
    
    /*
     * FFA Weapons in INSTA modes
     */
     
    bool check_gun(int gunselect, bool is_spawn=false)
    {
        if(!initialized) return true;
        //if(mod_gamemode) return true;
        lastgun = gunselect;
        if(is_item_mode() || m_efficiency) return true;
        bool correct_gun = gunselect == GUN_RIFLE || gunselect == GUN_FIST;
        if(!correct_gun)
        {
            if(is_spawn)
            { 
                if(spawns < 3) correct_gun = true;
            }
            else
            { 
                if(lastspawn > -1 && totalmillis - lastspawn < 2000) correct_gun = true;
            }
        }
        if(!correct_gun) impossible(2, gunselect);
        return correct_gun;
    }
    
    /*
     * Player position exceeded
     */
    
    void player_position_exceeded()
    {
        //if(mod_gamemode) return;
        cheat(clientnum, 23, (int)exceed_position);
    }
    
    /*
     * Impossible Actions
     */
   
    bool impossible(int c, int info)
    {
        //if(mod_gamemode) return false;
        char cheat_info[1000];
        switch (c)
        {
            case 0: // item not spawned
            case 1: // item not inrange
                sprintf(cheat_info, "TRIED TO PICKUP ITEM IN NON INSTA MODE / OUT OF RANGE (%i)", info);
                break;
            case 2: // ffa gun in non insta mode
                sprintf(cheat_info, "TRIED TO SWITCH / SPAWN WITH A NON RIFLE / FIST GUN (%i)", info);
                break;
            case 3: // flag pick up in non flag mode
                sprintf(cheat_info, "TRIED TO PICK UP FLAG / SCORE IN NON FLAG MODE");
                break;
            default: return false;
        }
        cheat_info[sizeof(cheat_info)-1] = 0;
        cheat(clientnum, 22, 0, cheat_info);
        return true;
    }
    
    /*
     * Lag
     */
     
    void check_lag(int lag)
    {
        if(lag >= 500) 
        {
            lagged = true;
            last_lag = totalmillis;
            return;
        }
        
        if(last_lag > 0 && totalmillis >= (last_lag + 10000)) 
        {
            reset_lag();
        }
    }
    
    private:
    
    /*
     * Private Reset Functions
     */
    
    void reset_last_action()
    {
        lastshoot = lastjumppad = lastteleport = lastdamage = lastspawn = lastjump = -1;    
    }
    
    void reset_jumphack()
    {
        jumphack = 0; jumppads = 0;
        lastjumphack_dist = jumphack_dist = 0;
    }

    void reset_speedhack_dist()
    {
        speedhack2 = speedhack2_dist = jumpdist = 0;
        speedhack2_strangedist_c = speedhack2_strangedist = 0;
        speedhack2_lastdist = 0;
        was_falling = false;
        speedhack2_lastreset = totalmillis;
        pos = vec(-1e10f, -1e10f, -1e10f);
    }
    
    void reset_speedhack_ping()
    {
        speedhack = speedhack_updates = lastpingsnapshot = lastpingupdates = pingupdates = 0;
    }

    /*
     * Last Action
     */
    
    int last_player_action()
    {
        int shortest = lastshoot;
        if(lastjumppad > -1 && lastjumppad > shortest) shortest = lastjumppad;
        if(lastteleport > -1 && lastteleport > shortest) shortest = lastteleport;
        if(lastdamage > -1 && lastdamage > shortest) shortest = lastdamage;
        
        return shortest > -1 ? totalmillis - shortest : shortest;
    }
    
    int is_speedhack_dist()
    {
        if(speedhack2 >= 30) 
        {
            float speed = speedhack2_dist / (float)speedhack2 / (float)4;
            if(speed < 1.2) return 0;
            else reset_speedhack_dist();
            return (speed * (float)100000);
        }
        
        if(speedhack2_strangedist_c >= 10)
        {
            float speed = speedhack2_strangedist / (float)speedhack2_strangedist_c / (float)4;
            return (speed * (float)100000);// no check needed, this is anyways >= 12.5x
        }
        
        return 0;
    } 

    /*
     * Speed Hack Ping
     */
     
    void check_ping()
    {
        pingupdates++;
        if(pingupdates % 20 != 0) return; 
        if(lastpingsnapshot) 
        {
            int snapsec = (totalmillis - lastpingsnapshot) / 1000;
            if(snapsec)
            {
                int pingupdates = 20 / snapsec;
                if(pingupdates >= 5) // 2x from real time
                {
                    speedhack++;
                    speedhack_updates += pingupdates;
                }
            }                
            else // more than 20 updates / sec
            {
                speedhack++;
                int updates = pingupdates - lastpingupdates;
                speedhack_updates += updates > 0 ? updates : 20; 
            }
            if(speedhack >= 20) // trapped 20 times into the detection
            {
                float speed = ((float)speedhack_updates / (float)speedhack) / (float)4;
                if(speed >= 1.5)
                {
                    reset_speedhack_ping();
                    int speed2 = (speed * (float)100000);
                    cheat(clientnum, 6, speed2);
                }
            }
            lastpingupdates = pingupdates;
        }
        lastpingsnapshot = totalmillis;
    }    

    /*
     * Jumphack
     */ 

    int is_jumphack()
    {
        if(jumphack < 2) return 0;
        int avg_dist = jumphack_dist / (int)jumphack;
        return avg_dist >= 10 ? avg_dist : 0;
    }
    
    void try_decrease_jumphack()
    {
        if(jumppads)
        {
            jumppads--;
            if(jumphack) 
            {
                jumphack--;
                jumphack_dist -= (int)lastjumphack_dist;
            }
        }
    }
    
    /*
     * Invisible 
     */
     
    void reset_invisible()
    {
        invisiblehack_count = 0;
        invisible_first = -1;
        invisible_last = -1;    
        positionhack = 0;
    }
    
    /*
     * Is item mode
     */
     
    bool is_item_mode()
    {
        return !m_insta && !m_efficiency;
    }
    
    /*
     * FFA Item Bugfix
     */
    
    void fix_items()
    {
        if(clientnum < 0 || clientnum >= 128 || !getinfo(clientnum)) return;
        
        bool item_mode = is_item_mode();
        
        loopv(sents)
        { 
            if(!sents[i].spawned || !item_mode) 
            { 
                sendf(clientnum, 1, "ri3", N_ITEMACC, i, -1); 
            } 
        } 
    }
    
    /*
     * Lag
     */
     
    void reset_lag()
    {
        lagged = false;
        last_lag = 0; 
    }

};
    
#endif

#ifdef anticheat_parsepacket

#define AC_PROTOCOL_VERSION 258
#define ac_check_sender if(ca->clientnum != ci->clientnum && ca->ownernum != ci->clientnum) break;
#define ac_check_invis if((ca->state.state == CS_ALIVE || ca->state.state == CS_LAGGED) && ac->is_player_invisible()) ci->state.state = CS_LAGGED;

void anti_cheat_parsepacket(int type, clientinfo *ci, clientinfo *cq, packetbuf p)
{
    #if PROTOCOL_VERSION != AC_PROTOCOL_VERSION
    throw;
    #endif
 
    clientinfo *ca = cq ? cq : ci;
    anticheat *ac = &ca->ac;

    switch(type)
    {
    
        case N_JUMPPAD:
        case N_TELEPORT:
        {
            ca = getinfo(getint(p));
            if(!ca) break;
            ac_check_sender;
            ac_check_invis;
            if(ca->state.state != CS_ALIVE) break;
            if(type == N_TELEPORT) ac->teleport(getint(p));
            else ac->jumppad(getint(p));
            break;
        }
        
        case N_SOUND:
        {      
            int sound = getint(p);

            if(sound != S_JUMP && sound != S_LAND && sound != S_NOAMMO 
                && (m_capture && sound != S_ITEMAMMO)) 
            {
                ac->unknown_sound(sound);
                break;
            }
            if(sound == S_JUMP) ac_check_invis;
            break;
        }
        
        case N_SHOOT:
        {
            getint(p);
            ac_check_invis;
            ac->check_gun(getint(p));
            ac->shoot();
            break;
        }
        
        case N_EXPLODE:
            ac_check_invis;
            break;
            
        case N_ITEMPICKUP:
        {
            ac_check_invis;
            int n = getint(p);
            if(!sents.inrange(n)) 
            {
                ac->unknown_item(n, sents.length());
                break;
            }
            if(!sents[n].spawned && totalmillis - sents[n].lastpickup >= 1000)
            {   
                ac->item_not_spawned(n, sents[n].spawntime);
            }
            break;
        }
 
        case N_POS:
        {   
            ca = getinfo(getuint(p));
            if(!ca) break;
            
            ac_check_sender;
            
            ac = &ca->ac; 
            if(!ac->initialized) return;
            
            p.get();
            uint flags = getuint(p);
            vec pos;
            loopk(3)
            {
                int n = p.get(); n |= p.get()<<8; if(flags&(1<<k)) { n |= p.get()<<16; if(n&0x800000) n |= -1<<24; }
                pos[k] = n/DMF;
            }
            
            bool falling = flags&(1<<4);
            
            if(ca->state.state == CS_ALIVE)
            {
                /*
                 * Position based speed-hack detection. 
                 * Teleporters are giving false positives, that's why
                 * I am decreasing speedhack2 @ N_TELEPORT.
                 * Falling (= Jumping / Falling from somewhere) is excluded as well.
                 */
               
                int real_lag = totalmillis - ca->lastposupdate;
                int last_lag = ca->last_lag;

                if(ca->lastposupdate > 0) ac->check_lag(real_lag);

                if(pos != ac->pos && last_lag > 0 && real_lag > 0 && real_lag <= 35
                    && last_lag <= 35 && ca->lag <= 35 && ca->state.state == CS_ALIVE)
                {   

                    if(!ac->was_falling && !falling) 
                    {
                        float dist = distance(pos, ac->pos);
                        ac->check_speedhack_dist(dist, real_lag);
                    }

                }

                if(!falling) ac->no_falling();

                ac->was_falling = falling;
     
            }

            ac->pos = pos;
            
            if(ca->state.state == CS_LAGGED) ca->state.state = CS_ALIVE;

            break;
        }
        
        case N_SPAWN:
        {
            int ls = getint(p), gunselect = getint(p);
            
            if(gunselect<GUN_FIST || gunselect>GUN_PISTOL) 
            {
                ac->unknown_weapon(gunselect);
                break;
            }
            if((ca->state.state!=CS_ALIVE && ca->state.state!=CS_DEAD) || ls!=ca->state.lifesequence || ca->state.lastspawn<0) break;
            
            ac->spawn();
            
            break;
        } 

        case N_GUNSELECT:
        {
            int gunselect = getint(p);
            if(ca->state.state != CS_ALIVE) break;
            if(gunselect < GUN_FIST || gunselect > GUN_PISTOL) 
            {
                ac->unknown_weapon(gunselect);
                break;
            }
            ac->check_gun(gunselect);
            break;
        }

        case N_ITEMLIST:
        {
            int n;
            if(!notgotitems && gamemode != 1)
            {
                while((n = getint(p))>=0 && n<MAXENTS && !p.overread())
                {
                    int item_type = getint(p);
                    if(sents[n].type != item_type)
                    {
                        ac->modified_map_items();
                        break;
                    }
                }  
                break;
            }        
            break;
        }
        
        case N_INITFLAGS:
        {
            extern void ac_parseflags(ucharbuf &p, bool commit, clientinfo *ci);
            if(smode == &ctfmode) ac_parseflags(p, (ca->state.state!=CS_SPECTATOR || ca->privilege) && !strcmp(ca->clientmap, smapname), ca);
            break;
        }
        
        case N_BASES:
        {
            extern void ac_parsebases(ucharbuf &p, bool commit, clientinfo *ci);
            if(smode == &capturemode) ac_parsebases(p, (ca->state.state!=CS_SPECTATOR || ca->privilege) && !strcmp(ca->clientmap, smapname), ca);
            break;
        }
            
        case N_TAKEFLAG:
        {
            if(smode != &ctfmode && ac->lastspawn > -1 && totalmillis - ac->lastspawn >= 2000)
            {
                ac->impossible(3, -1);
                return;
            }
            
            ac->is_player_invisible();
            int i = getint(p), version = getint(p);
            if(ctfmode.notgotflags || !ctfmode.flags.inrange(i) || ca->state.state!=CS_ALIVE || !ca->team[0]) return;

            ctfservmode::flag &f = ctfmode.flags[i];
            
            if(f.version != version) break;
            
            vec flag_location = vec(0, 0, 0);
            
            bool flag_dropped = !std::isnan(f.droploc.x) && (f.droploc != vec(0, 0, 0) && f.droploc != f.spawnloc);

            if(m_ctf) flag_location = flag_dropped ? f.droploc : f.spawnloc;
            if((m_hold || m_protect) && ctfmode.holdspawns.inrange(f.spawnindex)) 
                flag_location = flag_dropped ? f.droploc : ctfmode.holdspawns[f.spawnindex].o; 
                
            int flag_owner = ctfmode.flags[i].owner;

            bool has_flag = false;
            
            loopv(ctfmode.flags) 
            {
                if(ctfmode.flags[i].owner == ca->clientnum)
                {
                    has_flag = true;
                    break;
                }
            }
            
            const char *flag_team = ctfflagteam(f.team);
            bool is_team_flag = m_hold || !strcmp(ca->team, flag_team);
           
            bool score_flag = is_team_flag && !m_hold;
            
            bool enemy_flag_dropped = false;
            
            if(!m_hold || !m_protect) // this matters only in "ctf" (to make sure score limit below isnt checked while returning enemy flag)
            {
                ctfservmode::flag &e = ctfmode.flags[!strcmp(ca->team, "good") ? ctfteamflag("evil") : ctfteamflag("good")];
                enemy_flag_dropped = !std::isnan(e.droploc.x) && (e.droploc != vec(0, 0, 0) && e.droploc != e.spawnloc); // enemy flag is dropped
            }
            
            if(flag_dropped) break;
            
            float flag_dist = distance(flag_location, ac->pos); 
            if(flag_dist >= 250 || flag_owner != -1)
            {
                if((m_protect && (f.invistime && !is_team_flag)) || (!has_flag && score_flag && !m_protect)) break;
                ac->get_flag(flag_dist, m_protect ? !score_flag : score_flag, flag_owner != -1);
            }   
            
            if(enemy_flag_dropped) break;
            
            if(score_flag && !m_protect && !m_hold)
            {
                if(i > 2 || i < 1) break;
                int score = ctfmode.scores[i-1] + 1;
                if(score > ctfmode.FLAGLIMIT)
                {
                    cheat(ci->clientnum, 1, score);
                }
            }
            
            break;
        }
        
        case N_PING:
            if(gamemillis - ci->maploaded > 5000) ac->ping();
            break;
        
        case N_CLIENTPING:
            ac->_ping = getint(p);
            break;

        case N_EDITENT:
        case N_EDITVAR:
        case N_EDITMODE:
        case N_COPY:
        case N_PASTE:
        case N_CLIPBOARD:
        {
            if(!m_edit) ac->edit_packet(type);
            break;
        }
        
        default:;
    }
}

void ac_parseflags(ucharbuf &p, bool commit, clientinfo *ci)
{
    int numflags = getint(p);
    
    if(!ctfmode.notgotflags && commit)
    {
        loopi(numflags)
        {
            int team = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(m_hold) 
            {
                if(!ctfmode.holdspawns.inrange(i) || !vec_equal(ctfmode.holdspawns[i].o, o))
                {
                    ci->ac.modified_map_flags();
                    return;
                }
            }
            else 
            {
                if(!ctfmode.flags.inrange(i) || ctfmode.flags[i].team != team || !vec_equal(ctfmode.flags[i].spawnloc, o))
                {
                    ci->ac.modified_map_flags();
                    return;
                }
            } 
        }         
        return;
    }
}

void ac_parsebases(ucharbuf &p, bool commit, clientinfo *ci)
{
    int numbases = getint(p);
      
    if(!capturemode.notgotbases)
    {
        loopi(numbases)
        {
            int ammotype = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(!capturemode.bases.inrange(i) || capturemode.bases[i].ammotype != ammotype || !vec_equal(capturemode.bases[i].o, o))
                ci->ac.modified_capture_bases();
        }
        return;
    }
}

void anti_cheat_serverupdate()
{
    if(smode == &ctfmode && totalmillis % 10 == 0)
    {
        loopv(ctfmode.flags) 
        {
            ctfservmode::flag &f = ctfmode.flags[i];
            clientinfo *ci = getinfo(f.owner);
            if(!ci) continue;
            if(ci->ac.invisiblehack_count >= 4) ctfmode.dropflag(ci, ci);
        }
    }
}

#endif

