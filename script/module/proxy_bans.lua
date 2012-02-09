require "geoip"

server.event_handler("connecting", function(cn, ip, name)

    if geoip.ip_to_country_code(ip) == "A1" then
        server.log("Server disconnected " .. name .. " cause of using proxy.")
        return -1
    end
end)
