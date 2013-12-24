--[[
	An admin command for sending the map to a player
]]

local usage = "#sendmap <cn>"

return function(actor, target)
	if target and server.valid_cn(target) then
		server.sendmap(actor, target)
	else
		return false, usage
	end
end
