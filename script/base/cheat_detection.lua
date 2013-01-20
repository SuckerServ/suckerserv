--[[
    CHEATER-DETECTION

    Copyright (C) 2011 Thomas

    COMMENTS:
    
        LOGS WITH (TESTING) ARE !!_NOT_!! 100% secure proofs
    
--]]

local ban_time = 21600 -- 6 hrs
local kick_msg = string.format("\f3cheating - (bantime: %i minutes)", round(ban_time / 60, 0))
local min_spawntime = 4950
local min_scoretime = 3000
local max_points = 100
local spawnhack = {}
local speedhack_low = { }
local invisiblehack = { }
local positionhack = { }
local fun_var_type = type
local point_table = { }
local logged = { }
local debug = false
local global_log = false
local kicked = { }
local deathmillis = { }
local first_spawn = { }

cheaters = { }

local network_packet_types = "N_CONNECT, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS, N_SHOOT, N_EXPLODE, N_SUICIDE, N_DIED, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH, N_GUNSELECT, N_TAUNT, N_MAPCHANGE, N_MAPVOTE, N_TEAMINFO, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD, N_PING, N_PONG, N_CLIENTPING, N_TIMEUP, N_FORCEINTERMISSION, N_SERVMSG, N_ITEMLIST, N_RESUME, N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR, N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER, N_SETTEAM, N_BASES, N_BASEINFO, N_BASESCORE, N_REPAMMO, N_BASEREGEN, N_ANNOUNCE, N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO, N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS, N_TAKEFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_TRYDROPFLAG, N_DROPFLAG, N_SCOREFLAG, N_INITFLAGS, N_SAYTEAM, N_CLIENT, N_AUTHTRY, N_AUTHKICK, N_AUTHCHAL, N_AUTHANS, N_REQAUTH, N_PAUSEGAME, N_GAMESPEED, N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT, N_BOTBALANCE, N_MAPCRC, N_CHECKMAPS, N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHTEAM, N_INITTOKENS, N_TAKETOKEN, N_EXPIRETOKENS, N_DROPTOKENS, N_DEPOSITTOKENS, N_STEALTOKENS, N_SERVCMD, N_DEMOPACKET, NUMMSG"
      network_packet_types = strSplit(network_packet_types, ", ")
      
local sound_types = "S_JUMP, S_LAND, S_RIFLE, S_PUNCH1, S_SG, S_CG, S_RLFIRE, S_RLHIT, S_WEAPLOAD, S_ITEMAMMO, S_ITEMHEALTH, S_ITEMARMOUR, S_ITEMPUP, S_ITEMSPAWN, S_TELEPORT, S_NOAMMO, S_PUPOUT, S_PAIN1, S_PAIN2, S_PAIN3, S_PAIN4, S_PAIN5, S_PAIN6, S_DIE1, S_DIE2, S_FLAUNCH, S_FEXPLODE, S_SPLASH1, S_SPLASH2, S_GRUNT1, S_GRUNT2, S_RUMBLE, S_PAINO, S_PAINR, S_DEATHR, S_PAINE, S_DEATHE, S_PAINS, S_DEATHS, S_PAINB, S_DEATHB, S_PAINP, S_PIGGR2, S_PAINH, S_DEATHH, S_PAIND, S_DEATHD, S_PIGR1, S_ICEBALL, S_SLIMEBALL, S_JUMPPAD, S_PISTOL, S_V_BASECAP, S_V_BASELOST, S_V_FIGHT, S_V_BOOST, S_V_BOOST10, S_V_QUAD, S_V_QUAD10, S_V_RESPAWNPOINT, S_FLAGPICKUP, S_FLAGDROP, S_FLAGRETURN, S_FLAGSCORE, S_FLAGRESET, S_BURN, S_CHAINSAW_ATTACK, S_CHAINSAW_IDLE, S_HIT"
      sound_types = strSplit(sound_types, ", ")
local gun_types = "S_PUNCH1, S_SG, S_CG, S_RLFIRE, S_RIFLE, S_FLAUNCH, S_PISTOL, S_FLAUNCH, S_ICEBALL, S_SLIMEBALL, S_PIGR1"
      gun_types = strSplit(gun_types, ", ")
        
local function log_action_global(cheat_str, points, _max_points)
    if not global_log then return end
    
    local function escape(s)
        return string.gsub(s, "([^A-Za-z0-9_])", function(c)
            return string.format("%%%02x", string.byte(c))
        end)
    end
    
    local LOG_URL = string.format("http://EXAMPLE.org/EXAMPLE/cheater.php?cheater=%s&server=%s&points=%d&max_points=%d", escape(cheat_str), escape(server.servername), points, _max_points)
    http.client.get(LOG_URL, function(body) end)
