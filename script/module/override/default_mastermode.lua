local default_mastermode = server.default_mastermode

server.event_handler("setmaster", function(cn)

	if server.mastermode < default_mastermode then
		server.mastermode = default_mastermode
	end
end)

server.event_handler("disconnect", function(cn, reason)

	if server.playercount == 0 then
		server.mastermode = default_mastermode
	elseif server.mastermode < default_mastermode then
		server.mastermode = default_mastermode
	end
end)

server.event_handler("started", function()
	server.mastermode = default_mastermode
end)

local function unload()
	server.mastermode = 0
end

return {unload = unload}
