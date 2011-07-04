--[[

	A player command to unban a banned player

]]

local usage = "#unban <ip>"

return function(cn, ip)
    if not net.ipmask(ip) then return; end
    if server.ip_vars(ip).ban_time then
        server.unban(ip)
        server.player_msg(cn, string.format(server.unban_message, ip))
    else
        server.player_msg(cn, string.format(server.no_matching_ban_message, ip))
    end
end
