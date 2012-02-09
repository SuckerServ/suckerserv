-- #slay <cn>|\"<name>\"|all

local usage = "#slay <cn>|\"<name>\"|all"

return function(cn, target)

	if not target then
		return false, usage
	end

	if target == "all" then
		for p in server.gplayers() do p:slay() end
        
		return
	elseif not server.valid_cn(target) then
		target = server.name_to_cn_list_matches(cn, target)

		if not target then
			return
		end
	else
		return false, usage
	end

	server.player_slay(target)
end
