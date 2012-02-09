-- [[ based on a player command written by Thomas ]] --

return function(cn, n)
    if not n then
        return false, "#maxclients <size>"
    end

    n = tonumber(n)

    if n >= server.playercount and n <= 128 and n >= 0 then
        server.maxplayers = n
    else
        server.player_msg(cn, string.format("size needs to be >= %d and <= 128", _if(server.playercount > 0, server.playercount, 1)))
    end 
end