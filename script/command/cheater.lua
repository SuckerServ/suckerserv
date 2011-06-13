--[[

	A player command to send a name of a found cheater to the log

]]
local events = {}
local usage = "#cheater <cn>|\"<name>\""

local function init()
end


local function unload()
end

local function run(cn,cheat)

	if not cheat then

		return false, usage
	end

	if not server.valid_cn(cheat) then

		cheat = server.name_to_cn_list_matches(cn,cheat)

		if not cheat then

			return

		end
	end

	cheat = tonumber(cheat)
	cn = tonumber(cn)
	
	if cheat == cn then

		return false, "You can't report yourself"
	end

	if not server.player_vars(cn).cheater then

		server.player_vars(cn).cheater = {}
	end

	local cheat_report = (server.player_vars(cn).cheater[cheat] or 0) + 1
	server.player_vars(cn).cheater[cheat] = cheat_report

	if cheat_report > 4 then

		server.player_msg(cn,orange("Don't spam with the #cheater command or you will be ignored."))
	end

	if cheat_report < 8 then

		if server.player_connection_time(cheat) > 10 then

			server.player_msg(cn,"Thank you for your report, hopefully an admin will check this out very soon.")
			server.log("CHEATER: " .. server.player_name(cheat) .. "(" .. cheat .. ") was reported by " .. server.player_name(cn) .. "(" .. cn .. ")")
        end
    end

end

return {init = init,run = run,unload = unload}
