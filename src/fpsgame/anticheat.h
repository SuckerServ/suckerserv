/*
 * Cheat Detection / Anticheat System (c) 2011 by Thomas
 * 
 * TODO: Check Jumppads / Teleporter IDs
 *
 */

#ifdef anticheat

bool anti_cheat_enabled = true;
bool anti_cheat_add_log_to_demo = true;
	
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
    
bool is_invisible(clientinfo *ci)
{
    if (!anti_cheat_enabled || !ci || ci->state.state != CS_ALIVE) 
        return false;
     
    int lag = totalmillis - ci->lastposupdate;
    if (lag == totalmillis) lag = -1;
    if (ci->state.o == vec(0, 0, 0) || lag >= 7500 || lag == -1)
    {
        cheat(ci->clientnum, 9, lag);
        return true;
    }
    return false;
}

float distance(vec a, vec b)
{
    int x = a.x - b.x, y = a.y - b.y, z = a.z - b.z;
    return sqrt(x*x + y*y + z*z);
}
    
bool is_invisible(int cn) 
{
    return is_invisible(getinfo(cn));
}
    
bool vec_equal(vec a, vec b)
{
    loopi(3) if ((int)a[i] != (int)b[i]) return false; 
    return true;
}

#else
 
extern bool is_invisible(int);
extern void cheat(int cn, int cheat, int info1, const char *info2="");
extern vector<server_entity> sents;

class anticheat
{
    public:

    int clientnum;
    int pingupdates, lastpingupdates, lastpingsnapshot, speedhack, speedhack_updates;
    int speedhack2, speedhack2_dist, speedhack2_lastreset;
    int timetrial;
    int lastshoot, lastjumppad, lastteleport, lastdamage, lastspawn;
    int lastjump;
    int jumphack, jumppads, jumphack_dist;
    int _ping;
    float last_fall_loc;
    float speedhack2_lastdist, jumpdist, lastjumphack_dist;
    bool was_falling;

    void reset(int cn=-1)
    {
        if (cn > -1) clientnum = cn;
        
        jumpdist = 0;
        last_fall_loc = 0;
        
        reset_speedhack_dist();
        reset_speedhack_ping();
        reset_last_action();
        reset_jumphack();
    }
    
    /*
     * Player Actions
     */

    void teleport()
    {
        if (speedhack2) 
        {
            speedhack2--;
            speedhack2_dist -= (int)speedhack2_lastdist;
        }
        lastteleport = totalmillis;    
    }
    
    void no_falling() { jumpdist = 0; try_decrease_jumphack(); }
    void shoot() { lastshoot = totalmillis; }
    void damage() { lastdamage = totalmillis; }
    void jumppad() { lastjumppad = totalmillis; jumppads++; }
    void spawn() { lastspawn = totalmillis; reset_speedhack_dist(); reset_last_action(); reset_jumphack(); fix_items(); }
    void ping() { check_ping(); }
    
    /*
     * Invisible 
     */
     
    bool is_player_invisible()
    {
        if (lastspawn == -1 || totalmillis - lastspawn < 1000) return false;
        return is_invisible(clientnum);
    }
    
    /*
     * Speed Hack Position
     */

    void check_speedhack_dist(float dist)
    {
        if (dist >= 7)
        {
            speedhack2++;
            speedhack2_dist += (int)dist;
            speedhack2_lastdist = dist;
            
            int speed = is_speedhack_dist();
            if (speed && speed / 1000 < 4 && _ping > 2000) 
            {
                reset_speedhack_dist();
                speed = 0;
            }
            else
            {
                check_speedhack_pos_reset();
            }
            
            if (speed) cheat(clientnum, 16, speed);
        }
    }
    
    /*
     * Jumphack
     */
    
    void check_jumphack(float dist, float fall_loc)
    {      
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
            if (jumpdist > 25) 
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
        defformatstring(info)("TRIED ITEM: %i ITEMLIST LENGTH: %i", item, len);
        cheat(clientnum, 19, 0, info);
    }
    
    /*
     * Item Not Spawned
     */    
    
    void item_not_spawned(int item, int spawntime)
    {
        defformatstring(info)("TRIED ITEM: %i SPAWNTIME: %i", item, spawntime);
        cheat(clientnum, 20, 0, info);
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
    
    /*
     * Speed Hack Position
     */
    
    void check_speedhack_pos_reset()
    {
        if (totalmillis - speedhack2_lastreset >= 30000) reset_speedhack_dist();
    }
    
    int is_speedhack_dist()
    {
        if (speedhack2 < 30) return 0;
        
        float speed = (float)speedhack2_dist / (float)speedhack2 / (float)4;
        
        if (speed >= 1.5) 
        {
            reset_speedhack_dist();
            return (speed * (float)1000);
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
                    int speed2 = (speed * (float)1000);
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
        if (jumphack < 5) return 0;
        int avg_dist = jumphack_dist / (int)jumphack;
        return avg_dist > 25 ? avg_dist : 0;
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
     * FFA Item Bugfix
     */
    
    void fix_items()
    {
        if (clientnum < 0 || clientnum >= 128) return;
        loopv(sents)
        { 
            if (!sents[i].spawned) 
            { 
                sendf(clientnum, 1, "ri3", N_ITEMACC, i, -1); 
            } 
        } 
    }
};
    
#endif