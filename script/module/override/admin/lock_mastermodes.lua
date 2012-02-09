local mm = {
    open	= server.disallow_mastermode_open_for_admins == 1,
    veto	= server.disallow_mastermode_veto_for_admins == 1,
    locked	= server.disallow_mastermode_locked_for_admins == 1,
    private	= server.disallow_mastermode_private_for_admins == 1
}

server.event_handler("setmastermode_request", function(cn, old, new)

    if mm[new]
    then
    	server.player_msg(cn, red("Mastermode " .. new .. " is disabled."))
    	return -1
    end
end)
