--[[
    Display awards at intermission
    
    Copyright (C) 2011 LoveForEver
	
	Inspired bu best_stats script from Hopmod
]]
local stats = server.player_vars_table(server.player_id)

local stats_record_class = {
    
    update = function(object, cn, value)
        
        if not object.value then
            object.value = value
            object.cn = {cn}
            return
        end
        
        if value < object.value then return end
        
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
    takeflag   = {},
    scoreflag  = {},
    returnflag = {}
}

for _, subobject in pairs(best) do
    setmetatable(subobject, {__index = stats_record_class})
end

server.event_handler("intermission", function()

    local totalplayercount = server.playercount + server.speccount + server.botcount
    if totalplayercount < 2 then return end
	
	local function update_stats(player)
    
        local cn = player.cn
        local participated = player:frags()
        
        if participated then
            best.accuracy:update(cn, player:accuracy())
            best.frags:update(cn, player:frags())
            best.kpd:update(cn, player:frags() - player:deaths())
			
			local player_stats = stats.vars(cn)
			best.takeflag:update(cn, player_stats.takeflag or 0)
            best.scoreflag:update(cn, player_stats.scoreflag or 0)
            best.returnflag:update(cn, player_stats.returnflag or 0)
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
        best.kpd.value = round((server.player_frags(cn)+1)/(server.player_deaths(cn)+1), 2)
    end
    
    local function format_message(record_name, record, append)
        if not record.value or #record.cn > 2 then return "" end
        append = append or ""
        return blue(string.format("%s: %s", record_name, white(record:list_names()) .. " " .. red(record.value .. append)))
    end
    
	local stats_message = print_list(
        format_message("frags", best.frags),
        format_message("kpd", best.kpd),
        format_message("accuracy", best.accuracy, "%"),
		format_message("flag scorere", best.scoreflag))
        
    if #stats_message > 0 then
    server.msg(string.format(server.awards_message, stats_message))
    end
	
    stats.clear()
    
    for _, record in pairs(best) do
        record:clear()
    end
end)