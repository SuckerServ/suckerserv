-- [[ based on a player command written by Thomas ]] --

return function(cn)
	server.reload_lua
	server.msg(string.format(server.reload_message, server.player_displayname(cn)))
end
