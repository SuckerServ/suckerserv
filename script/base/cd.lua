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
type[2] = { kick, "edit-packet-in-non-edit-mode (type: %i)" }
type[3] = { kick, "unknown packet (type: %i)" }
type[4] = { kick, "unknown-weapon (unknown-gun: %i)" }
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
        server.log(string.format(logmsg, info))
        logged[cn][cheat_type] = true
    end
    
    if action ~= nil then
        action(cn)
    end
            
end)