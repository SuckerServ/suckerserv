namespace message{
    
    int disc_msgs;
    int disc_window;
    
    namespace resend_time{
        
        unsigned text = 0;
        unsigned sayteam = 0;
        unsigned mapvote = 0;
        unsigned switchname = 0;
        unsigned switchteam = 0;
        unsigned kick = 0;
        unsigned remip = 0;
        unsigned newmap = 0;
        unsigned spec = 0;

    } //namespace resend_time
    
    bool limit(clientinfo * ci, unsigned long long * millis, unsigned long long resend_time, const char * message_type = NULL)
    {
        if(ci->privilege == PRIV_ADMIN) return false;

        unsigned long long curmillis = getmilliseconds(); // don't use signed-int and overflow prone totalmillis
        unsigned long long wait = curmillis - *millis;

        if(wait >= resend_time){
            *millis = curmillis;
        }
        else{
            defformatstring(error_message, RED "Rejected %s message (wait %i seconds before resending)!", message_type, static_cast<int>(std::ceil((resend_time - wait)/1000.0)));
            ci->sendprivtext(error_message);
        }

        return wait < resend_time;
    }
    
    string set_player_privilege = "The server has %s your privilege to %s%s.";
    
} //namespace message

string ext_admin_pass = "";

struct disconnect_info
{
    int cn;
    int code;
    int session_id;
    std::string reason;
};

static int execute_disconnect(void * info_vptr)
{
    disconnect_info * info = reinterpret_cast<disconnect_info *>(info_vptr);
    clientinfo * ci = getinfo(info->cn);
    if(!ci || ci->sessionid != info->session_id)
    {
        delete info;
        return 0;
    }
    ci->disconnect_reason = info->reason;
    disconnect_client(info->cn, info->code);
    delete info;
    return 0;
}

void disconnect(int cn, int code, std::string reason)
{
    clientinfo * ci = get_ci(cn);

    disconnect_info * info = new disconnect_info;
    info->cn = cn;
    info->session_id = ci->sessionid;
    info->code = code;
    info->reason = reason;
    
    sched_callback(&execute_disconnect, info);
}

void changetime(int remaining)
{
    gamelimit = gamemillis + remaining;
    if(remaining > 0) sendf(-1, 1, "ri2", N_TIMEUP, remaining / 1000);
    next_timeupdate = gamemillis + (remaining % (60*1000));
    if(gamemillis < next_timeupdate)
    {
        event_timeupdate(event_listeners(), boost::make_tuple(get_minutes_left(), get_seconds_left()));
    }
}

int get_minutes_left()
{
    return (gamemillis>=gamelimit ? 0 : (gamelimit - gamemillis + 60000 - 1)/60000);
}

void set_minutes_left(int mins)
{
    changetime(mins * 1000 * 60);
}

int get_seconds_left()
{
    return (gamemillis>=gamelimit ? 0 : (gamelimit - gamemillis) / 1000);
}

void set_seconds_left(int seconds)
{
    changetime(seconds * 1000);
}

void player_msg(int cn,const char * text)
{
    convert2cube textcubeenc(text);
    get_ci(cn)->sendprivtext(textcubeenc.str());
}

void server_msg(const char * text)
{
    convert2cube textcubeenc(text);
    sendservmsg(textcubeenc.str());
}

int player_id(lua_State * L)
{
    int cn = luaL_checkint(L, 1);
    clientinfo * ci = get_ci(cn);
        
    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);
    
    if(ci->state.aitype == AI_NONE)
    {
        uint ip_long = getclientip(get_ci(cn)->clientnum);
        luaL_addlstring(&buffer, reinterpret_cast<const char *>(&ip_long), sizeof(ip_long));
        convert2utf8 utf8name(ci->name);
        luaL_addlstring(&buffer, utf8name.str(), utf8name.length());
    }
    else
    {
        luaL_addstring(&buffer, "bot");
        luaL_addlstring(&buffer, reinterpret_cast<const char *>(&ci->sessionid), sizeof(ci->sessionid));   
    }
    
    luaL_pushresult(&buffer);
    return 1;
}

int player_sessionid(int cn)
{
    clientinfo * ci = getinfo(cn);
    return (ci ? ci->sessionid : -1);
}

int player_ownernum(int cn)
{
    return get_ci(cn)->ownernum;
}

std::string player_name(int cn) {
    convert2utf8 utf8name(get_ci(cn)->name);
    return utf8name.stdstr();
}

