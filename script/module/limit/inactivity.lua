--[[
    A script to move inactive players to spectators
]]

local interval_time = server.inactivity_check_time
if interval_time <= 0 then
    interval_time = 60000
end
local inactive_time = server.inactivity_time
local death_only = server.inactivity_death_only == 1

local is_unload


local function clean(cn)

    server.player_vars(cn).inactivity_x = nil
    server.player_vars(cn).inactivity_y = nil
    server.player_vars(cn).inactivity_z = nil
    server.player_vars(cn).inactivity_time = nil
end

local function mrproper()

    for p in server.gplayers() do
	clean(p.cn)
    end
end

local function update(cn)

    server.player_vars(cn).inactivity_x, server.player_vars(cn).inactivity_y, server.player_vars(cn).inactivity_z = server.player_pos(cn)
    server.player_vars(cn).inactivity_time = server.gamemillis
end

server.event_handler("finishedgame", mrproper)

server.event_handler("spectator", function(cn, joined)

    if joined == 1
    then
	clean(cn)
    else
	update(cn)
    end
end)

server.event_handler("frag", update)	-- function(tcn, acn)

server.event_handler("suicide", update)

server.interval(interval_time, function()

    if is_unload
    then
	return -1
    end
    
    if server.mastermode < 2
    then
	for p in server.gplayers()
	do
	    local last_x, last_y, last_z = p:vars().inactivity_x, p:vars().inactivity_y, p:vars().inactivity_z
            
            if not (last_x and last_y and last_z)
            then
        	update(p.cn)
            else
        	local x, y, z = p:pos()
		
		if (last_x == x) and (last_y == y) and (last_z == z)
        	then
        	    local last_time = p:vars().inactivity_time
		    local con_time = server.gamemillis
        	
        	    if ((con_time - last_time) >= inactive_time) and (not death_only or (p:status_code() == server.DEAD))
        	    then
	    		p:msg(server.inactivitylimit_message)
			p:spec()
			server.log("Server moved " .. p:name() .. " to spectator, because of inactivity.")
		    end
		else
		    update(p.cn)
		end
	    end
	end
    end
end)


return {unload = function()

    is_unload = true
    
    mrproper()
end}
