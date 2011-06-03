
return function(cn,option)

	if not option or option == "1" or option == "start" then
		server.recorddemo("log/demo/" .. os.date("%y_%m_%d.%k_%M") .. "." .. server.gamemode .. "." .. server.map .. ".dmo")
		server.msg(server.recorddemo_start_message)
	else
		server.stopdemo()
	end

end