end      

local function is_known_map(map)
    return supported_map(map)
end

local function admin_msg(msg)
    for i, cn in ipairs(server.clients()) do
        if server.player_priv_code(cn) >= 2 then
            server.player_msg(cn, msg)
        end
    end
end

local function network_type(packet)
    packet = packet + 1
    if #network_packet_types >= packet then
        return network_packet_types[packet]
    end
    return string.format("unknown (%i)", packet)
end

local function sound_type(sound)
    sound = sound + 1
    if #sound_types >= sound then
        return sound_types[sound]
    end
    return string.format("unknown (%i)", sound)
end

local function gun_type(gun)
    gun = gun + 1
    if #gun_types >= gun then
        return gun_types[gun]
    end
    return string.format("unknown (%i)", gun)
end

local function kick(cn)
    server.kick(cn, ban_time, "CHEATER-DETECTION", kick_msg)
end

local function speedhack(speed) return _if(speed >= 1.6, 75, 25) end
local function spawn_hack(spawntime) return _if(spawntime <= 3000, 100, 50) end
local function pos_exceeded(pos) return _if(pos >= 200, 25, 10) end
local function getflag(distance, info, cn)
    if info == "stealflag" then return max_points end
    if server.player_lag(cn) <= 40 then
        return _if(distance >= 400, max_points, 34)
    else
        return 10
    end
end
local function gun_distance(distance, gun, cn)
    if gun ~= "S_PUNCH1" and gun ~= "S_RIFLE" then
       return -1
    end
    local lag = server.player_lag(cn)  
    if lag > 40 then return -1 end   
    if gun == "S_PUNCH1" and distance >= 250 and lag <= 40 then
        return 100
    end 
    if gun == "S_RIFLE" and distance >= 2000 and lag <= 40 then
        return 100
    end
end

local type = { }

-- "log long", "log short", "points"
type[1] = { "flag-score-hack (flags: %i)", "flag-limit exceeded", max_points }
type[2] = { "edit-packet-in-non-edit-mode (type: %s)", "fly hack", max_points }
type[3] = { "unknown packet (type: %s)", "unknown packet", 50 }
type[4] = { "unknown-weapon (unknown-gun: %s)", "unknown weapon", 34 }
type[6] = { "speed-hack-ping (avg-speed: %.2fx)", "speedhack ping", 10 }
type[7] = { "spawn-time-hack (spawntime: %i ms)", "spawntime hack", spawn_hack }
type[8] = { "sent-unknown-sound (sound: %s)", "unknown sound", max_points }
type[9] = { "invisible (invis-millis: %i)", "invisible hack", 34 }
type[10] = { "getflag (distance: %i)", "getflag", getflag }
type[11] = { "modified-map-items", "modified map items", max_points }
type[12] = { "modified-map-flags", "modified map flags", max_points }
type[13] = { "modified-capture-bases", "modified capture bases", max_points }
type[14] = { "shoot-out-of-gun-distance-range", "shoot distance", gun_distance }
type[15] = { "scored-in-less-than-" .. round(min_scoretime / 1000, 1) .. "-seconds (%i ms)", "flag score hack", max_points }
type[16] = { "speed-hack-pos (avg-speed: %.2fx)", "speedhack", speedhack }
type[17] = { "jumphack (height: %i)", "jumphack", 0 }
type[18] = { nil, nil }
type[19] = { "unknown-map-item %s", "unknown map item", 50 }
type[20] = { "map-item-not-spawned (item: %i)", "item not spawned", 0 }
type[21] = { nil, nil }
type[22] = { "impossible-player-action", "impossible player action", 25 }
type[23] = { "player-position exceeded (pos: %i)", "player position exceeded", pos_exceeded }
type[24] = { "null-player-position / invisible", "player position / invisible", 25 }

local function check_points(cn, _type, info, info_str, cn)
    if point_table[cn] == nil then point_table[cn] = 0 end
    
    local point_action = type[_type][3]
    
    local cheat_points
    
    if fun_var_type(point_action) == "function" then
        cheat_points = point_action(info, info_str, cn)
    else
        cheat_points = point_action or 0
    end
    
    point_table[cn] = point_table[cn] + cheat_points
    return { _if(point_table[cn] > max_points, max_points, point_table[cn]), cheat_points }
end

server.event_handler("disconnect", function(cn)
    logged[cn] = nil
    point_table[cn] = nil
    kicked[cn] = nil
end)

