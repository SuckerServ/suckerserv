--[[

	A player command to mute a player
    
]]

return function(cn,tcn,time)

    if not server.mute then
        return false, "mute module not loaded"
    end

    if not tcn then
	return false, "#mute <cn>|\"<name>\" [<time>]"
    end

    if not server.valid_cn(tcn) then

        tcn = server.name_to_cn_list_matches(cn,tcn)

	    if not tcn then
		return
	    end
    end

    server.mute(tcn,time)
end
