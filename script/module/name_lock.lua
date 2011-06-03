-- Module to Prevent Players from renaming
-- (c) 2010 Thomas

local no_rename_event = false

server.event_handler("rename", function(cn, old, new)
	if no_rename_event then return end
	no_rename_event = true
	server.sleep(10, function()
		server.player_rename(cn, old, true)
		server.player_msg(cn, server.player_namelock_message)
		no_rename_event = false
	end)
end)
