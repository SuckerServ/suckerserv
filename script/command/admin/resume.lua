
return function(cn, countdown)

	if not countdown then
		server.pausegame(false)
		server.msg(string.format(server.resume_message, server.player_name(cn)))
	else
		local cdown = tonumber(countdown)

		if cdown < 1 then
			server.pausegame(false)
		else
			cdown = round(cdown, 0)

			server.interval(1000, function()

				if cdown == 0 then

					server.pausegame(false)
					server.msg(string.format(server.resume_message, server.player_name(cn)))
					return -1

				else

					if cdown == 1 then
						server.msg("game will resume in " .. blue(cdown) .. " sec")
					else
						server.msg("game will resume in " .. blue(cdown) .. " secs")
					end
					cdown = cdown - 1

				end

			end)

		end
	end

end
