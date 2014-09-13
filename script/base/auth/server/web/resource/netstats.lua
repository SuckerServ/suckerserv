require "http_server"
require "Json"

local tx_bytes_sample = server.tx_bytes
local tx_bytes_rate = 0

local rx_bytes_sample = server.rx_bytes
local rx_bytes_rate = 0

local min_ping = 0
local avg_ping = 0
local max_ping = 0

server.interval(1000, function()
    
    local latest_tx_bytes = server.tx_bytes
    local latest_rx_bytes = server.rx_bytes
    local latest_tx_packets = server.tx_packets
    local latest_rx_packets = server.rx_packets
    
    tx_bytes_rate = latest_tx_bytes - tx_bytes_sample
    tx_bytes_sample = latest_tx_bytes
    
    rx_bytes_rate = latest_rx_bytes - rx_bytes_sample
    rx_bytes_sample = latest_rx_bytes
    
--    min_ping = nil
--    avg_ping = nil
--    max_ping = nil
    
--    for player in server.gplayers() do
--    
--        local ping = player:ping()
--        
--        if not min_ping or ping < min_ping then
--            min_ping = ping
--        end
--        
--        if not max_ping or ping > max_ping then
--            max_ping = ping
--        end
--        
--        avg_ping = (avg_ping and avg_ping/2 + ping/2) or ping
--    end
end)

http_server_root["netstats"] = http_server.resource({
    get = function(request)
        local output = {
            downloaded = rx_bytes_sample,
            download_rate = rx_bytes_rate,
            uploaded = tx_bytes_sample,
            upload_rate = tx_bytes_rate,
--            min_ping = min_ping,
--            avg_ping = avg_ping,
--            max_ping = max_ping
        }
        http_response.send_json(request, output)
    end
})

