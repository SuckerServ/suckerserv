--[[
    CHEATER-DETECTION

    Copyright (C) 2011 Thomas
    
    TODO: 

        HOLD / CTF  -> check score distance
        
    COMMENTS:
    
        LOGS WITH (TESTING) ARE !!_NOT_!! 100% secure proofs
    
--]]

local ban_time = 21600 -- 6 hrs
local kick_msg = string.format(red("cheating - (bantime: %i minutes)"), round(ban_time / 60, 0))
local min_spawntime = 4950
local min_scoretime = 3000
local lag_checktime = 0 -- 0 to disable (10000 = Check all 10 sec)
local max_ping = 800
local max_pj = 70
local spawnhack_force_death = false -- force player to suicide 1 sec after first spawnhack was detected
local spawnhack = {}
local speedhack_low = { }
local invisiblehack = { }

local network_packet_types = "N_CONNECT, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS, N_SHOOT, N_EXPLODE, N_SUICIDE, N_DIED, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH, N_GUNSELECT, N_TAUNT, N_MAPCHANGE, N_MAPVOTE, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD, N_PING, N_PONG, N_CLIENTPING, N_TIMEUP, N_MAPRELOAD, N_FORCEINTERMISSION, N_SERVMSG, N_ITEMLIST, N_RESUME, N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR, N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER, N_SETTEAM, N_BASES, N_BASEINFO, N_BASESCORE, N_REPAMMO, N_BASEREGEN, N_ANNOUNCE, N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO, N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS, N_TAKEFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_TRYDROPFLAG, N_DROPFLAG, N_SCOREFLAG, N_INITFLAGS, N_SAYTEAM, N_CLIENT, N_AUTHTRY, N_AUTHCHAL, N_AUTHANS, N_REQAUTH, N_PAUSEGAME, N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT, N_BOTBALANCE, N_MAPCRC, N_CHECKMAPS, N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHTEAM, NUMSV"
      network_packet_types = strSplit(network_packet_types, ", ")
local sound_types = "S_JUMP, S_LAND, S_RIFLE, S_PUNCH1, S_SG, S_CG, S_RLFIRE, S_RLHIT, S_WEAPLOAD, S_ITEMAMMO, S_ITEMHEALTH, S_ITEMARMOUR, S_ITEMPUP, S_ITEMSPAWN, S_TELEPORT, S_NOAMMO, S_PUPOUT, S_PAIN1, S_PAIN2, S_PAIN3, S_PAIN4, S_PAIN5, S_PAIN6, S_DIE1, S_DIE2, S_FLAUNCH, S_FEXPLODE, S_SPLASH1, S_SPLASH2, S_GRUNT1, S_GRUNT2, S_RUMBLE, S_PAINO, S_PAINR, S_DEATHR, S_PAINE, S_DEATHE, S_PAINS, S_DEATHS, S_PAINB, S_DEATHB, S_PAINP, S_PIGGR2, S_PAINH, S_DEATHH, S_PAIND, S_DEATHD, S_PIGR1, S_ICEBALL, S_SLIMEBALL, S_JUMPPAD, S_PISTOL, S_V_BASECAP, S_V_BASELOST, S_V_FIGHT, S_V_BOOST, S_V_BOOST10, S_V_QUAD, S_V_QUAD10, S_V_RESPAWNPOINT, S_FLAGPICKUP, S_FLAGDROP, S_FLAGRETURN, S_FLAGSCORE, S_FLAGRESET, S_BURN, S_CHAINSAW_ATTACK, S_CHAINSAW_IDLE, S_HIT"
      sound_types = strSplit(sound_types, ", ")
local gun_types = "S_PUNCH1, S_SG, S_CG, S_RLFIRE, S_RIFLE, S_FLAUNCH, S_PISTOL, S_FLAUNCH, S_ICEBALL, S_SLIMEBALL, S_PIGR1"
      gun_types = strSplit(gun_types, ", ")
      
 
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

local function is_known_map(map)
    return supported_maps[map] ~= nil
end

local function admin_msg(msg)
    for i, cn in ipairs(server.clients()) do
        if server.player_priv_code(cn) >= 2 then
            server.player_msg(cn, msg)
        end
    end
end

local function kick(cn, _)
    server.kick(cn, ban_time, "CHEATER-DETECTION", kick_msg)
end

local function gun(cn, selgun)
    if gun_type(selgun) == "S_PUNCH1" then -- fist / chainsaw or what ever it is named
        kick(cn)
    end
end

local function spec(cn, type)
    server.spec(cn)
    server.player_msg(cn, red("PLEASE TURN YOUR CHEATS OFF."))
end

local function lag(cn)
    server.spec(cn)
    server.player_msg(cn, red("You have been specced due to lag / very high ping."))   
end

local function speedhack(cn, speed)
    if speed >= 1.6 then
        kick(cn)
    else
        local first = speedhack_low[cn] == nil
        if first then speedhack_low[cn] = 1 end
        if not first then speedhack_low[cn] = speedhack_low[cn] + 1 end    
        if speedhack_low[cn] >= 3 then
            kick(cn)
        end
    end
end

local function spawn_hack(cn)
    if spawnhack[cn] >= 2 then
        kick(cn)
    else
        if not spawnhack_force_death then return end
        local ses = server.player_sessionid(cn)
        server.sleep(1000, function()
            for i, v in ipairs(server.players()) do
                if ses == server.player_sessionid(v) then
                    server.player_suicide(v)
                end
            end
        end)
    end
end

local function invisible_hack(cn, speed)
    local first = invisiblehack[cn] == nil
    if first then invisiblehack[cn] = 1 end
    if not first then invisiblehack[cn] = invisiblehack[cn] + 1 end    
    if invisiblehack[cn] >= 3 then
        kick(cn)
    end
