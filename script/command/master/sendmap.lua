--[[
	An admin command for sending the map to a player
]]

local usage = "#sendmap <cn>|all"

return function(actor, target)
	if target == "all" then
		for p in server.gclients() do
			if p.cn ~= actor then
				server.sendmap(actor, p.cn)
			end
		end
	elseif target and server.valid_cn(target) then
		server.sendmap(actor, target)
	else
		return false, usage
	end
end
