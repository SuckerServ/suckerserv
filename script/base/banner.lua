require "mmdb"

local BANNER_DELAY = 2000

local function send_connect_message(cn)

    if server.player_is_spy(cn) or server.is_bot(cn) then return end

    local country = ""

    if server.display_country_on_connect == 1 then
        country = mmdb.lookup_ip(server.player_ip(cn), "country", "names", "en")
        if not country or #country < 1 then country = "Unknown" end
    end

    if server.display_city_on_connect == 1 then
        local city = mmdb.lookup_ip(server.player_ip(cn), "city", "names", "en")
        if not city or #city < 1 then city = "Unknown" end
    end

    if server.player_ranking then
        player_ranking = server.player_ranking(server.player_name(cn))
    end
    if not player_ranking then
        player_ranking = "Unknown"
    end

    local name = server.player_displayname(cn)

    if server.player_vars(cn).reserved_name then
        name = green(server.player_name(cn)) .. " (" .. cn .. "): " .. yellow(server.player_vars(cn).reserved_name)
    elseif server.player_vars(cn).stats_auth_name then
        name = green(server.player_name(cn)) .. " (" .. cn .. "): " .. yellow(server.player_vars(cn).stats_auth_name)
    end

    if server.player_priv_code(cn) > server.PRIV_NONE then
        name = name .. "(" .. server.player_priv(cn) .. ")"
    end

    local normal_message = string.format(server.client_connect_message, name, city, country, player_ranking)
    local admin_message = string.format(server.client_connect_admin_message, normal_message, server.player_ip(cn))

    for _, cn in ipairs(server.clients()) do

        local message = normal_message

        if server.player_priv_code(cn) == server.PRIV_ADMIN then
            message = admin_message
        end

        server.player_msg(cn, message)
    end
end

local function sendServerBanner(cn)

    server.player_vars(cn).maploaded = true
    
    if server.player_vars(cn).shown_banner then return end
    
    local sid = server.player_sessionid(cn)
    
    server.sleep(BANNER_DELAY, function()

        -- cancel if not the same player from 1 second ago
        if sid ~= server.player_sessionid(cn) then return end
        
        server.player_msg(cn, server.motd)
        server.player_msg(cn, server.connect_info_message)

        server.player_vars(cn).shown_banner = true

        send_connect_message(cn)
    end)
end

server.event_handler("maploaded", sendServerBanner)
server.event_handler("mapchange", function()
    for p in server.gclients() do
        p:vars().maploaded = nil
    end
end)
