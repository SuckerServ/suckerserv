
do
    local triggerCallEvent = server.create_event_signal("admin-message")
    
    function server.console(admin, msg)
        server.msg(string.format("Remote Admin%s: %s", magenta("("..admin..")"), green(msg)))
        triggerCallEvent(admin, msg)
    end
end

function server.info_msg(text)
    server.msg(string.format("Server Info: %s", green(text)))
end

