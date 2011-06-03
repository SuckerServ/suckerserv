
return function(cn, countdown)

	if not countdown then
		server.pausegame(false)
		server.msg(red(">>> " ..white"Pause is now " ..blue"off"))
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
						server.msg(red(">>> " ..white"game will resume in " .. blue(cdown) .. " secs"))
					else
						server.msg(red(">>> " ..white"game will resume in " .. blue(cdown) .. " secs"))
					end
					cdown = cdown - 1

				end

			end)

		end
	end

end
