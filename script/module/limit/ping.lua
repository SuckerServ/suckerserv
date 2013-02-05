--[[
    A script to move lagging players to spectators after 2 warnings, each time, lags are detected
]]

local interval_time = server.ping_check_time
local ping_limit = server.ping_limit
local pj_limit = server.ping_pj_limit

if interval_time <= 0 then
    interval_time = 60000
end
local max_warnings = 2

local is_intermission
local is_unload


local function unset_vars(cn)

    server.player_vars(cn).ping_warnings = nil
end


server.event_handler("disconnect", unset_vars)

server.event_handler("intermission", function()

    is_intermission = true
end)

server.event_handler("mapchange", function()

    is_intermission = nil
end)

server.interval(interval_time, function()

    if is_unload
    then
	return -1
    end
    
    if is_intermission or (server.playercount < 3)
    then
	return
    end
    
    for p in server.gplayers()
    do
	local ping = p:ping()
	local pj = p:lag()
	
        if (pj > pj_limit) and (ping > ping_limit)
        then
    	    local warns = p:vars().ping_warnings or 0
    	    
            if warns < max_warnings
            then
        	p:msg(red(string.format("Your ping is too high! The server expects pings to be below %i ms.", ping_limit)))
        	p:vars().ping_warnings = warns + 1
            else
                p:msg(server.pinglimit_message)
                p:spec()
                server.log("Server moved " .. p:name() .. " to spectators, because of lags. (ping: " .. ping .. ", pj: " .. pj .. ")")
            end
        end
    end
end)


local function unload()

    is_unload = true
    
    for p in server.gclients() do
	unset_vars(p.cn)
    end
end


return {unload = unload}
