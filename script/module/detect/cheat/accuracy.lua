local min_acc = server.cd_accuracy_min_acc
local min_played_time = server.cd_accuracy_min_played_time
local min_frags = server.cd_accuracy_min_frags

server.event_handler("finishedgame",function()

	for p in server.aplayers() do

		local acc = p:accuracy()
		local time = p:timeplayed()
		local frags = p:frags() + p:teamkills() + p:suicides()

		if (frags > min_frags) and (time > min_played_time) and (acc > min_acc) then
			server.log("WARNING: " .. p:name() .. "(" .. p.cn .. ") had an accuracy of: " .. acc .. "%. [frags/deaths: " .. frags .. "/" .. p:deaths() .. " | ip: " .. p:ip() .. "]")
		end
	end
end)

