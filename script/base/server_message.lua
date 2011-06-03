
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
