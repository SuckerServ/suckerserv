--[[
    Display best player stats at intermission
    
    Copyright (C) 2009 Graham Daws

    01/08/2011 Thomas: use score() instead of frags(), better kpd calculation, use client way for accuracy
]]

local stats = server.player_vars_table(server.player_id)

local stats_record_class = {
    
    update = function(object, cn, value, desc)
        
        if not object.value then
            object.value = value
            object.cn = {cn}
            return
        end
        
        if desc ~= nil then
            if value > object.value then return end 
        else
            if value < object.value then return end
        end
        
        if value == object.value then
            table.insert(object.cn, cn)
        else
            object.value = value
            object.cn = {cn}
        end
    end,
    
    list_names = function(object)
        return print_displaynamelist(object.cn)
    end,
    
    clear = function(object)
        object.cn = nil
        object.value = nil
    end
}

local best = {
    frags      = {},
    kpd        = {},
    accuracy   = {},
    damage     = {},
    takeflag   = {},
    scoreflag  = {},
    returnflag = {},
    timetrial  = {}
}

for _, subobject in pairs(best) do
    setmetatable(subobject, {__index = stats_record_class})
end

server.event_handler("intermission", function()

    local totalplayercount = server.playercount + server.speccount + server.botcount
    if totalplayercount < 2 then return end
    
    local check_ctf_stats = gamemodeinfo.ctf
    
    local function update_stats(player)
    
        local cn = player.cn
        local qualify = player:score() > round(server.gamemillis / 60000); -- game duration in mins
        
        if qualify then
            best.accuracy:update(cn, player:accuracy2())
            best.frags:update(cn, player:score())
            best.kpd:update(cn, player:score() - player:deaths())
	    if not gamemodeinfo.insta then
                best.damage:update(cn, player:damage())
	    end
        end
        
        if check_ctf_stats then
            
            local player_stats = stats.vars(cn)
            
            best.takeflag:update(cn, player_stats.takeflag or 0)
            best.scoreflag:update(cn, player_stats.scoreflag or 0)
            best.returnflag:update(cn, player_stats.returnflag or 0)

            local timetrial = player_stats.timetrial or 0
            if timetrial > 0 then
                best.timetrial:update(cn, timetrial, true)
            end

        end
        
    end
    
    for p in server.gplayers() do
        update_stats(p)
    end
    
    for b in server.gbots() do
        update_stats(b)
    end
    
    if best.kpd.value then
        local cn = best.kpd.cn[1]
        local frags = server.player_score(cn)
        local deaths = server.player_deaths(cn)
        if deaths == 0 then deaths = 1 end
        best.kpd.value = round(frags / deaths, 2)
    end

    server.awards = best
    
    local function format_message(record_name, record, append)
        if not record.value or #record.cn > 2 then return "" end
        if gamemodeinfo.insta and record_name == "damage" then return end
        append = append or ""
        return blue(string.format("%s: %s", record_name, white(record:list_names()) .. " " .. red(record.value .. append)))
    end

    local stats_message = print_list(
        format_message("frags", best.frags),
        format_message("kpd", best.kpd),
        format_message("accuracy", best.accuracy, "%"),
        format_message("damage", best.damage))
        
    if #stats_message > 0 then
        server.msg("awards_stats", {stats = stats_message})
    end
    
    if check_ctf_stats then
        
        local flagstats_message = print_list(
            format_message("stolen", best.takeflag), 
            format_message("scored", best.scoreflag),
            format_message("returned", best.returnflag),
            format_message("flagrun", best.timetrial, " ms"))
        
        if #flagstats_message > 0 then
			server.msg("awards_flags", {flagstats = flagstats_message})
        end
    end
    
    stats.clear()
    
    for _, record in pairs(best) do
        record:clear()
    end
end)

server.event_handler("takeflag", function(cn)
    local player_stats = stats.vars(cn)
    player_stats.takeflag = (player_stats.takeflag or 0) + 1
end)

server.event_handler("scoreflag", function(cn, _, __, timetrial)
    local player_stats = stats.vars(cn)
    player_stats.scoreflag = (player_stats.scoreflag or 0) + 1
    player_stats.timetrial = (player_stats.timetrial or 0)
    if timetrial == 0 then return end
    if player_stats.timetrial == 0 or player_stats.timetrial > timetrial then
        player_stats.timetrial = timetrial
    end
end)

server.event_handler("returnflag", function(cn)
    local player_stats = stats.vars(cn)
    player_stats.returnflag = (player_stats.returnflag or 0) + 1
end)

