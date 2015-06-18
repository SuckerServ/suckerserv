
auth.directory.server{
    id = "MASTER",
    hostname = "master.sauerbraten.org",
    port = 28787
}

auth.directory.domain{
    server = "MASTER",
    id = ""
}

server.current_master_global_authed = nil
server.current_master_global_auth_user = nil

local banned = list_to_set(table_unique(server.masterauth_banned))

auth.listener("", function(cn, user_id, domain, status, kick_cn)

    if status ~= auth.request_status.SUCCESS then return end

    local priv_code = server.player_priv_code(cn)

    if banned[user_id] then
        if priv_code == 0 then
            server.player_msg(cn, red("You have been banned from using /auth on this server"))
        end
        return
    end

    local name = server.player_name(cn)

    if kick_cn ~= nil and kick_cn >= 0 then
        if not server.valid_cn(kick_cn) then
            server.player_msg(cn, red("Invalid player cn given"))
            return
        end

        if server.player_priv_code(kick_cn) > server.PRIV_AUTH then
            server.player_msg(cn, red("Kick player request denied"))
            return
        end

        local vitcim = server.player_displayname(kick_cn)
        server.kick(kick_cn, nil, name, string.format("%s as '%s' kicked %s", server.player_displayname(cn), magenta(user_id), vitcim))

        return
    end

    if priv_code == 0 then
        if server.setauth(cn) then
            server.current_master_global_authed = cn
            server.current_master_global_auth_user = user_id
            server.msg("claimmaster", { name = server.player_displayname(cn), uid = user_id })
            server.log(string.format("%s(%i) claimed master as '%s'", name, cn, user_id))
            server.admin_log(string.format("%s(%i) claimed master as '%s'", name, cn, user_id))
        end
    end
end)

server.event_handler("connect", function(cn)
    if server.current_master_authed then
        server.player_msg(cn, "claimmaster", { name = server.player_displayname(server.current_master_authed), uid = server.current_master_global_auth_user })
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
