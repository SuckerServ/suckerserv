server.event_handler("shot",function(cn,gun,hits)

	if cn < 128 then

		local gametime = server.gamemillis
		local reload_limit = nil

		if server.player_vars(cn).cdr_last_weapon and not (gun == server.player_vars(cn).cdr_last_weapon) then

			server.player_vars(cn).cdr_last_shot = nil

		end

		if gun == 2 then

			reload_limit = 90

		elseif gun == 0 then

			reload_limit = 240

		elseif gun == 6 then

			reload_limit = 490

		elseif gun == 5 then

			reload_limit = 490

		elseif gun == 3 then

			reload_limit = 795

		elseif gun == 1 then

			reload_limit = 1395

		elseif gun == 4 then

			reload_limit = 1495

		end

		if server.player_vars(cn).cdr_last_shot and ((gametime - server.player_vars(cn).cdr_last_shot) < reload_limit) then
		    
            local message = string.gsub(
                "WARNING: $name ($cn)'s weapon reload time is too low ($reload_time should be $reload_limit). [pj: $pj | ping: $ping | weapon: $weapon | ip: $ip]",
                "%$([%w_]+)", {
                name = server.player_name(cn),
                cn = cn,
                reload_time = (gametime - server.player_vars(cn).cdr_last_shot),
                reload_limit = reload_limit,
                pj = server.player_lag(cn),
                ping = server.player_ping(cn),
                weapon = gun,
                ip = server.player_ip(cn)
            })
            
            server.log(message)
		end

		server.player_vars(cn).cdr_last_shot = gametime
		server.player_vars(cn).cdr_last_weapon = gun

	end

end)

