--[[
    CHEATER-DETECTION

    Copyright (C) 2011 Thomas
    
    TODO: 
    
        Add Jumppad -> Check (check if jumppad exists)
        Teleporter  -> Check if teleporter exists)
        Getflag     -> distance / lag-check
        Respawn     -> Check respawntime in ctf/capture etc
        
    COMMENTS:
    
        Speed Hack  -> logonly at this time, since i am not 100%
                       sure it doesn't produce false positives
                       
--]]

local network_packet_types = "N_CONNECT, N_SERVINFO, N_WELCOME, N_INITCLIENT, N_POS, N_TEXT, N_SOUND, N_CDIS, N_SHOOT, N_EXPLODE, N_SUICIDE, N_DIED, N_DAMAGE, N_HITPUSH, N_SHOTFX, N_EXPLODEFX, N_TRYSPAWN, N_SPAWNSTATE, N_SPAWN, N_FORCEDEATH, N_GUNSELECT, N_TAUNT, N_MAPCHANGE, N_MAPVOTE, N_ITEMSPAWN, N_ITEMPICKUP, N_ITEMACC, N_TELEPORT, N_JUMPPAD, N_PING, N_PONG, N_CLIENTPING, N_TIMEUP, N_MAPRELOAD, N_FORCEINTERMISSION, N_SERVMSG, N_ITEMLIST, N_RESUME, N_EDITMODE, N_EDITENT, N_EDITF, N_EDITT, N_EDITM, N_FLIP, N_COPY, N_PASTE, N_ROTATE, N_REPLACE, N_DELCUBE, N_REMIP, N_NEWMAP, N_GETMAP, N_SENDMAP, N_CLIPBOARD, N_EDITVAR, N_MASTERMODE, N_KICK, N_CLEARBANS, N_CURRENTMASTER, N_SPECTATOR, N_SETMASTER, N_SETTEAM, N_BASES, N_BASEINFO, N_BASESCORE, N_REPAMMO, N_BASEREGEN, N_ANNOUNCE, N_LISTDEMOS, N_SENDDEMOLIST, N_GETDEMO, N_SENDDEMO, N_DEMOPLAYBACK, N_RECORDDEMO, N_STOPDEMO, N_CLEARDEMOS, N_TAKEFLAG, N_RETURNFLAG, N_RESETFLAG, N_INVISFLAG, N_TRYDROPFLAG, N_DROPFLAG, N_SCOREFLAG, N_INITFLAGS, N_SAYTEAM, N_CLIENT, N_AUTHTRY, N_AUTHCHAL, N_AUTHANS, N_REQAUTH, N_PAUSEGAME, N_ADDBOT, N_DELBOT, N_INITAI, N_FROMAI, N_BOTLIMIT, N_BOTBALANCE, N_MAPCRC, N_CHECKMAPS, N_SWITCHNAME, N_SWITCHMODEL, N_SWITCHTEAM, NUMSV"
      network_packet_types = strSplit(network_packet_types, ", ")

local function network_type(packet)
    packet = packet + 1
    if #network_packet_types >= packet then
        return network_packet_types[packet]
    end
    return "unknown"
end

local ban_time = 21600 -- 6 hrs
local kick_msg = string.format(red("cheating - (bantime: %i minutes)"), round(ban_time / 60, 0))

local function kick(cn)
    server.kick(cn, ban_time, "CHEATER-DETECTION", kick_msg)
end

local function spec(cn, type)
    server.spec(cn)
    server.player_msg(cn, red("PLEASE TURN OFF YOUR CHEATS."))
end

local type = { }

type[1] = { kick, "flag-score-hack (flags: %i)" }
type[2] = { kick, "edit-packet-in-non-edit-mode (type: %s)" }
type[3] = { kick, "unknown packet (type: %i)" }
type[4] = { kick, "unknown-weapon (unknown-gun: %s)" }
type[5] = { kick, "sent itemlist twice" }
type[6] = { nil,  "speed-hack (avg-lag: %i ms)" } 

local logged = { }

server.event_handler("connect", function (cn)
    logged[cn] = { }
end)

server.event_handler("disconnect", function (cn)
    logged[cn] = nil
end)


server.event_handler("cheat", function (cn, cheat_type, info)

    if cheat_type > #type or cheat_type < 1 then return end
        
    local action = type[cheat_type][1]
    local logmsg = string.format("CHEATER: %s IP: %s CHEAT: ", server.player_displayname(cn), server.player_ip(cn)) .. type[cheat_type][2]
    
    if logged[cn][cheat_type] == nil then
        if (cheat_type == 3 or cheat_type == 2) and info > 0 then
            info = network_type(info)
        end
        server.log(string.format(logmsg, info))
        logged[cn][cheat_type] = true
    end
    
    if action ~= nil then
        action(cn)
    end
            
end)