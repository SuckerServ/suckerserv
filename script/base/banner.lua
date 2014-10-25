local BANNER_DELAY = 2000

local function send_connect_message(cn)

    if server.player_is_spy(cn) or server.is_bot(cn) then return end

--[[
    if server.display_city_on_connect == 1 then
        local city = server.mmdatabase:lookup_ip(server.player_ip(cn), "city", "names", "en")
        if not city or #city < 1 then city = "Unknown" end
    end

    if server.display_rank_on_connect then
        if server.player_ranking then
            player_ranking = server.player_ranking(server.player_name(cn))
        end
        if not player_ranking then
            player_ranking = "Unknown"
        end
    end
]]--

    local priv = ""

    if server.player_priv_code(cn) > server.PRIV_NONE then
        priv = server.player_priv(cn)
    end

    for _, dest_cn in ipairs(server.clients()) do
        local country = ""

        if server.display_country_on_connect == 1 then
            country = server.mmdatabase:lookup_ip(server.player_ip(cn), "country", "names", server.player_lang(dest_cn))

            if not country or #country < 1 then
                country = server.mmdatabase:lookup_ip(server.player_ip(cn), "country", "names", messages.languages["default"])
            end

            if not country or #country < 1 then
                country = "Unknown"
            end
        end

        local normal_message = server.parse_message(dest_cn, "client_connect", {country = country, name = server.player_name(cn), cn = cn, priv = priv})

        if priv == "" then
            normal_message = normal_message:sub(1, -5)
        end

        if server.player_priv_code(dest_cn) == server.PRIV_ADMIN then
            local admin_message = server.parse_message(dest_cn, "client_connect_admin", { ip = server.player_ip(cn) })
            if priv == "" then
                admin_message = normal_message .. "(" .. admin_message .. ")"
            else
                admin_message = normal_message:sub(1, -2) .. " ; " .. admin_message .. ")"
            end
            server.player_msg(dest_cn, admin_message)
        else
            server.player_msg(dest_cn, normal_message)
        end
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
        server.player_msg(cn, "connect_info")

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
