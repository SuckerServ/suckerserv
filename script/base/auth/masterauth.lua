
auth.directory.server{
    id = "MASTER",
    hostname = "sauerbraten.org",
    port = 28787
}

auth.directory.domain{
    server = "MASTER",
    id = ""
}

server.current_master_global_authed = nil
server.current_master_global_auth_user = nil
local banned = list_to_set(table_unique(server.parse_list(server.masterauth_banned)))

auth.listener("", function(cn, user_id, domain, status)
    
    if status ~= auth.request_status.SUCCESS then return end
    
    if server.player_priv_code(cn) == 0 then
        
        local name = server.player_name(cn)
        
        if banned[user_id] then
            server.player_msg(cn, server.player_auth_banned_message)
            return
        end
        
        if server.setmaster(cn) then
            server.current_master_global_authed = cn
            server.current_master_global_auth_user = user_id
            server.msg(string.format(server.claimmaster_message, server.player_displayname(cn), user_id))
            server.log(string.format("%s(%i) claimed master as '%s'", name, cn, user_id))
            server.admin_log(string.format("%s(%i) claimed master as '%s'", name, cn, user_id))
        end
    end
end)

server.event_handler("connect", function(cn)
    if server.current_master_authed then
        server.player_msg(cn, string.format(server.claimmaster_message, server.player_displayname(cn), server.current_master_global_auth_user))
    end
end)
server.event_handler("disconnect", function(cn)
    if server.current_master_global_authed == cn then
        server.current_master_global_authed = nil
    end
end)
server.event_handler("privilege", function(cn, old_priv, new_priv)
    if server.current_master_global_authed == cn and new_priv ~= server.PRIV_MASTER then
        server.current_master_global_authed = nil
    end
end)