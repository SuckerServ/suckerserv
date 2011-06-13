require "http.client"
require "Json"

local URL = "http://hopmod.googlecode.com/svn/data/bans.json"
local ADMIN = "global"
local UPDATE = 1000*60*60*24

local bans = {}

local function update()
    http.client.get(URL, function(body, status)
        
        if not body then
            server.log_error("Failed to download the global ban list: " .. status)
        end
        
        local data = Json.Decode(body)
        
        local updated_bans = {}
        local change = false
        
        for _, ban in pairs(data) do
            
            updated_bans[ban.address] = true
            
            if not server.is_banned(ban.address) then
                server.ban(ban.address, -1, ADMIN)
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

server.interval(UPDATE, update)
update()

server.update_global_bans = update

