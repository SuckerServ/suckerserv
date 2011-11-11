-- (C) 2010 by Thomas
-- Player command to see how much traffic the server caused

return function(cn)
    local t_in, t_out, t_sum

    local function set_traffic()
        t_in = math.round(server.rx_bytes/1024,2)
        t_out = math.round(server.tx_bytes/1024,2)
        t_sum = t_in + t_out
    end

    set_traffic()

    server.player_msg(cn, string.format(
    "Used Traffic => IN: %dKB OUT: %dKB SUM: %dKB", 
        t_in, 
        t_out,
        t_sum              
    ))

    local function print_speed()
        if not server.valid_cn(cn) then return end
        local tmp_in  = math.round(((server.rx_bytes/1024) - t_in),2)
        local tmp_out = math.round(((server.tx_bytes/1024) - t_out),2)
        local tmp_sum = tmp_in + tmp_out
        server.player_msg(cn, string.format(
        "Traffic Speed => IN: %dKB/s OUT: %dKB/s SUM: %dKB/s", 
            tmp_in, 
            tmp_out,
            tmp_sum
        ))   
        
        set_traffic()
    end

    server.sleep(1000, print_speed)
    server.sleep(2000, print_speed)
    server.sleep(3000, print_speed)
end