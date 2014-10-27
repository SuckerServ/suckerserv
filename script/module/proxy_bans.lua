server.event_handler("connecting", function(cn, ip, name)

    if  server.mmdatabase:lookup_ip(ip, "traits", "is_anonymous_proxy") == "true" then
        server.log("Server disconnected " .. name .. " cause of using proxy.")
        return -1
    end
end)
