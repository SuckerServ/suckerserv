
do
    local triggerCallEvent = server.create_event_signal("admin-message")
    
    function server.console(admin, msg)
        server.msg(string.format(server.remoteadmin_message, admin, msg))
        triggerCallEvent(admin, msg)
    end
end

function server.info_msg(text)
    server.msg(string.format(server.info_message, text))
end

function server.msg_admin(msg)
    for client in server.gclients() do
        if client:priv_code() == server.PRIV_ADMIN then
            client:msg(magenta(msg))
        end
    end
end

function server.msg_master(msg)
    for client in server.gclients() do
        if client:priv_code() >= server.PRIV_MASTER then
            client:msg(magenta(msg))
        end
    end
end

