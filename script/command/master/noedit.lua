--[[
	An admin command for spectating player for a defined time when entering edit mode
]]

local usage = "#noedit 0|1"
local noedit = false
local SPEC_TIME = 5000

local function init()
	server.event_handler("editmode", function(cn, val) 
		if noedit and val then
			local sid = server.player_sessionid(cn)
			server.force_spec(cn)
			server.sleep(SPEC_TIME, function()
				if sid == server.player_sessionid(cn) then
					server.unforce_spec(cn)
					server.player_respawn(cn)
				end
			end)
		end
	end)
	server.event_handler("mapchange", function() noedit = false end)
end

local function unload()
	noedit = nil
end

local function run(cn, option)
	option = tonumber(option)

	if option and (option == 0 or option == 1) then
		noedit = (option == 1)
	else
		return false, usage
	end

	if noedit then
		server.player_msg(cn, "noedit_enabled")
	else
		server.player_msg(cn, "noedit_disabled")
	end
end

return {init = init,run = run,unload = unload}
