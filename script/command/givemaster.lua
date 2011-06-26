--[[

	A player command to raise player's privilege to master

]]


return function(cn, target)

	if not target then

		return false, "#givemaster <cn>"
	end

	if not server.valid_cn(target) then

		return false, "CN is not valid"
	end

	server.unsetmaster()
	server.player_msg(target, string.format(server.givemaster_message, server.player_displayname(cn)))
	server.setmaster(target)

end
