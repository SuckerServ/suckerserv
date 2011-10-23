/*
 * Cheat Detection / Anticheat System (c) 2011 by Thomas
 * 
 * TODO: Check Jumppads / Teleporter IDs
 *
 */

#ifdef anticheat

bool anti_cheat_enabled = true;
bool anti_cheat_add_log_to_demo = true;
int anti_cheat_system_rev = 1;
	
void cheat(int cn, int cheat, int info1, const char *info2)
{
	if (!anti_cheat_enabled) return;
    
    if (anti_cheat_add_log_to_demo && demorecord)
    {
        clientinfo *ci = getinfo(cn);
        if (ci)
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
             * if (wn == 2048 && reason == -1)
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
    if (!anti_cheat_enabled || !ci || ci->state.state != CS_ALIVE || ci->ping >= 7500) 
        return false;
     
    int lag = totalmillis - ci->lastposupdate;
    if (lag == totalmillis) lag = -1;
    
    if (ci->state.o == vec(-1e10f, -1e10f, -1e10f) || ci->state.o == vec(0, 0, 0))
        return 2; // weird position hack
    
    if (lag >= 7500 || lag == -1)
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
    loopi(3) if ((int)a[i] != (int)b[i]) return false; 
    return true;
}

#else
 
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
    float exceed_position;
    int _ping;
    float last_fall_loc;
    float speedhack2_lastdist, jumpdist, lastjumphack_dist;
    bool was_falling;
    bool initialized;

    void reset(int cn=-1)
    {
        if (!anti_cheat_enabled) 
        {
            initialized = false;
            return;
        }
        
        if (cn > -1) clientnum = cn;

        jumpdist = 0;
        last_fall_loc = 0;
        spawns = 0;
        lastgun = -1;
        exceed_position = 0;

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
    void teleport(int n) { if (speedhack2_strangedist_c) { speedhack2_strangedist_c--; speedhack2_strangedist -= speedhack2_lastdist; } lastteleport = totalmillis; }
    void spawn() { reset_invisible(); reset_speedhack_dist(); reset_last_action(); reset_jumphack(); fix_items(); lastspawn = totalmillis; spawns++; }
    void ping() { check_ping(); }
    
    /*
     * Invisible 
     */
     
    bool is_player_invisible()
    {
        if (!initialized || (lastspawn > -1 && totalmillis - lastspawn < 2000)) return false;
        int is_invis = is_invisible(clientnum);
        
        switch (is_invis)
        {
            case 1:
                invisiblehack_count++;
                if (invisible_first == -1) invisible_first = totalmillis;
                invisible_last = totalmillis;
                if (invisiblehack_count >= 7) 
                {
                    int inv_time = invisible_last - invisible_first;
                    if (inv_time >= 2000) cheat(clientnum, 9, inv_time); 
                    reset_invisible();
                }
                break;
            case 2:
                positionhack++;
                if (positionhack >= 4) cheat(clientnum, 24, -1);   
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
        if (!initialized) return;
        if (!lagged && dist >= 5.5) // DON'T CHANGE THIS VALUE!!!
        {          
            if (dist < 50) // < 12.5x
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
            /*if (speed  && speed / 100000 < 4 && _ping > 2000) 
            {
                reset_speedhack_dist();
                speed = 0;
            }
            else*/
            if (totalmillis - speedhack2_lastreset >= 45000)
            {
                reset_speedhack_dist();
            }
            
            if (speed) cheat(clientnum, 16, speed);
        }
    }
    
    /*
     * Jumphack
     */
    
    void check_jumphack(float dist, float fall_loc)
    {     
        if (!initialized) return;
        int last_action = last_player_action();
        
        if ((last_action < 1500 && last_action != -1) || (lastjump > -1 && totalmillis - lastjump < 500)) 
        {
            jumpdist = 0;
            return;
        }

        if (dist > 0)
        {
            jumpdist = !was_falling ? (int)dist : jumpdist + (int)dist;
        }
        else  // falling down
        {
            if (jumpdist > 20) 
            {
                jumphack++;
                lastjumphack_dist = jumpdist;
                jumphack_dist += jumpdist;
                
                //defformatstring(debug)("jumphack %i %2.f (%d) %ix", clientnum, jumpdist, last_action, jumphack);
                //sendservmsg(debug);
                    
                jumpdist = 0;
                try_decrease_jumphack(); // big jumppads 
                
                int jdist = is_jumphack();
                if (jdist > 0)
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
     
    void check_get_flag(float dist)
    {
        if (dist >= 250)
        {
            //cheat(clientnum, 10, (int)dist); //FIXME
        }
    }
    
    /*
     * Unknown Item
     */    
    
    void unknown_item(int item, int len)
    {
        if (initialized && lastspawn > -1 && totalmillis - lastspawn >= 2000)
        {
            //if (!is_item_mode() && impossible(1, item)) return;
            defformatstring(info)("TRIED ITEM: %i ITEMLIST LENGTH: %i", item, len);
            cheat(clientnum, 19, 0, info);
        }
    }
    
    /*
     * Item Not Spawned
     */    
    
    void item_not_spawned(int item, int spawntime)
    {
        if (initialized && lastspawn > -1 && totalmillis - lastspawn >= 2000)
        {
            //if (!is_item_mode() && impossible(0, item)) return; // There are item pick ups in insta. You just don't see them.
            defformatstring(info)("TRIED ITEM: %i SPAWNTIME: %i", item, spawntime);
            cheat(clientnum, 20, 0, info);
        }
    }
    
    /*
     * FFA Weapons in INSTA modes
     */
     
    bool check_gun(int gunselect, bool is_spawn=false)
    {
        if (!initialized) return true;
        //if (mod_gamemode) return true;
        lastgun = gunselect;
        if (is_item_mode() || m_efficiency) return true;
        bool correct_gun = gunselect == GUN_RIFLE || gunselect == GUN_FIST;
        if (!correct_gun)
        {
            if (is_spawn)
            { 
                if (spawns < 3) correct_gun = true;
            }
            else
            { 
                if (lastspawn > -1 && totalmillis - lastspawn < 2000) correct_gun = true;
            }
        }
        if (!correct_gun) impossible(2, gunselect);
        return correct_gun;
    }
    
    /*
     * Player position exceeded
     */
    
    void player_position_exceeded()
    {
        //if (mod_gamemode) return;
        cheat(clientnum, 23, (int)exceed_position);
    }
    
    /*
     * Impossible Actions
     */
   
    bool impossible(int c, int info)
    {
        //if (mod_gamemode) return false;
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
        if (lag >= 500) 
        {
            lagged = true;
            last_lag = totalmillis;
            return;
        }
        
        if (last_lag > 0 && totalmillis >= (last_lag + 10000)) 
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
        jumphack = 0;
        lastjumphack_dist = jumphack_dist = 0;
    }

    void reset_speedhack_dist()
    {
        speedhack2 = speedhack2_dist = jumpdist = 0;
        speedhack2_strangedist_c = speedhack2_strangedist = 0;
        speedhack2_lastdist = 0;
        was_falling = false;
        speedhack2_lastreset = totalmillis;
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
        if (lastjumppad > -1 && lastjumppad > shortest) shortest = lastjumppad;
        if (lastteleport > -1 && lastteleport > shortest) shortest = lastteleport;
        if (lastdamage > -1 && lastdamage > shortest) shortest = lastdamage;
        
        return shortest > -1 ? totalmillis - shortest : shortest;
    }
    
    int is_speedhack_dist()
    {
        if (speedhack2 >= 30) 
        {
            float speed = speedhack2_dist / (float)speedhack2 / (float)4;
            if (speed < 1.2) return 0;
            else reset_speedhack_dist();
            return (speed * (float)100000);
        }
        
        if (speedhack2_strangedist_c >= 10)
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
        if (pingupdates % 20 != 0) return; 
        if (lastpingsnapshot) 
        {
            int snapsec = (totalmillis - lastpingsnapshot) / 1000;
            if (snapsec)
            {
                int pingupdates = 20 / snapsec;
                if (pingupdates >= 5) // 2x from real time
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
            if (speedhack >= 20) // trapped 20 times into the detection
            {
                float speed = ((float)speedhack_updates / (float)speedhack) / (float)4;
                if (speed >= 1.5)
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
        if (jumphack < 2) return 0;
        int avg_dist = jumphack_dist / (int)jumphack;
        return avg_dist >= 10 ? avg_dist : 0;
    }
    
    void try_decrease_jumphack()
    {
        if (jumppads)
        {
            jumppads--;
            if (jumphack) 
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
        if (clientnum < 0 || clientnum >= 128 || !getinfo(clientnum)) return;
        
        bool item_mode = is_item_mode();
        
        loopv(sents)
        { 
            if (!sents[i].spawned || !item_mode) 
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