void player_rename(int cn, const char * newname, bool public_rename)
{
    char safenewname[MAXNAMELEN + 1];
    convert2cube newnamecubeenc(newname);
    filtertext(safenewname, newnamecubeenc.str(), false, MAXNAMELEN);
    if(!safenewname[0]) copystring(safenewname, "unnamed", MAXNAMELEN + 1);
    
    clientinfo * ci = get_ci(cn);
    
    if (!ci || cn >= 128) return;
    
    putuint(ci->messages, N_SWITCHNAME);
    sendstring(safenewname, ci->messages);
    
    vector<uchar> switchname_message;
    putuint(switchname_message, N_SWITCHNAME);
    sendstring(safenewname, switchname_message);
    
    packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
    putuint(p, N_CLIENT);
    putint(p, ci->clientnum);
    putint(p, switchname_message.length());
    p.put(switchname_message.getbuf(), switchname_message.length());
    sendpacket(ci->clientnum, 1, p.finalize(), (public_rename ? -1 : ci->clientnum));
    
    char oldname[MAXNAMELEN+1];
    copystring(oldname, ci->name, MAXNAMELEN+1);
    
    copystring(ci->name, safenewname, MAXNAMELEN+1);
    
    if(public_rename)
    {
        event_renaming(event_listeners(), boost::make_tuple(ci->clientnum, 0));
        convert2utf8 oldnameutf8(oldname);
        convert2utf8 nameutf8(ci->name);
        event_rename(event_listeners(), boost::make_tuple(ci->clientnum, oldnameutf8.str(), nameutf8.str()));
    }
}

std::string player_displayname(int cn)
{
    clientinfo * ci = get_ci(cn);
    
    std::string output;
    output.reserve(MAXNAMELEN + 5);
    
    convert2utf8 utf8name(ci->name);
    output = utf8name.str();
    
    bool is_bot = ci->state.aitype != AI_NONE;
    bool duplicate = false;
    
    if(!is_bot)
    {
        loopv(clients)
        {
            if(clients[i]->clientnum == cn) continue;
            if(!strcmp(clients[i]->name, ci->name))
            {
                duplicate = true;
                break;
            }
        }
    }
    
    if(is_bot || duplicate)
    {
        char open = (is_bot ? '[' : '(');
        char close = (is_bot ? ']' : ')');
        
        output += "\fs" MAGENTA " ";
        output += open;
        output += boost::lexical_cast<std::string>(cn);
        output += close;
        output += "\fr";
    }
    
    return output;
}

std::string player_team(int cn)
{
    if(!m_teammode) return "";
    convert2utf8 utf8name(get_ci(cn)->team);
    return utf8name.stdstr();
}

const char * player_ip(int cn)
{
    return get_ci(cn)->hostname();
}

unsigned long player_iplong(int cn)
{
    return ntohl(getclientip(get_ci(cn)->clientnum));
}

int player_status_code(int cn)
{
    return get_ci(cn)->state.state;
}

int player_ping(int cn)
{
    return get_ci(cn)->ping;
}

int player_ping_update(int cn)
{
    return get_ci(cn)->lastpingupdate;
}

int player_lag(int cn)
{
    return get_ci(cn)->lag;
}

int player_real_lag(int cn)
{
    return totalmillis - get_ci(cn)->lastposupdate;
}

int player_maploaded(int cn)
{
    return get_ci(cn)->maploaded;
}

int player_deathmillis(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->state.lastdeath;
}

int player_frags(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->state.frags + ci->state.suicides + ci->state.teamkills;
}

int player_score(int cn)
{
    return get_ci(cn)->state.frags;
}

int player_deaths(int cn)
{
    return get_ci(cn)->state.deaths;
}

int player_suicides(int cn)
{
    return get_ci(cn)->state.suicides;
}

int player_teamkills(int cn)
{
    return get_ci(cn)->state.teamkills;
}

int player_damage(int cn)
{
    return get_ci(cn)->state.damage;
}

int player_damagewasted(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->state.explosivedamage + ci->state.shotdamage - ci->state.damage;
}

int player_maxhealth(int cn)
{
    return get_ci(cn)->state.maxhealth;
}

int player_health(int cn)
{
    return get_ci(cn)->state.health;
}

int player_armour(int cn)
{
    return get_ci(cn)->state.armour;
}

int player_armour_type(int cn)
{
    return get_ci(cn)->state.armourtype;  
}

int player_gun(int cn)
{
    return get_ci(cn)->state.gunselect;
}

int player_hits(int cn)
{
    return get_ci(cn)->state.hits;
}

