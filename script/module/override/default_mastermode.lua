local default_mastermode = server.default_mastermode

local default_reset_matermode = server.reset_mastermode
server.reset_mastermode = 0 -- otherwise the mastermode is reset to auth if the last master drops privileges

server.event_handler("disconnect", function(cn, reason)

	if server.playercount == 0 then
		server.mastermode = default_mastermode
	end
end)

server.event_handler("started", function()
	server.mastermode = default_mastermode
end)

local function unload()
	server.mastermode = 0
	server.reset_mastermode = default_reset_matermode
end

return {unload = unload}
