--[[
    CHEATER-DETECTION

    Copyright (C) 2011 Thomas
    
    TODO: 

        HOLD / CTF  -> check score distance
        
    COMMENTS:
    
        Speed Hack  -> logonly at this time, since i am not 100%
                       sure it doesn't produce false positives
                       
--]]

local network_packet_types = "N_CONNECT, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS, N_SHOOT, N_EXPLODE, N_SUICIDE, N_DIED, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH, N_GUNSELECT, N_TAUNT, N_MAPCHANGE, N_MAPVOTE, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD, N_PING, N_PONG, N_CLIENTPING, N_TIMEUP, N_MAPRELOAD, N_FORCEINTERMISSION, N_SERVMSG, N_ITEMLIST, N_RESUME, N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR, N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER, N_SETTEAM, N_BASES, N_BASEINFO, N_BASESCORE, N_REPAMMO, N_BASEREGEN, N_ANNOUNCE, N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO, N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS, N_TAKEFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_TRYDROPFLAG, N_DROPFLAG, N_SCOREFLAG, N_INITFLAGS, N_SAYTEAM, N_CLIENT, N_AUTHTRY, N_AUTHCHAL, N_AUTHANS, N_REQAUTH, N_PAUSEGAME, N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT, N_BOTBALANCE, N_MAPCRC, N_CHECKMAPS, N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHTEAM, NUMSV"
      network_packet_types = strSplit(network_packet_types, ", ")
local sound_types = "S_JUMP, S_LAND, S_RIFLE, S_PUNCH1, S_SG, S_CG, S_RLFIRE, S_RLHIT, S_WEAPLOAD, S_ITEMAMMO, S_ITEMHEALTH, S_ITEMARMOUR, S_ITEMPUP, S_ITEMSPAWN, S_TELEPORT, S_NOAMMO, S_PUPOUT, S_PAIN1, S_PAIN2, S_PAIN3, S_PAIN4, S_PAIN5, S_PAIN6, S_DIE1, S_DIE2, S_FLAUNCH, S_FEXPLODE, S_SPLASH1, S_SPLASH2, S_GRUNT1, S_GRUNT2, S_RUMBLE, S_PAINO, S_PAINR, S_DEATHR, S_PAINE, S_DEATHE, S_PAINS, S_DEATHS, S_PAINB, S_DEATHB, S_PAINP, S_PIGGR2, S_PAINH, S_DEATHH, S_PAIND, S_DEATHD, S_PIGR1, S_ICEBALL, S_SLIMEBALL, S_JUMPPAD, S_PISTOL, S_V_BASECAP, S_V_BASELOST, S_V_FIGHT, S_V_BOOST, S_V_BOOST10, S_V_QUAD, S_V_QUAD10, S_V_RESPAWNPOINT, S_FLAGPICKUP, S_FLAGDROP, S_FLAGRETURN, S_FLAGSCORE, S_FLAGRESET, S_BURN, S_CHAINSAW_ATTACK, S_CHAINSAW_IDLE, S_HIT"
      sound_types = strSplit(sound_types, ", ")
      
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

local ban_time = 21600 -- 6 hrs
local kick_msg = string.format(red("cheating - (bantime: %i minutes)"), round(ban_time / 60, 0))
local min_spawntime = 4600
local min_scoretime = 3000

local function kick(cn)
    server.kick(cn, ban_time, "CHEATER-DETECTION", kick_msg)
end

local function spec(cn, type)
    server.spec(cn)
    server.player_msg(cn, red("PLEASE TURN OFF YOUR CHEATS."))
end

local type = { }

type[1] = { kick,  "flag-score-hack (flags: %i)" }
type[2] = { kick,  "edit-packet-in-non-edit-mode (type: %s)" }
type[3] = { kick,  "unknown packet (type: %i)" }
type[4] = { kick,  "unknown-weapon (unknown-gun: %i)" }
type[5] = { nil,   "sent itemlist twice" }
type[6] = { nil,   "speed-hack (avg-lag: %i ms)" } 
type[7] = { kick,  "spawn-time-hack (spawntime: %i ms)" } 
type[8] = { kick,  "sent-unknown-sound (sound: %s)" } 
type[9] = { nil,   "invisible (invis-millis: %i)" } 
type[10] = { nil,  "getflag (distance: %i)" } 
type[11] = { kick, "modified-map-items" } 
type[12] = { kick, "modified-map-flags" } 
type[12] = { kick, "modified-capture-bases" } 

local logged = { }

server.event_handler("connect", function(cn)
    logged[cn] = { }
end)

server.event_handler("disconnect", function(cn)
    logged[cn] = nil
end)

local function cheat(cn, cheat_type, info)
    if cheat_type > #type or cheat_type < 1 then return end
        
    local action = type[cheat_type][1]
    local logmsg = 
        string.format("CHEATER: %s IP: %s LAG: %i GAMEMODE: %s MAP: %s CHEAT: ", 
            server.player_displayname(cn), 
            server.player_ip(cn),
            server.player_real_lag(cn),
            server.gamemode,
            server.map
        ) .. type[cheat_type][2]
        
    if logged[cn][cheat_type] == nil or cheat_type ~= 6 then
        if (cheat_type == 3 or cheat_type == 2) and info > 0 then
            info = network_type(info)
        end
        if cheat_type == 8 then
            info = sound_type(info)
        end
        server.log(string.format(logmsg, info) .. _if(action==nil," (TESTING)", ""))
        logged[cn][cheat_type] = true
    end
    
    if action ~= nil then
        action(cn)
    end
end

server.event_handler("cheat", cheat)

server.event_handler("spawn", function(cn)
    local gamemode = server.gamemode

    if 
       not string.find(gamemode, "hold") 
       and not string.find(gamemode, "ctf")
       and not gamemode == "capture" 
    then 
        return
    end
    
    if server.player_connection_time(cn) == 0 then return end
    
    local deathmillis = server.player_deathmillis(cn)
    
    if deathmillis == 0 or server.gamemillis < 5000 then return end
    
    local spawntime = server.gamemillis - deathmillis
    
    if spawntime > 0 and spawntime < min_spawntime then
        cheat(cn, 7, spawntime)
    end
end)
