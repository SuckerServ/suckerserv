local http = require "http.client"
local Json = require "Json"
--[[
    You can checkout the bans in a readable form here:

        http://sauer.nomorecheating.org/hopmod/view_bans.php

--]]

local ADMIN_NAME = "global"
local UPDATE_INTERVAL = 1000*60*60*0.5

local GBAN_MIRRORS = server.ban_lists

local MIRROR = 1

local bans = {}

local function update()
    http.client.get(GBAN_MIRRORS[MIRROR], function(body, status)

        if not body then
            server.log_error(string.format("Failed to download the global ban list from %s", GBAN_MIRRORS[MIRROR]))

            if MIRROR < #GBAN_MIRRORS then
                MIRROR = MIRROR + 1
                update()
            end

            return
        end

        local data = Json.Decode(body)

        local updated_bans = {}
        local change = false

        for _, ban in pairs(data) do

            updated_bans[ban.address] = true

            if not server.is_banned(ban.address) then
                local bantime = tonumber(ban.expire) or -1
                if bantime ~= -1 then bantime = bantime - os.time(os.date("!*t")) end
                local reason = ban.reason or "global ban"
                server.ban(ban.address, bantime, ADMIN_NAME, reason, nil, true)
                change = true
            end

            bans[ban.address] = nil
        end

        local subtracted_bans = bans
        for address in pairs(subtracted_bans) do
            server.unban(address)
            change = true
        end

        bans = updated_bans

        if change then
            server.log_status("Updated global bans")
        end
    end)
end

server.interval(UPDATE_INTERVAL, update)
update()

server.update_global_bans = update
