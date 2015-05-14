require "http.client"
require "Json"

local ADMIN_NAME = "global"
local UPDATE_INTERVAL = 1000*60*60*0.5

local GBAN_MIRRORS =
{
--    { url = "https://raw.githubusercontent.com/pisto/ASkidban/master/compiled/ipv4", format = "raw", name = "ASkidban", reason = "proxy" },
}

local bans = {}

local function update()
    for _, mirror in pairs(GBAN_MIRRORS) do
        http.client.get(mirror.url, function(body, status)
            if not body then
                server.log_error(string.format("Failed to download the global ban list from %s", mirror.url))
                return
            end

            local data = {}
            if mirror.format == "json" then
                data = Json.Decode(body)
            elseif mirror.format == "raw" then
                data = {}
                for l in body:gmatch("([^\n]+)\n?") do
                    table.insert(data, { address = l, reason = mirror.reason, admin = mirror.name })
                end
            end

            local updated_bans = {}
            local change = false

            if not bans[mirror.name] then
                bans[mirror.name] = {}
            end

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

                bans[mirror.name][ban.address] = nil
            end

            local subtracted_bans = bans[mirror.name]
            for address in pairs(subtracted_bans) do
                server.unban(address)
                change = true
            end

            bans[mirror.name] = updated_bans

            if change then
                server.log_status("Updated global bans")
            end
        end)
    end
end

server.interval(UPDATE_INTERVAL, update)
update()

server.update_global_bans = update
