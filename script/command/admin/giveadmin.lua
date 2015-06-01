--[[

	A player command to raise player's privilege to admin

]]

local usage = "#giveadmin <cn>"

return function(cn, target)
    if not target then
        return false, usage
    end

    if not server.valid_cn(target) then
        return false, "CN is not valid"
    end

    server.unsetmaster()
    server.player_msg(target, "giveadmin", {name = server.player_displayname(cn) })
    server.admin_log(string.format("GIVEADMIN: %s gave admin to %s", server.player_displayname(cn), server.player_displayname(target)))
    server.setadmin(target)
end
