--[[
	A player command to raise player's privilege to master
]]

local usage = "#givemaster <cn>"

return function(cn, target)

	if not target then

		return false, usage
	end

	if not server.valid_cn(target) then
		return false, "CN is not valid"
	end

	server.unsetpriv(cn)
	server.player_msg(target, string.format(server.givemaster_message, server.player_displayname(cn)))
	server.admin_log(string.format("GIVEADMIN: %s gave master to %s", server.player_displayname(cn), server.player_displayname(target)))
	if not (server.player_priv_code(target) >= server.PRIV_MASTER) then
		server.setmaster(target)
	end
end
