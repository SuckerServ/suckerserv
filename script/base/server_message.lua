
do
    local triggerCallEvent = server.create_event_signal("admin-message")
    
    function server.console(admin, msg)
        server.msg("remoteadmin", { admin = admin, msg = msg })
        triggerCallEvent(admin, msg)
    end
end

function server.info_msg(text)
    server.msg("server_info" { msg = text })
end
