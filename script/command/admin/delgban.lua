--[[
    2011 - Thomas
--]]

local function mark_gban_as_deleted(ip)
    for ipmask, vars in pairs(server.ip_vars()) do
        if ip_inrange(ip, ipmask) then
            server.set_ip_var(ipmask, "ignore_gban", true)
            return ipmask
        end
    end
    
    error("unable to find gban")
end

return function(cn, ipmask) 
    if not ipmask then 
        return false, "bla"
    end
    
    local ban = server.ip_vars(ipmask)
    
    if not (ban.is_gban or false) then
        return false, string.format("%s isn't a gban", ipmask)
    end

    server.player_msg(cn, string.format("marked gban (%s) as deleted", mark_gban_as_deleted(ipmask)))
end
