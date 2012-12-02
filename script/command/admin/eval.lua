--[[

	A player command to execute a script on the server

]]
require "cubescript"

return function(cn, ...)

	local code = ""

	for _, item in ipairs({...}) do
		item = tostring(item)
		if #item > 0 then
			if #code > 0 then
				code = code .. " "
			end
			code = code .. item
		end
	end

	if code == "" then
		return false, "#eval <code>"
	end

	--server.player_msg(cn, server["do"](code) or "")
    
	server.player_msg(cn, cubescript.eval_string(code) or "")
end

