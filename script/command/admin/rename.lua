--[[

    	A command to rename a player
	By piernov — <piernov@piernov.org>

]]

local usage = "#rename <cn> <newname>"

return function(cn,player_cn,new_name)
    if not player_cn or not new_name then
        return false, "#rename <cn> <newname>"
    end
    if not server.valid_cn(player_cn) then
        return false, "Invalid CN"
    end

    server.player_rename(player_cn, new_name, true)
    server.player_msg(player_cn, string.format(server.player_renamed_message, new_name, server.player_displayname(cn)))
    server.admin_log(string.format("RENAME: Player renamed to %s by %s", new_name, server.player_displayname(cn))) 
end
