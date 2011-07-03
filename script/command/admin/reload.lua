-- [[ based on a player command written by Thomas ]] --

return function(cn)
	server.reload_lua
	server.msg(string.format(concat ((red ">>>") "Server has been" (blue "rehashed") "by" (blue"%s")), server.player_displayname(cn)))
end