int player_misses(int cn)
{
    return get_ci(cn)->state.misses;
}

int player_shots(int cn)
{
    return get_ci(cn)->state.shots;
}

int player_accuracy(int cn)
{
    clientinfo * ci = get_ci(cn);
    int shots = ci->state.shots;
    int hits = shots - ci->state.misses;
    return static_cast<int>(roundf(static_cast<float>(hits)/std::max(shots,1)*100));
}

int player_accuracy2(int cn)
{
    clientinfo * ci = get_ci(cn);
    return static_cast<int>(roundf(static_cast<float>(ci->state.damage*100/max(ci->state.shotdamage,1))));
}

bool player_is_spy(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->spy;
}

int player_clientmillis(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->clientmillis;
}

int player_timetrial(int cn)
{ 
    clientinfo * ci = get_ci(cn);
    return ci->timetrial >= 0 ? ci->timetrial : -1;
}

int player_privilege_code(int cn)
{
    return get_ci(cn)->privilege;
}

const char * player_privilege(int cn)
{
    return privname(get_ci(cn)->privilege);
}

const char * player_status(int cn)
{
    switch(get_ci(cn)->state.state)
    {
        case CS_ALIVE: return "alive";
        case CS_DEAD: return "dead"; 
        case CS_SPAWNING: return "spawning"; 
        case CS_LAGGED: return "lagged"; 
        case CS_SPECTATOR: return "spectator";
        case CS_EDITING: return "editing"; 
        default: return "unknown";
    }
}

int player_connection_time(int cn)
{
    return (totalmillis - get_ci(cn)->connectmillis)/1000;
}

int player_timeplayed(int cn)
{
    clientinfo * ci = get_ci(cn);
    return (ci->state.timeplayed + (ci->state.state != CS_SPECTATOR ? (lastmillis - ci->state.lasttimeplayed) : 0))/1000;
}

int player_win(int cn)
{
    clientinfo * ci = get_ci(cn);
    
    if(!m_teammode)
    {
        loopv(clients)
        {
            if(clients[i] == ci || clients[i]->state.state == CS_SPECTATOR) continue;
            
            bool more_frags = clients[i]->state.frags > ci->state.frags;
            bool eq_frags = clients[i]->state.frags == ci->state.frags;
            
            bool less_deaths = clients[i]->state.deaths < ci->state.deaths;
            bool eq_deaths = clients[i]->state.deaths == ci->state.deaths;
            
            int p1_acc = player_accuracy(clients[i]->clientnum);
            int p2_acc = player_accuracy(cn);
            
            bool better_acc = p1_acc > p2_acc;
            bool eq_acc = p1_acc == p2_acc;
            
            bool lower_ping = clients[i]->ping < ci->ping;
            
            if( more_frags || (eq_frags && less_deaths) ||
                (eq_frags && eq_deaths && better_acc) || 
                (eq_frags && eq_deaths && eq_acc && lower_ping)) return false;            
        }
        return true;
    }
    else return team_win(ci->team);
}

void player_slay(int cn)
{
    clientinfo * ci = get_ci(cn);
    if(ci->state.state != CS_ALIVE) return;
    ci->state.state = CS_DEAD;
    sendf(-1, 1, "ri2", N_FORCEDEATH, cn);
}

bool player_changeteam(int cn,const char * newteam, bool dosuicide)
{
    clientinfo * ci = get_ci(cn);
    convert2cube newteamcubeenc(newteam);
    convert2utf8 oldteamutf8(ci->team);
    
    if(!m_teammode || (smode && !smode->canchangeteam(ci, ci->team, newteamcubeenc.str())) ||
        event_chteamrequest(event_listeners(), boost::make_tuple(cn, oldteamutf8.str(), newteam, -1)) || !addteaminfo(newteamcubeenc.str()))
    {
        return false;
    }
    
    if(dosuicide && (smode || ci->state.state==CS_ALIVE)) suicide(ci);
    event_reteam(event_listeners(), boost::make_tuple(ci->clientnum, oldteamutf8.str(), newteam));
    
    copystring(ci->team, newteamcubeenc.str(), MAXTEAMLEN+1);
    sendf(-1, 1, "riis", N_SETTEAM, cn, newteamcubeenc.str());
    
    if(ci->state.aitype == AI_NONE) aiman::dorefresh = true;
    
    return true;
}

int player_rank(int cn){return get_ci(cn)->rank;}
bool player_isbot(int cn){return get_ci(cn)->state.aitype != AI_NONE;}

