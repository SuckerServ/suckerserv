-- Setmaster
-- (c) 2010 Thomas

local failed = { }
local FAILED_LIMIT = 5

local function update_display_open()
    server.display_open = server.allow_setmaster == 1
end

local function setmaster(cn, hash, set)

    if not set then
         server.unsetpriv(cn)
        return -1
    end

    local no_hash = hash == ""
    local no_master = server.master == -1
    
    if no_hash and no_master and server.allow_setmaster == 1 then
        server.setmaster(cn)
        return -1
    end
    
    if no_hash or failed[cn] == FAILED_LIMIT then
        return -1 
    end
    
    if server.hashpassword(cn, server.admin_password) == hash then
        if no_master then
            server.setadmin(cn) 
        else
            server.set_invisible_admin(cn)
        end
    elseif server.hashpassword(cn, server.master_password) == hash then
        if no_master then
            server.setmaster(cn) 
        else
            server.set_invisible_master(cn)
        end
    else
        server.log(string.format("Player: %s(%i) IP: %s -- failed setmaster login!", server.player_name(cn), cn, server.player_ip(cn)))
        
        failed[cn] = (failed[cn] or 0) + 1
        
        if failed[cn] == FAILED_LIMIT then
            server.player_msg(cn, server.setmaster_refused_message)
        end
    end
    
    return -1
end

server.event_handler("disconnect", function(cn)
    failed[cn] = nil
end)

server.event_handler("connecting", function(cn, host, name, hash, reserved_slot)
	if reserved_slot then return end
    if hash ~= "" then
	    failed[cn] = 0
		setmaster(cn, hash, 1)
    end
end)

server.event_handler("setmaster", function(cn, hash, set)
    setmaster(cn, hash, set)
end)

server.event_handler("mapchange", function()
    update_display_open()
end)

update_display_open()