local function cheat(cn, cheat_type, info, info_str)
    if cheat_type > #type or cheat_type < 1 then return end
    
    if cheat_type >= 11 and cheat_type <= 13 and not is_known_map(server.map) then
        return
    end
    if cheat_type == 19 and not is_known_map(server.map) then
        return
    end

    if type[cheat_type][2] == nil then return end
    
    if server.gamespeed ~= 100 then return end

    local logmsg =
        string.format("CHEATER: %s IP: %s PING: %i LAG: %i GAMEMODE: %s MAP: %s CHEAT: ",
            server.player_displayname(cn),
            server.player_ip(cn),
            server.player_ping(cn),
            server.player_real_lag(cn),
            server.gamemode,
            server.map
        ) .. type[cheat_type][1]
        
    if logged[cn] == nil then logged[cn] = { } end
        
    if logged[cn][cheat_type] == nil or cheat_type ~= 6 then
    
        if (cheat_type == 3 or cheat_type == 2) and info > 0 then info = network_type(info) end
        if cheat_type == 4 then info = gun_type(info) end
        if cheat_type == 8 then info = sound_type(info) end
        if cheat_type == 14 then info_str = string.format(info_str, gun_type(info)) end
        if cheat_type == 6 or cheat_type == 16 or cheat_type == 17 then info = info / 100000 end
        if cheat_type == 10 then type[cheat_type][2] = info_str; info_str = "" end

        local points = check_points(cn, cheat_type, info, type[cheat_type][2], cn)
        
        if info_str ~= "" then info_str = " (INFO: " .. info_str .. ")" end
        
        local points_complete = points[1]
        local points_lastaction = points[2]
        
        if points_lastaction < 0 then return end
        
        local testing = points_lastaction <= 0
        local points_str = string.format(" (%d/%d)", points_complete, max_points)
        local log = string.format(logmsg, info)       
        
        server.log(log .. info_str .. points_str .. _if(testing, " (TESTING)", ""))
        log_action_global(log .. info_str .. _if(testing, " (TESTING)", ""), points_complete, max_points)

        local msg = string.format("\f3(\f8CHEATER-DETECTION\f3)\f8 \f1%s (%s) -> \f1%s\f6%s%s", server.player_displayname(cn), server.player_ip(cn), type[cheat_type][2], _if(testing, " (BETA)", ""), points_str)
        admin_msg(msg)
            
        if logged[cn][cheat_type] == nil then -- log action only once
            cheaters[#cheaters + 1] = msg        
        end
        
        logged[cn][cheat_type] = true
        
        if points_complete >= max_points and not debug and kicked[cn] == nil then
            kicked[cn] = true
            kick(cn)
        end

    end
end

server.event_handler("cheat", cheat)

local function gamemode_has_respawn_wait_time()
    if server.gamemode == "ctf" then return true end
    if server.gamemode == "insta ctf" then return true end
    if server.gamemode == "effic ctf" then return true end
    if server.gamemode == "capture" then return true end
    if server.gamemode == "hold" then return true end
    if server.gamemode == "insta hold" then return true end
    if server.gamemode == "effic hold" then return true end
    return false
end

local is_intermission = false

server.event_handler("intermission", function()
    is_intermission = true
end)
server.event_handler("mapchange", function()
    is_intermission = false
    first_spawn = { }
end)
server.event_handler("disconnect", function(cn)
    first_spawn[cn] = nil
    deathmillis[cn] = nil
end)

server.event_handler("suicide", function(cn)
    deathmillis[cn] = server.enet_time_get()
end)

server.event_handler("frag", function(cn)
    deathmillis[cn] = server.enet_time_get()
end)

server.event_handler("spawn", function(cn)
    if is_intermission then return end
    if first_spawn[cn] == nil then 
        first_spawn[cn] = true
        return
    end 

    if not gamemode_has_respawn_wait_time() then return end
    
    if server.player_connection_time(cn) <= 6 then return end
    
    local deathmillis = deathmillis[cn] or 0
    
    if deathmillis == 0 or server.gamemillis < 5000 then return end
    
    local spawntime = server.enet_time_get() - deathmillis
    
    --server.msg(string.format("(%s) spawntime: %d", server.player_name(cn), spawntime))
    
    if spawntime >= 0 and spawntime < min_spawntime then
        local first = spawnhack[cn] == nil
        if first then spawnhack[cn] = 1 end
        if not first then spawnhack[cn] = spawnhack[cn] + 1 end
        cheat(cn, 7, spawntime, "")
    end
end)