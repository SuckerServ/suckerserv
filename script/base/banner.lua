require "geoip"

local BANNER_DELAY = 1000
local show_country_message = server.show_country_message == 1

local function sendServerBanner(cn)
    
    if server.player_vars(cn).shown_banner then return end
    
    local sid = server.player_sessionid(cn)
    
    server.sleep(BANNER_DELAY, function()

        -- cancel if not the same player from 1 second ago
        if sid ~= server.player_sessionid(cn) then return end
        
        server.player_msg(cn, red(">>> ") .. server.motd)
		server.player_msg(cn, server.connect_info_message)

        server.player_vars(cn).shown_banner = true
    end)
end

local function onConnect(cn)

    local country = geoip.ip_to_country(server.player_ip(cn))
    
    if show_country_message and #country > 0 then

        local normal_message = string.format(server.client_connect_message, blue(server.player_displayname(cn)), blue(country))
        local admin_message = string.format(server.client_connect_admin_message, normal_message, blue(server.player_ip(cn)))
        
        for _, cn in ipairs(server.clients()) do
            
            local message = normal_message

            if server.player_priv_code(cn) == server.PRIV_ADMIN then
                message = admin_message
            end
            
            server.player_msg(cn, message)
        end
    end
end

server.event_handler("connect",onConnect)
server.event_handler("maploaded", sendServerBanner)
server.event_handler("disconnect", function(cn) server.player_vars(cn).shown_banner = nil end)
