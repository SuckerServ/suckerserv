-- #slay <cn>|\"<name>\"|all

local usage = "#slay <cn>|\"<name>\"|all"

return function(cn, target)

	if not target then
		return false, usage
	end

	if target == "all" then
		for p in server.gplayers() do p:slay() end
        
		return
	elseif server.valid_cn(target) then
    server.player_slay(target)
  else
		target = server.name_to_cn_list_matches(cn, target)

		if target then
      server.player_slay(target)
    else
      return false, usage
    end
	end
end
