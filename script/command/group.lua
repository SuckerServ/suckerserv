--[[

	A player command to group all players matching pattern <tag>

]]


return function(cn,arg1,arg2,arg3)

	if not gamemodeinfo.teams then

		return
	end

	if not arg1 then

		return false, "#group [all] <tag> [<team>]"
	end

	server.group_players(arg1,arg2,arg3)

end