bool player_has_joined_game(int cn)
{
    return get_ci(cn)->connected;
}

void player_join_game(int cn)
{
    clientinfo * ci = get_ci(cn);
    if(!ci->connected)
    {
        connected(ci);
    }
}

void player_reject_join_game(int cn)
{
    clientinfo * ci = get_ci(cn);
    if(!ci->connected)
    {
        disconnect_client(cn, ci->connectauth);   
    }
}

void changemap(const char * map,const char * mode = "",int mins = -1)
{
    int gmode = (mode[0] ? modecode(mode) : gamemode);
    if(!m_mp(gmode)) gmode = gamemode;
    convert2cube mapcubeenc(map);
    changemap(mapcubeenc.str(),gmode,mins);
}

int getplayercount()
{
    return numclients(-1, false, true);
}

int getbotcount()
{
    return numclients(-1, true, false) - numclients();
}

int getspeccount()
{
    return getplayercount() - numclients();
}

void team_msg(const char * team,const char * msg)
{
    if(!m_teammode) return;
    convert2cube teamcubeenc(team);
    convert2cube msgcubeenc(msg);
    loopv(clients)
    {
        clientinfo *t = clients[i];
        if(t->state.state==CS_SPECTATOR || t->state.aitype != AI_NONE || strcmp(t->team, teamcubeenc.str())) continue;
        t->sendprivtext(msgcubeenc.str());
    }
}

void player_force_spec(int cn)
{
    clientinfo * ci = get_ci(cn);
    ci->allow_self_unspec = false;
    setspectator(ci, true);
}

void player_unforce_spec(int cn)
{
    clientinfo * ci = get_ci(cn);
    ci->allow_self_unspec = true;
    setspectator(ci, false);
}

void player_spec(int cn)
{
    clientinfo * ci = get_ci(cn);
    ci->allow_self_unspec = true;
    setspectator(ci, true);
}

void player_unspec(int cn)
{
    setspectator(get_ci(cn), false);
}

void player_forgive_tk(int cn)
{
    clientinfo * ci = get_ci(cn);
    ci->state.frags++;
    ci->state.teamkills--;
}

void spec_all()
{
    loopv(clients)
    {
        clientinfo * ci = clients[i];
        if(ci->state.aitype != AI_NONE || ci->state.state == CS_SPECTATOR) continue;
        player_spec(ci->clientnum);
    }
}

int player_bots(int cn)
{
    clientinfo * ci = get_ci(cn);
    return ci->bots.length();
}

