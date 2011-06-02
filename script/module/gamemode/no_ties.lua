--[[
    like suddendeath: game will not end, while there is no clear ranking order
    increasess game time by one minute
    
    command function: no_ties(enable)
	to enable (enable = true)/ disable (enable = nil)
]]

local default_enabled = server.no_ties_enabled_by_default == 1

local is_enabled

local msg_overtime = "One minute overtime!"


function server.no_ties(enable)

    if not enable
    then
	is_enabled = nil
    else
	is_enabled = true
    end
end


server.event_handler("timeupdate", function(mins, secs)

    if (mins == 0) and (secs == 0) and is_enabled and not gamemodeinfo.edit
    then
	local scores = {}
	
	if gamemodeinfo.teams
	then
	    for _, team in pairs(server.teams())
	    do
		local score = server.team_score(team)
		
		if scores[score]
		then
		    server.changetime(60000)
		    server.msg(msg_overtime)
		    break
		end
		
		scores[score] = team
	    end
	else
    	    for p in server.gplayers()
    	    do
    		local score = p:score()
    		
    		if scores[score]
		then
		    server.changetime(60000)
		    server.msg(msg_overtime)
		    break
		end
		
		scores[score] = p.cn
	    end
	end
    end
end)

server.event_handler("finishedgame", function()

    is_enabled = nil
end)

server.event_handler("mapchange", function()

    is_enabled = default_enabled
end)

server.event_handler("connect", function()

    if server.playercount == 1
    then
	is_enabled = default_enabled
    end
end)


return {unload = function()

    is_enabled = nil
    
    server.unref("no_ties")
end}
