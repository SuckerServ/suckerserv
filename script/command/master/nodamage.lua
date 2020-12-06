--[[
	An admin command to disable damages
]]

local usage = "#nodamage 0|1"
local nodamage = false

local function init()
	nodamage = false
	server.event_handler("damage", function()
		if nodamage then
			return -1
		end
	end)
	server.event_handler("mapchange", function() nodamage = false end)
end

local function unload()
	nodamage = nil
end

local function run(cn, option)
	if not option then
		return false, usage
	elseif tonumber(option) == 1 then
		nodamage = true
		server.player_msg(cn, "nodamage_enabled")
	elseif tonumber(option) == 0 then
		nodamage = false
		server.player_msg(cn, "nodamage_disabled")
	else
		return false, usage
	end
end

return {init = init,run = run,unload = unload}
