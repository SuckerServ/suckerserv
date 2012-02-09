--[[
    
    Monitor player statistics for the purpose of detecting and broadcasting
    player achievements as one line messages. The idea for this feature was
    seen and copied from Frogmod.
    
    Copyright (C) 2009 Graham Daws
    
    MAINTAINERS
        Graham
        
    GUIDELINES
        * Messages should use the same colour as gameplay messages (yellow)
          with orange highlights
        * Consider sending achievement message privately to the subject player
        * Watch out for message flooding
        
    TODO
        * Keep and update a killing spree frags record (send private message 
          if they break their past record)
        * Achievement for consecutive hits
        * Report accuracy fluctuations
]]

-- Keep these messages at the top of the file so users can find them easily
local killingspree_message = {}
killingspree_message[5]  = yellow("%s is on a ") .. orange("KILLING SPREE!!")
killingspree_message[10] = yellow("%s is on a ") .. orange("RAMPAGE!!")
killingspree_message[15] = yellow("%s is ")      .. orange("DOMINATING!!")
killingspree_message[20] = yellow("%s is ")      .. orange("UNSTOPPABLE!!")
killingspree_message[30] = yellow("%s is ")      .. orange("GODLIKE!!")

local broadcast_minimum = 10
local long_killingspree = 15
local multikill_timelimit = 2000
local first_frag = true

local function send_first_frag_message(target, actor)
    if not first_frag or target == actor then return end
    server.msg(string.format(yellow("%s made the ") .. orange("FIRST KILL!!"), server.player_displayname(actor)))
    first_frag = false
end

local function send_killingspree_message(target_cn, target_vars, actor_cn, actor_vars)
    
    local actor_killingspree = actor_vars.killingspree or 0
    local target_killingspree = target_vars.killingspree or 0
    
    if actor_cn ~= target_cn then
        actor_killingspree = actor_killingspree + 1
        actor_vars.killingspree = actor_killingspree
    else
        actor_killingspree = 0
    end
    
    if killingspree_message[actor_killingspree] then
    
        local message = string.format(killingspree_message[actor_killingspree], server.player_displayname(actor_cn))
        
        if actor_killingspree < broadcast_minimum then
            server.player_msg(actor_cn, message)
        else
            server.msg(message)
        end
    end
    
    if target_killingspree >= long_killingspree then
        server.msg(string.format("\f2%s was stopped by \f6%s!!", server.player_displayname(target_cn), server.player_displayname(actor_cn)))
    end
    
    target_vars.killingspree = 0
end

local function send_multikills_message(target_cn, target_vars, actor_cn, actor_vars)
    
    local lastkill = actor_vars.lastkill or 0
    
    if actor_cn == target_cn then
        actor_vars.multikills = 1
        return
    end
    
    if server.gamemillis - lastkill < multikill_timelimit then
        
        local actor_multikills = (actor_vars.multikills or 1) + 1
        actor_vars.multikills = actor_multikills
        
        if (actor_multikills == 2) and gamemodeinfo.insta then
        
            server.player_msg(actor_cn, yellow("You scored a ") .. orange("DOUBLE KILL!!"))
        
        elseif actor_multikills == 3 then
        
            server.player_msg(actor_cn, yellow("You scored a ") .. orange("TRIPLE KILL!!"))
            
        elseif actor_multikills > 3 then
        
            server.player_msg(actor_cn, yellow("You scored ") ..  orange(string.format("%i MULTIPLE KILLS!!",actor_multikills)))
            
        end
        
    else -- reset
        actor_vars.multikills = 1
    end
    
    actor_vars.lastkill = server.gamemillis
    
end

server.event_handler("frag", function(target_cn, actor_cn)
    
    if server.is_teamkill(actor_cn, target_cn) then return end
    
    send_first_frag_message(target_cn, actor_cn)
    
    local target_vars = server.player_vars(target_cn)
    local actor_vars = server.player_vars(actor_cn)
    
    send_killingspree_message(target_cn, target_vars, actor_cn, actor_vars)
    
    send_multikills_message(target_cn, target_vars, actor_cn, actor_vars)
    
end)

server.event_handler("finishedgame",function()
    first_frag = true
    
    for player in server.gplayers() do
        player:vars().killingspree = 0
        player:vars().multikills = 1
        player:vars().lastkill = 0
    end
    
end)

server.event_handler("suicide", function(cn)
    server.player_vars(cn).killingspree = 0
    server.player_vars(cn).multikills = 1
end)

