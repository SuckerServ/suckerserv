require "geoip"

local BANNER_DELAY = 3000
local show_country_message = server.show_country_message == 1

local function send_connect_message(tcn)

    if server.player_is_spy(tcn) or server.is_bot(tcn) then return end
    if true == server.player_vars(tcn).shown_connect then return end

            local country = geoip.ip_to_country(server.player_ip(tcn))
            local city = geoip.ip_to_city(server.player_ip(tcn))
            if not city or #city < 1 then city = "Unknown" end


            if not show_country_message or #country < 1 then
                country = "Unknown"
            end

            if server.player_ranking then
                player_ranking = server.player_ranking(server.player_name(tcn))
            end

            if not player_ranking then
                player_ranking = "Unknown"
            end

            local name = server.player_displayname(tcn)

            if server.player_vars(tcn).reserved_name then
                name = green(server.player_name(tcn)) .. " (" .. tcn .. "): " .. yellow(server.player_vars(tcn).reserved_name)
            elseif server.player_vars(tcn).stats_auth_name then
                name = green(server.player_name(tcn)) .. " (" .. tcn .. "): " .. yellow(server.player_vars(tcn).stats_auth_name)
            end

            if server.player_priv_code(tcn) > server.PRIV_NONE then
                name = name .. "(" .. server.player_priv(tcn) .. ")"
            end

            local normal_message = string.format(server.client_connect_message, name, city, country, player_ranking)
            local admin_message = string.format(server.client_connect_admin_message, normal_message, server.player_ip(tcn))

            for _, cn in ipairs(server.clients()) do

                local message = normal_message

                if server.player_priv_code(cn) == server.PRIV_ADMIN then
                    message = admin_message
                end

                server.player_msg(cn, message)
            end
            server.player_vars(tcn).shown_connect = true

end

local function sendServerBanner(cn)

    server.player_vars(cn).maploaded = true
    
    if server.player_vars(cn).shown_banner then return end
    if true == server.player_vars(cn).shown_connect then return end

    server.interval(1000, function()

        if not server.gamepause then
            local sid = server.player_sessionid(cn)

                server.sleep(BANNER_DELAY, function()

                -- cancel if not the same player from 1 second ago
                if sid ~= server.player_sessionid(cn) then return -1 end
                if true == server.player_vars(cn).shown_connect then return -1 end

                server.player_msg(cn, server.motd)
                server.player_msg(cn, server.connect_info_message)

                server.player_vars(cn).shown_banner = true

                send_connect_message(cn)
            end)
            return -1
        end
    end)

end

server.event_handler("maploaded", sendServerBanner)
server.event_handler("mapchange", function()
    for p in server.gclients() do
        p:vars().maploaded = nil
    end
end)