end


local type = { }

type[1] = { kick,  "flag-score-hack (flags: %i)", "flag-limit exceeded" }
type[2] = { kick,  "edit-packet-in-non-edit-mode (type: %s)", "fly hack" }
type[3] = { kick,  "unknown packet (type: %i)", "unknown packet" }
type[4] = { kick,  "unknown-weapon (unknown-gun: %s)", "unknown weapon" }
type[6] = { nil,   "speed-hack-ping (avg-speed: %.2fx)", "speedhack ping" } 
type[7] = { spawn_hack,  "spawn-time-hack (spawntime: %i ms)", "spawntime hack" } 
type[8] = { kick,  "sent-unknown-sound (sound: %s)", "unknown sound" } 
type[9] = { invisible_hack, "invisible (invis-millis: %i)", "invisible hack" } 
type[10] = { nil,  "getflag (distance: %i)", "getflag" } 
type[11] = { kick, "modified-map-items", "modified map items" } 
type[12] = { kick, "modified-map-flags", "modified map flags" } 
type[13] = { kick, "modified-capture-bases", "modified capture bases" } 
type[14] = { nil,  "shoot-out-of-gun-distance-range", "shoot distance" } 
type[15] = { kick, "scored-in-less-than-" .. round(min_scoretime / 1000, 1) .. "-seconds (%i ms)", "flag score hack" } 
type[16] = { speedhack, "speed-hack-pos (avg-speed: %.2fx)", "speedhack" } 
type[17] = { nil,  "jumphack (height: %.2f)", "jumphack" } 
type[18] = { lag,  "lag / high ping", "lag" } 
type[19] = { kick, "unknown-map-item %s", "unknown map item" } 
type[20] = { nil,  "map-item-not-spawned (item: %i)", "item not spawned" } 
type[21] = { kick, "map-item-not-spawned (item: %i)" } 
type[22] = { nil,  "impossible-player-action", "impossible player action" } 
type[23] = { nil,  "player-position exceeded (pos: %i)", "player position exceeded" } 
type[24] = { invisible_hack, "null-player-position / invisible", "player position / invisible" } 

local logged = { }

server.event_handler("connect", function(cn)
    logged[cn] = { }
end)

server.event_handler("disconnect", function(cn)
    logged[cn] = nil
    spawnhack[cn] = nil
    speedhack_low[cn] = nil
    invisiblehack[cn] = nil
end)

local function cheat(cn, cheat_type, info, info_str)
    if cheat_type > #type or cheat_type < 1 then return end
    
    if cheat_type >= 11 and cheat_type <= 13 and not is_known_map(server.map) then
        return
    end

    if type[cheat_type][2] == nil then return end
        
    local action = type[cheat_type][1]
    local logmsg = 
        string.format("CHEATER: %s IP: %s PING: %i LAG: %i GAMEMODE: %s MAP: %s CHEAT: ", 
            server.player_displayname(cn), 
            server.player_ip(cn),
            server.player_ping(cn),
            server.player_real_lag(cn),
            server.gamemode,
            server.map
        ) .. type[cheat_type][2]
        
    if logged[cn][cheat_type] == nil or cheat_type ~= 6 then
    
        if (cheat_type == 3 or cheat_type == 2) and info > 0 then info = network_type(info) end
        if cheat_type == 4 then info = gun_type(info) end
        if cheat_type == 8 then info = sound_type(info) end
		if cheat_type == 14 then info_str = string.format(info_str, gun_type(info)) end
        if cheat_type == 6 or cheat_type == 16 or cheat_type == 17 then info = info / 100000 end

        if info_str ~= "" then info_str = " (INFO: " .. info_str .. ")" end
        
        local testing = action == nil
        local log = string.format(logmsg, info) .. info_str .. _if(testing, " (TESTING)", "")
        
        server.log(log)
        logged[cn][cheat_type] = true
        
        if type[cheat_type][3] ~= nil then
            admin_msg(string.format("\f3\f8CHEAT-DETECTION\f3\f8 %s -> %s%s", blue(server.player_displayname(cn)), red(type[cheat_type][3]),  orange(_if(testing, " (TESTING)", ""))))
        end
        
    end
    
    if action ~= nil then
        action(cn, info)
    end
end

server.event_handler("cheat", cheat)

if lag_checktime > 0 then
    server.interval(lag_checktime, function()
        for i, cn in ipairs(server.players()) do
            if server.player_ping >= max_ping or server.player_lag >= max_lag then
                if server.gamemillis - server.player_maploaded(cn) > lag_checktime then
                    cheat(cn, 18, -1, "")
                end
            end
        end
    end)
end


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

server.event_handler("spawn", function(cn)
    local gamemode = server.gamemode

    if not gamemode_has_respawn_wait_time() then return end
    
    if server.player_connection_time(cn) <= 6 then return end
    
    local deathmillis = server.player_deathmillis(cn)
    
    if deathmillis == 0 or server.gamemillis < 5000 then return end
    
    local spawntime = server.gamemillis - deathmillis
    
    --server.msg(string.format("(%s) spawntime: %d", server.player_name(cn), spawntime))
    
    if spawntime >= 0 and spawntime < min_spawntime then
        local first = spawnhack[cn] == nil
        if first then spawnhack[cn] = 1 end
        if not first then spawnhack[cn] = spawnhack[cn] + 1 end
        cheat(cn, 7, spawntime, "")
    end
end)


server.event_handler("scoreflag", function(cn, _, __, timetrial)  
    if timetrial > -1 and timetrial <= min_scoretime and is_known_map(server.map) then
        cheat(cn, 15, timetrial, "")
    end
end)
