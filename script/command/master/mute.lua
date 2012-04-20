--[[

	A player command to mute a player
    
]]

local usage = "#mute <cn>|\"<name>\" [<time>]"

return function(cn,tcn,time)

    if not server.mute then
        return false, "mute module not loaded"
    end

    if not tcn then
        return false, usage
    end

    if not server.valid_cn(tcn) then
        tcn = server.name_to_cn_list_matches(cn,tcn)
	    if not tcn then return end
    end

    server.mute(tcn,time)

    server.player_msg(cn, string.format(server.player_mute_admin_message, tcn))
end