int player_pos(lua_State * L)
{
    int cn = luaL_checkint(L,1);
    vec pos = get_ci(cn)->state.o;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

bool hasmaster()
{
    bool hasmaster = false;
    loopv(clients) if(clients[i]->privilege >= PRIV_MASTER) hasmaster = true;
    return hasmaster;
}

void cleanup_masterstate()
{
    if(reset_mm && !hasmaster())
    {
        mastermode = display_open ? MM_OPEN : MM_AUTH;
        allowedips.shrink(0);
        if(gamepaused) pausegame(false);
    }
}

void cleanup_masterstate(clientinfo * master)
{
    cleanup_masterstate();
    if(master->state.state==CS_SPECTATOR) aiman::removeai(master);
}

void send_currentmaster(){
    loopv(clients) {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        
        putint(p, N_CURRENTMASTER);
        putint(p, mastermode);
        
        loopv(clients)
            if(clients[j]->privilege > PRIV_NONE) if(
              !clients[j]->hide_privilege || 
              ((clients[i]->privilege > PRIV_NONE && clients[i]->hide_privilege) && clients[j]->hide_privilege)
            ) {
                // if normal master/admin
                // or if both client and target have invis privs
                putint(p, clients[j]->clientnum);
                putint(p, clients[j]->privilege);
            }
        
        putint(p, -1);
        sendpacket(i, 1, p.finalize());
    }
}

void unsetmaster()
{
    loopv(clients)
    {
        clientinfo * master = clients[i];
        if(master->privilege != PRIV_MASTER) continue;
        
        defformatstring(msg, "The server has revoked your master privilege.");
        master->sendprivtext(msg);
        
        master->privilege = PRIV_NONE;
        
        event_privilege(event_listeners(), boost::make_tuple(master->clientnum, static_cast<int>(PRIV_MASTER), static_cast<int>(PRIV_NONE)));
    }
    
    cleanup_masterstate();
    send_currentmaster();
}

void unset_player_privilege(int cn)
{
    clientinfo * ci = get_ci(cn);
    if(ci->privilege == PRIV_NONE) return;
    
    int old_priv = ci->privilege;
    ci->privilege = PRIV_NONE;
    
    cleanup_masterstate(ci);
    send_currentmaster();
    
    event_privilege(event_listeners(), boost::make_tuple(cn, old_priv, static_cast<int>(PRIV_NONE)));
}

void set_player_privilege(int cn, int priv_code, bool public_priv = false)
{
    const char * change;
    clientinfo * player = get_ci(cn);
    
    if(player->privilege == priv_code && player->hide_privilege != public_priv) return;
    
    public_priv = !player->spy && public_priv;
    
    if(priv_code == PRIV_NONE) unset_player_privilege(cn);
        
    int old_priv = player->privilege;
    player->privilege = priv_code;
    
    player->hide_privilege = !public_priv;
    
    if(old_priv == player->privilege)
        change = "\fs\f2changed\fr";
    else
        change = (old_priv < player->privilege ? "\fs\f2raised\fr" : "\fs\f0lowered\fr");
    
    defformatstring(msg, message::set_player_privilege, change, public_priv ? "" : "\fs\f4invisible\fr ", privname(priv_code));
    player->sendprivtext(msg);
    
    send_currentmaster();
    
    event_privilege(event_listeners(), boost::make_tuple(cn, old_priv, player->privilege));
}

bool set_player_master(int cn)
{
    set_player_privilege(cn, PRIV_MASTER, true);
    return true;
}

void set_player_auth(int cn)
{
    set_player_privilege(cn, PRIV_AUTH, true);
}

void set_player_admin(int cn)
{
    set_player_privilege(cn, PRIV_ADMIN, true);
}

void set_player_private_admin(int cn)
{
   set_player_privilege(cn, PRIV_ADMIN, false);
}

void set_player_private_master(int cn)
{
    set_player_privilege(cn, PRIV_MASTER, false);
}

void player_freeze(int cn)
{
    clientinfo * ci = get_ci(cn);
    sendf(ci->clientnum, 1, "rii", N_PAUSEGAME, 1);
}

void player_unfreeze(int cn)
{
    clientinfo * ci = get_ci(cn);
    sendf(ci->clientnum, 1, "rii", N_PAUSEGAME, 0);
}

static void execute_addbot(int skill)
{
    clientinfo * owner = aiman::addai(skill, -1);
    if(!owner) return;
    event_addbot(event_listeners(), boost::make_tuple(-1, skill, owner->clientnum));
    return;
}

void addbot(int skill)
{
    get_main_io_service().post(boost::bind(&execute_addbot, skill));
}

static void execute_deletebot(int cn)
{
    clientinfo * ci = getinfo(cn);
    if(!ci) return;
    aiman::deleteai(ci);
    return;
}

void deletebot(int cn)
{
    if(get_ci(cn)->state.aitype == AI_NONE) 
        luaL_error(get_lua_state(), "not a bot player");
    get_main_io_service().post(boost::bind(&execute_deletebot, cn));
}

void update_mastermask()
{
    bool autoapprove = mastermask & MM_AUTOAPPROVE;
    mastermask &= ~(1<<MM_VETO) & ~(1<<MM_LOCKED) & ~(1<<MM_PRIVATE) & ~MM_AUTOAPPROVE;
    mastermask |= (allow_mm_veto << MM_VETO);
    mastermask |= (allow_mm_locked << MM_LOCKED);
    mastermask |= (allow_mm_private << MM_PRIVATE);
    if(autoapprove) mastermask |= MM_AUTOAPPROVE;
}

const char * gamemodename()
{
    return modename(gamemode,"unknown");
}

namespace cubescript{
std::vector<int> make_client_list(bool (* clienttype)(clientinfo *))
{
    std::vector<int> result;
    result.reserve(clients.length());
    loopv(clients) if(clienttype(clients[i])) result.push_back(clients[i]->clientnum);
    return result;
}
} //namespace cubescript

namespace lua{
int make_client_list(lua_State * L,bool (* clienttype)(clientinfo *))
{
    lua_newtable(L);
    int count = 0;
    
    loopv(clients) if(clienttype(clients[i]))
    {
        lua_pushinteger(L,++count);
        lua_pushinteger(L,clients[i]->clientnum);
        lua_settable(L, -3);
    }
    
    return 1;
}
}//namespace lua

bool is_player(clientinfo * ci){return ci->state.state != CS_SPECTATOR && ci->state.aitype == AI_NONE;}
bool is_spectator(clientinfo * ci){return ci->state.state == CS_SPECTATOR; /* bots can't be spectators*/}
bool is_bot(clientinfo * ci){return ci->state.aitype != AI_NONE;}
bool is_any(clientinfo *){return true;}

std::vector<int> cs_player_list(){return cubescript::make_client_list(&is_player);}
int lua_player_list(lua_State * L){return lua::make_client_list(L, &is_player);}

std::vector<int> cs_spec_list(){return cubescript::make_client_list(&is_spectator);}
int lua_spec_list(lua_State * L){return lua::make_client_list(L, &is_spectator);}

std::vector<int> cs_bot_list(){return cubescript::make_client_list(&is_bot);}
int lua_bot_list(lua_State *L){return lua::make_client_list(L, &is_bot);}

std::vector<int> cs_client_list(){return cubescript::make_client_list(&is_any);}
int lua_client_list(lua_State * L){return lua::make_client_list(L, &is_any);}

std::vector<std::string> get_teams()
{
    std::set<std::string> teams;
    loopv(clients)
    {
        convert2utf8 teamutf8(clients[i]->team);
        teams.insert(teamutf8.stdstr());
    }
    std::vector<std::string> result;
    std::copy(teams.begin(),teams.end(),std::back_inserter(result));
    return result;
}

int lua_team_list(lua_State * L)
{
    lua_newtable(L);
    std::vector<std::string> teams = get_teams();
    int count = 0;
    for(std::vector<std::string>::iterator it = teams.begin(); it != teams.end(); ++it)
    {
        lua_pushinteger(L, ++count);
        lua_pushstring(L, it->c_str());
        lua_settable(L, -3);
    }
    return 1;
}

int get_team_score(const char * team)
{
    int score = 0;
    if(smode) return smode->getteamscore(team);
    else
    {
        teaminfo *t = teaminfos.access(team);
        if(t) score = t->frags;
    }
    return score;
}

std::vector<int> get_team_players(const char * team)
{
    convert2cube teamcubeenc(team);
    std::vector<int> result;
    loopv(clients)
        if(clients[i]->state.state != CS_SPECTATOR && clients[i]->state.aitype == AI_NONE && !strcmp(clients[i]->team, teamcubeenc.str()))
            result.push_back(clients[i]->clientnum);
    return result;
}

int lua_team_players(lua_State * L)
{
    std::vector<int> players = get_team_players(luaL_checkstring(L,1));
    lua_newtable(L);
    int count = 0;
    for(std::vector<int>::iterator it = players.begin(); it != players.end(); ++it)
    {
        lua_pushinteger(L, ++count);
        lua_pushinteger(L, *it);
        lua_settable(L, -3);
    }
    
    return 1;
}

int team_win(const char * team)
{
    std::vector<std::string> teams = get_teams();
    int score = get_team_score(team);
    for(std::vector<std::string>::iterator it = teams.begin(); it != teams.end(); ++it)
    {
        if(*it == team) continue;
        if(get_team_score(it->c_str()) > score) return false;
    }
    return true;
}

int team_draw(const char * team)
{
    std::vector<std::string> teams = get_teams();
    int score = get_team_score(team);
    for(std::vector<std::string>::iterator it = teams.begin(); it != teams.end(); ++it)
    {
        if(*it == team) continue;
        if(get_team_score(it->c_str()) != score) return false;
    }
    return true;
}

int team_size(const char * team)
{
    int count = 0;
    convert2cube teamcubeenc(team);
    loopv(clients) if(strcmp(clients[i]->team, teamcubeenc.str())==0) count++;
    return count;
}

void recorddemo(const char * filename)
{
    if(demorecord) return;
    else setupdemorecord(false, filename);
}

int lua_gamemodeinfo(lua_State * L)
{
    int gamemode_argument = gamemode;
    
    if(lua_gettop(L) > 0 && lua_type(L, 1) == LUA_TSTRING)
    {
        gamemode_argument = modecode(lua_tostring(L, 1));
        if(gamemode_argument == -1) return 0;
    }
    
    lua_newtable(L);
    
    lua_pushboolean(L, m_check(gamemode_argument, M_NOITEMS));
    lua_setfield(L, -2, "noitems");
    
    lua_pushboolean(L, m_check(gamemode_argument,  M_NOAMMO|M_NOITEMS));
    lua_setfield(L, -2, "noammo");
    
    lua_pushboolean(L, m_check(gamemode_argument, M_INSTA));
    lua_setfield(L, -2, "insta");
    
    lua_pushboolean(L, m_check(gamemode_argument, M_TACTICS));
    lua_setfield(L, -2, "tactics");
    
    lua_pushboolean(L, m_check(gamemode_argument, M_EFFICIENCY));
    lua_setfield(L, -2, "efficiency");
    
    lua_pushboolean(L, m_check(gamemode_argument,  M_CAPTURE));
    lua_setfield(L, -2, "capture");
    
    lua_pushboolean(L, m_check(gamemode_argument, M_CAPTURE | M_REGEN));
    lua_setfield(L, -2, "regencapture");
    
    lua_pushboolean(L, m_check(gamemode_argument, M_CTF));
    lua_setfield(L, -2, "ctf");
    
    lua_pushboolean(L, m_checkall(gamemode_argument, M_CTF | M_PROTECT));
    lua_setfield(L, -2, "protect");
    
    lua_pushboolean(L, m_checkall(gamemode_argument, M_CTF | M_HOLD));
    lua_setfield(L, -2, "hold");
    
    lua_pushboolean(L, m_check(gamemode_argument, M_TEAM));
    lua_setfield(L, -2, "teams");
    
    lua_pushboolean(L, m_check(gamemode_argument, M_OVERTIME));
    lua_setfield(L, -2, "overtime");
    
    lua_pushboolean(L, m_checknot(gamemode_argument, M_DEMO|M_EDIT|M_LOCAL));
    lua_setfield(L, -2, "timed");
    
    return 1;
}

void send_auth_challenge(int cn, int id, const char * domain, const char * challenge)
{
    clientinfo * ci = get_ci(cn);
    sendf(ci->clientnum, 1, "risis", N_AUTHCHAL, domain, id, challenge);
}

void send_auth_request(int cn, const char * domain)
{
    clientinfo * ci = get_ci(cn);
    sendf(ci->clientnum, 1, "ris", N_REQAUTH, domain);
}

static bool compare_player_score(const std::pair<int,int> & x, const std::pair<int,int> & y)
{
    return x.first > y.first;
}

static void calc_player_ranks(const char * team)
{
    if(m_edit) return;
    
    if(m_teammode && !team)
    {
        std::vector<std::string> teams = get_teams();
        for(std::vector<std::string>::const_iterator it = teams.begin();
             it != teams.end(); it++) calc_player_ranks(it->c_str());
        return;
    }
    
    std::vector<std::pair<int,int> > players;
    players.reserve(clients.length());
    
    loopv(clients) 
        if(clients[i]->state.state != CS_SPECTATOR && (!team || !strcmp(clients[i]->team,team)))
            players.push_back(std::pair<int,int>(clients[i]->state.frags, i));
    
    std::sort(players.begin(), players.end(), compare_player_score);
    
    int rank = 0;
    for(std::vector<std::pair<int,int> >::const_iterator it = players.begin();
        it != players.end(); ++it)
    {
        rank++;
        if(it != players.begin() && it->first == (it-1)->first) rank--;
        clients[it->second]->rank = rank;
    }
}

void calc_player_ranks()
{
    return calc_player_ranks(NULL);
}

void set_mastermode(int value)
{
    set_mastermode_cn(value, -1);
}

void set_mastermode_cn(int value, int cn)
{
    if(value == mastermode)
    {
        return;
    }
    
    int prev_mastermode = mastermode;
    
    mastermode = value;
    allowedips.shrink(0);
    if(allow_mm_private_reconnect && mastermode >= MM_PRIVATE )
    {
        loopv(clients) allowedips.add(getclientip(clients[i]->clientnum));
    }
    
    event_setmastermode(event_listeners(), boost::make_tuple(cn, mastermodename(prev_mastermode), mastermodename(mastermode)));
    
    sendf(-1, 1, "rii", N_MASTERMODE, mastermode);
}

int get_mastermode()
{
    return mastermode;
}

void add_allowed_ip(const char * hostname)
{
    ENetAddress addr;
    if(enet_address_set_host(&addr, hostname) != 0)
        luaL_error(get_lua_state(), "invalid hostname given");
    allowedips.add(addr.host);
}

void suicide(int cn)
{
    suicide(get_ci(cn));
}

void player_servcmd(int cn, const char *string)
{
    if(cn != -1 && !get_ci(cn)) return;
    sendf(cn, 1, "rdis", N_SERVCMD, string);
}

bool compare_admin_password(const char * x)
{
    return !strcmp(x, adminpass);
}

bool send_item(int type, int recipient) 
{
    int ent_index = sents_type_index[type];
    if(interm > 0 || !sents.inrange(ent_index)) return false;
    clientinfo *ci = getinfo(recipient);
    if(!ci || (!ci->local && !ci->state.canpickup(sents[ent_index].type))) return false;
    sendf(-1, 1, "ri3", N_ITEMACC, ent_index, recipient);
    ci->state.pickup(sents[ent_index].type);
    return true;
}

class player_token
{   
public:
    player_token(clientinfo * ci)
    :m_cn(ci->clientnum), m_session_id(ci->sessionid)
    {
        
    }
    
    clientinfo * get_clientinfo()const
    {
        clientinfo * ci = getinfo(m_cn);
        if(!ci) return NULL;
        if(!ci || ci->sessionid != m_session_id) return NULL;
        return ci;
    }
private:
    int m_cn;
    int m_session_id;       
};

int deferred_respawn_request(void * arg)
{
    player_token * player = reinterpret_cast<player_token *>(arg);
    clientinfo * ci = player->get_clientinfo();
    delete player;
    if(!ci) return 0;
    try_respawn(ci, ci);
    return 0;
}

void try_respawn(clientinfo * ci, clientinfo * cq)
{
    if(!ci || !cq || cq->state.state!=CS_DEAD || cq->state.lastspawn>=0 || (smode && !smode->canspawn(cq))) return;
    if(!ci->clientmap[0] && !ci->mapcrc) 
    {
        ci->mapcrc = -1;
        if(ci == cq) { if(ci->state.state != CS_DEAD) return; }
        else if(cq->ownernum != ci->clientnum) { cq = NULL; return; }
    }
    if(cq->state.deadflush)
    {
        if(event_respawnrequest(event_listeners(), boost::make_tuple(cq->clientnum, cq->state.lastdeath)))
        {
            return;
        }
        
        flushevents(cq, cq->state.deadflush);
        cq->state.respawn();
    }
    cleartimedevents(cq);
    sendspawn(cq);
}

void player_respawn(int cn)
{
    clientinfo * ci = getinfo(cn);
    try_respawn(ci, ci);
}

const char *extfiltertext(const char *src)
{
    static string dst; 
    filtertext(dst, src);
    return dst;
}

void player_nospawn(int cn, int no_spawn)
{
    clientinfo *ci = getinfo(cn);
    if (!ci) return;
    ci->no_spawn = no_spawn;
}

void updateservinfo(int cn, const char* servername)
{
    clientinfo *ci = getinfo(cn);
    if (!ci) return;
    sendf(ci->clientnum, 1, "ri5s", N_SERVINFO, ci->clientnum, PROTOCOL_VERSION, ci->sessionid, 0, servername);
}

void editvar(int cn, const char *var, int value) 
{
    clientinfo *ci = getinfo(cn);
    if (!ci) return;
    sendf(cn, 1, "ri5si", N_CLIENT, cn, 100/*should be safe*/, N_EDITVAR, ID_VAR, var, value);
}

void sendmap(int acn, int tcn)
{
    clientinfo *target = getinfo(tcn);
    if(!target) return;
    if(!mapdata) sendf(acn, 1, "ris", N_SERVMSG, "no map to send");
    else if(target->getmap) sendf(acn, 1, "ris", N_SERVMSG, "already sending map");
    else
    {
        sendservmsgf("[%s is getting the map]", colorname(target));
        if((target->getmap = sendfile(target->clientnum, 2, mapdata, "ri", N_SENDMAP)))
            target->getmap->freeCallback = freegetmap;
        target->needclipboard = totalmillis ? totalmillis : 1;
    }
}

int hitpush(lua_State * L)
{
    int target = luaL_checkint(L, 1);
    int actor = luaL_checkint(L, 2);
    int damage = luaL_checkint(L, 3);
    int gun = luaL_checkint(L, 4);

    const vec push (luaL_checknumber(L, 5), luaL_checknumber(L, 6), luaL_checknumber(L, 7));
    clientinfo *ci = getinfo(target);
    gamestate &ts = ci->state;
    if(target==actor) ci->setpushed();
    else if(!push.iszero())
    {
        ivec v(vec(push).rescale(DNF));
        sendf(ts.health<=0 ? -1 : ci->ownernum, 1, "ri7", N_HITPUSH, target, gun, damage, v.x, v.y, v.z);
        ci->setpushed();
    }
    return 1;
}
