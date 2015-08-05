--[[
    set player status to invmaster at connect
]]


local domains = table_unique(server.parse_list(server["auto_invmaster_domains"])) or table_unique(server.parse_list(server["invmaster_domains"]))

if not domains then
    server.log_error("invmaster: no domains set")
    return
end

local trigger_event = server.create_event_signal("invmaster-module")

server.event_handler("connect", function(cn)
    
    if server.is_bot(cn) then
        return
    end
    
    local sid = server.player_sessionid(cn)
    
    server.sleep(2000, function()
        
        if server.valid_cn(cn) and (sid == server.player_sessionid(cn)) then
            if server.player_priv_code(cn) < server.PRIV_MASTER then
                
                for _, domain in pairs(domains) do
                    auth.send_request(cn, domain, function(cn, user_id, domain, status)
                        
                        if not (sid == server.player_sessionid(cn)) or not (status == auth.request_status.SUCCESS) or (server.player_priv_code(cn) >= server.PRIV_MASTER) then
                            return
                        end
                        
                        server.set_invisible_master(cn)
                        
                        server.log(user_id .. " playing as " .. server.player_name(cn) .. "(" .. cn .. ") used auth to claim invisible master.")
                        server.admin_log(user_id .. " playing as " .. server.player_name(cn) .. "(" .. cn .. ") used auth to claim invisible master.")

                        trigger_event(cn, user_id)
                    end)
                end
            end
        end
    end)
end)
