require "geoip"
require "crypto"


local banned_ips = {
    ["26201aedf59f572eaf388de4b93e33e1a6992249a4b81aff"]	= true,
    ["a8f97d01e422f09a60142f296e7ee67f07f55e5b2a55c9de"]	= true
}


server.event_handler("connecting", function(cn, ip, name)

    if geoip.ip_to_country_code(ip) == "A1"
	or banned_ips[crypto.tigersum(tostring(ip))]
    then
	server.log("Server disconnected " .. name .. " cause of using proxy.")
	return -1
    end
end)
