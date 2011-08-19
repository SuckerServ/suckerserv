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
local min_spawntime = 4600
local min_scoretime = 3000


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
    return string.format("unknown (%i)", sound)
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



local type = { }

type[1] = { kick,  "flag-score-hack (flags: %i)" }
type[2] = { kick,  "edit-packet-in-non-edit-mode (type: %s)" }
type[3] = { kick,  "unknown packet (type: %i)" }
type[4] = { kick,  "unknown-weapon (unknown-gun: %s)" }
type[6] = { nil,   "speed-hack-ping (avg-speed: %.2fx)" } 
type[7] = { kick,  "spawn-time-hack (spawntime: %i ms)" } 
type[8] = { kick,  "sent-unknown-sound (sound: %s)" } 
type[9] = { nil,   "invisible (invis-millis: %i)" } 
type[10] = { nil,  "getflag (distance: %i)" } 
type[11] = { kick, "modified-map-items" } 
type[12] = { kick, "modified-map-flags" } 
type[13] = { kick, "modified-capture-bases" } 
type[14] = { gun,  "shoot-out-of-gun-distance-range", true } 
type[15] = { kick, "scored-in-less-than-3-seconds (%i ms)" } 
type[16] = { nil,  "speed-hack-pos (avg-speed: %.2fx)" } 

local logged = { }

server.event_handler("connect", function(cn)
    logged[cn] = { }
end)

server.event_handler("disconnect", function(cn)
    logged[cn] = nil
end)

local function cheat(cn, cheat_type, info, info_str)
    if cheat_type > #type or cheat_type < 1 then return end
    
    if type[cheat_type][2] == nil then return end
        
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
    
        if (cheat_type == 3 or cheat_type == 2) and info > 0 then info = network_type(info) end
        if cheat_type == 4 then info = gun_type(info) end
        if cheat_type == 8 then info = sound_type(info) end
		if cheat_type == 14 then info_str = string.format(info_str, gun_type(info)) end
        if cheat_type == 6 or cheat_type == 16 then info = info / 1000 end

		if info_str ~= "" then info_str = " (INFO: " .. info_str .. ")" end
		
        server.log(string.format(logmsg, info) .. info_str .. _if(action==nil," (TESTING)", ""))
        logged[cn][cheat_type] = true
        
    end
    
    if action ~= nil then
        if type[cheat_type][2] ~= nil then
            action(cn, info)
        else
            action(cn)
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

server.event_handler("spawn", function(cn)
    local gamemode = server.gamemode

    if not gamemode_has_respawn_wait_time() then return end
    
    if server.player_connection_time(cn) == 0 then return end
    
    local deathmillis = server.player_deathmillis(cn)
    
    if deathmillis == 0 or server.gamemillis < 5000 then return end
    
    local spawntime = server.gamemillis - deathmillis
    
    if spawntime > 0 and spawntime < min_spawntime then
        cheat(cn, 7, spawntime, "")
    end
end)


server.event_handler("scoreflag", function(cn, _, __, timetrial)
    if timetrial > - 1 and timetrial <= 3000 then
        server.msg("cheat")
        cheat(cn, 15, timetrial, "")
    end
end)
