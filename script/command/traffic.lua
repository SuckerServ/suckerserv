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

	server.player_msg(cn, 
	      string.format("%sUsed Traffic => IN: %s%dKB %sOUT: %s%dKB %sSUM: %s%dKB", 
		    white(), 
		    orange(), 
		    t_in, 
		    white(),
		    orange(),
		    t_out,
		    white(),
		    orange(),
		    t_sum 		   
	      )
	)

	local function print_speed()
	    if not server.valid_cn(cn) then return end
	    local tmp_in  = math.round(((server.rx_bytes/1024) - t_in),2)
	    local tmp_out = math.round(((server.tx_bytes/1024) - t_out),2)
	    local tmp_sum = tmp_in + tmp_out
	    server.player_msg(cn, 
		string.format("%sTraffic Speed => IN: %s%dKB/s %sOUT: %s%dKB/s %sSUM: %s%dKB/s", 
		    white(), 
		    orange(), 
		    tmp_in, 
		    white(),
		    orange(),
		    tmp_out,
		    white(),
		    orange(),
		    tmp_sum
		)   
	    )
	    set_traffic()
	end

	server.sleep(1000, print_speed)
	server.sleep(2000, print_speed)
	server.sleep(3000, print_speed)
end