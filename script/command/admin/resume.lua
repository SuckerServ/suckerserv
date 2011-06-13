
return function(cn, countdown)

	if not countdown then
		server.pausegame(false)
	else
		local cdown = tonumber(countdown)

		if cdown < 1 then
			server.pausegame(false)
		else
			cdown = round(cdown, 0)

			server.interval(1000, function()

				if cdown == 0 then

					server.pausegame(false)
					return -1

				else

					if cdown == 1 then
						server.msg(string.format(server.game_resume_sec, cdown))
					else
						server.msg(string.format(server.game_resume_secs, cdown))
					end
					cdown = cdown - 1

				end

			end)

		end
	end

end
