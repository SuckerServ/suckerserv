
auth.directory.server{
    id = "MASTER",
    hostname = "sauerbraten.org",
    port = 28787
}

auth.directory.domain{
    server = "MASTER",
    id = ""
}

local banned = list_to_set(table_unique(server.parse_list(server.masterauth_banned)))

local current_master_authed = false
local auth_name
local auth_player_cn
local message

local function broadcast() return function(text) server.msg(text) end end
local function unicast(cn) return function(text) server.player_msg(cn, text) end end

local function send_info_message(sender)
    
    if not message then
       local cn = auth_player_cn
       local name = server.player_displayname(cn)
       message = string.format(server.claimmaster_message, name, auth_name)
    end
    
    sender(message)
end

auth.listener("", function(cn, user_id, domain, status)

    if status ~= auth.request_status.SUCCESS then return end
    
    local no_admin = server.master == -1 or server.player_priv_code(server.master) ~= server.PRIV_ADMIN
    
    if server.player_priv_code(cn) == 0 and no_admin then
        
        local name = server.player_name(cn)
        
        if banned[user_id] then
            server.player_msg(cn, server.player_auth_banned_message)
            return
        end
        
        if server.setmaster(cn) then
        
            current_master_authed = true
            message = nil
            auth_name = user_id
            auth_player_cn = cn
            
            send_info_message(broadcast())
            
            server.log(string.format("%s(%i) claimed master as '%s'", name, cn, user_id))
        else
            server.player_msg(cn, server.player_master_disabled_message)
        end
    end
end)

server.event_handler("connect", function(cn)
    if current_master_authed then
        send_info_message(unicast(cn))
    end
end)

server.event_handler("privilege", function(cn, old_priv, new_priv)
    if current_master_authed and cn == auth_player_cn and new_priv ~= server.PRIV_MASTER then
        current_master_authed = false
    end
end)

