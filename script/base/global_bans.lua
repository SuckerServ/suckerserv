local http = require "http.client"
local Json = require "Json"

local ADMIN_NAME = "global"
local UPDATE_INTERVAL = 1000*60*60*0.5

local GBAN_MIRRORS =
{
--    { url = "https://raw.githubusercontent.com/pisto/ASkidban/master/compiled/ipv4", format = "raw", name = "ASkidban", reason = "proxy" }
}

local MIRROR = 1

local bans = {}

local function update()
    http.client.get(GBAN_MIRRORS[MIRROR].url, function(body, status)
        if not body then
            server.log_error(string.format("Failed to download the global ban list from %s", GBAN_MIRRORS[MIRROR].url))

            if MIRROR < #GBAN_MIRRORS then
                MIRROR = MIRROR + 1
                update()
            end

            return
        end

        local data
        if GBAN_MIRRORS[MIRROR].format == "json" then
            data = Json.Decode(body)
        elseif GBAN_MIRRORS[MIRROR].format == "raw" then
            data = {}
            for l in body:gmatch("([^\n]+)\n?") do
                table.insert(data, { address = l, reason = GBAN_MIRRORS[MIRROR].reason, admin = GBAN_MIRRORS[MIRROR].name })
            end
        end

        local updated_bans = {}
        local change = false

        for _, ban in pairs(data) do

            updated_bans[ban.address] = true

            if not server.is_banned(ban.address) then
                local bantime = tonumber(ban.expire) or -1
                if bantime ~= -1 then bantime = bantime - os.time(os.date("!*t")) end
                local reason = ban.reason or "global ban"
                local admin = ban.admin or ADMIN_NAME
                server.ban(ban.address, bantime, admin, reason, nil, true)
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
