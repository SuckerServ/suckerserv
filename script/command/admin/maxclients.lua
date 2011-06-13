-- [[ based on a player command written by Thomas ]] --

return function(cn, n)
    
	if not n then
		return false, "#maxclients <size>"
	end

	n = tonumber(n)

	if n >= server.playercount and n <= 128 then
		server.maxplayers = n
	end
    
end
