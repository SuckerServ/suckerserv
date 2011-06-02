-- Module to Prevent Players from using offensive names
-- Add names to conf/name_bans.txt!
-- (c) 2010 Thomas


local fhandle = io.open("conf/name_bans.txt")
local banned_names = string.split(fhandle:read("*a"), "[^ ]+")
fhandle:close()

local function is_banned_name(name)
	local name = string.upper(name)
	for _, ban_name in ipairs(banned_names) do
		local ban_name = string.upper(ban_name)
		if string.find(name, ban_name) then
			return true
		end
	end
	return false
end

server.event_handler("connecting", function(cn, ip, name)
	if is_banned_name(name) then
		return -1
	end
end)

server.event_handler("rename", function(cn, old, new)
	if is_banned_name(new) then
		server.player_rename(cn, "unnamed", true)
	end
end)
