--[[
	An admin command for hiding edit from a player to others
]]

local usage = "#editmute 0|1 [<cn>]"
local editmute = false

local function unload()
	editmute = nil
	for p in server.gplayers() do
		p:vars().editmute = nil
	end
end

local function init()
	server.event_handler("editpacket", function(cn) 
		if editmute or server.player_vars(cn).editmute then
			return -1
		end
	end)
	server.event_handler("mapchange", unload)
end

local function run(cn, option, target)
	local option = tonumber(option)
	local target = tonumber(target)

	if option and (option == 0 or option == 1) then
		option = (option == 1)
	else
		return false, usage
	end

	if target then
		if server.valid_cn(target) then
			server.player_vars(target).editmute = option
		else
			return false, usage
		end
		local cn = target
	else
		editmute = option
	end

	if option then
		server.player_msg(cn, "editmute_enabled")
	else
		server.player_msg(cn, "editmute_disabled")
	end
end

return {init = init,run = run,unload = unload}
