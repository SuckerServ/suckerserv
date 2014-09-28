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
    
    if no_hash and server.allow_setmaster == 1 then
        if not server.hasmaster() then
            server.setmaster(cn)
        else
            server.player_msg(cn, "master_already")
        end
        return -1
    end
    
    if no_hash or failed[cn] == FAILED_LIMIT then
        return -1 
    end

    local is_spy = server.hashpassword(cn, server.admin_password .. "/spy") == hash
    local success = is_spy
    if not success then
        success = server.hashpassword(cn, server.admin_password) == hash
    end

    if server.admin_password ~= "" and success then
        if is_spy then server.setspy(cn, true)
        else
            server.setadmin(cn) 
        end
    elseif server.master_password ~= "" and server.hashpassword(cn, server.master_password) == hash then
        server.setmaster(cn) 
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

server.event_handler("setmaster", setmaster)
server.event_handler("mapchange", update_display_open)

update_display_open()
