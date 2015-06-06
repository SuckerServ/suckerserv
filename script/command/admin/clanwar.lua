--[[
  A little clanwar command 
  Based on fairgame command + pause when someone leave and resume with delay if all players are present
  By piernov <piernov@piernov.org>
]]


local usage = "#cw <map> [<mode>] [lockteams]"

return function(cn, map, mode, lockteams)
        if not server.clanwar then
            return false, "clanwar module is not loaded."
        end

	if not map then
		return false, "#cw <map> [<mode>] [lockteams]"
	end

	mode = mode or server.gamemode

	if not server.parse_mode(mode) then
		return false, server.unrecognized_gamemode_message
	else
		mode = server.parse_mode(mode)
	end

        server.clanwar(cn, true, map, mode, lockteams)

end
