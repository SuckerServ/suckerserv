--[[
	A player command to unban a banned player
]]

local usage = "#unban \"<ip>\""

return function(cn, ip)
    if not ip then
        return false, usage
    end
    local res = check_ip(ip)
    if #res == 1 then
        return false, string.format("Invalid IP (%s)", res[1])
    end
    if server.ip_vars(ip).ban_time then
        server.unban(ip)
        server.msg("unban_message", { name = server.player_displayname(cn), ip = ip })
        admin_log(string.format("UNBAN: %s unbaned IP: %s", server.player_displayname(cn), ip))
    else
        server.player_msg(cn, "no_matching_ban", { ip = ip })
    end
end
