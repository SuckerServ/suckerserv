--[[

	A player command to ban a player

]]


return function(cn,arg1,arg2,arg3,arg4,arg5)

	local all = 0
	local lcn = nil
	local time = 3600
	local reason = ""

	if not arg1 then

		return false, "#ban <cn> [<time> [m|h]] [\"<reason>\"]"

	elseif arg1 == "all" then

		all = 1

		if not arg2 then

			return false, "CN is missing"

		elseif server.valid_cn(arg2) then

			lcn = arg2

		else

			return false, "CN is not valid"
		end

		if arg3 then

			if arg3 > "0" and arg3 < "13670" then

				time = tonumber(arg3)

				if arg4 then

					if arg4 == "h" then

						time = time * 3600

					elseif arg4 == "m" then

						time = time * 60

					elseif arg5 then

						return false, "time.unit is not valid"

					else

						reason = arg4
					end

					if arg5 then

						reason = arg5
					end
				end
			else

				reason = arg3
			end
		end

	elseif server.valid_cn(arg1) then

		lcn = arg1

		if arg2 then

			local larg2 = tonumber(arg2)

			if larg2 > 0 and larg2 < 13670 then

				time = larg2

				if arg3 then

					if arg3 == "h" then

						time = time * 3600

					elseif arg3 == "m" then

						time = time * 60

					elseif arg4 then

						return false, "time.unit is not valid"

					else

						reason = arg3
					end

					if arg4 then

						reason = arg4
					end
				end
			else

				reason = arg2
			end
		end

	else

		return false, "CN is not valid"
	end

	if lcn == cn then

		return false, "Don't kick yourself"
	else

		if all == 1 then

			server.kick_bannedip_group = true
		end

		server.kick(lcn,time,server.player_displayname(cn),reason)
		server.admin_log(string.format("BAN: Player %s has been banned for %s. Reason %s", server.player_displayname(cn), time, reason))

		if all == 1 then

			server.kick_bannedip_group = false
		end
	end

end
