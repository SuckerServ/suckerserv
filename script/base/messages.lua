--[[

  Module for customizing messages
  By piernov <piernov@piernov.org>

]]

messages = {}

messages.FORMAT_STRING = "%{text}"
messages.TIME_STRING = "%H:%M:%S"
messages.FORMAT_TABLE = {
	time = os.date(messages.TIME_STRING),
	green = "0",
	blue = "1",
	yellow = "2",
	red = "3",
	gray = "4",
	magenta = "5",
	orange = "6",
	white = "7",
}

local native_msg = server.msg
local native_player_msg = server.player_msg

function interp(s, tab)
  return (s:gsub('(%%%b{})', function(w) return tab[w:sub(3, -2)] or w end))
end
getmetatable("").__mod = interp

local function parse_message(cn, text)
	if server.enable_timezone == 1 then
		local fd = io.popen("TZ='%{timezone}' date +%{timestring}" % {timezone = server.player_vars(cn).timezone, timestring = messages.TIME_STRING} )
		messages.FORMAT_TABLE.time = fd:read('*l'):gsub('\n*$', '')
		fd:close()
	end

	text = text % messages.FORMAT_TABLE
	messages.FORMAT_TABLE.text = text
	return messages.FORMAT_STRING % messages.FORMAT_TABLE
end

function server.player_msg(cn, text)
	native_player_msg(cn, parse_message(cn, text))
end

function server.msg(text)
	for client in server.gclients() do
		client:msg(text)
	end
end

server.event_handler("disconnect", function(cn)
	local normal_message = server.client_disconnect_message % {name = server.player_name(cn), cn = cn}
	local admin_message = normal_message .. server.client_disconnect_admin_message % {ip = server.player_ip(cn)}

	for _, cn in ipairs(server.clients()) do
		if server.player_priv_code(cn) == server.PRIV_ADMIN then
			server.player_msg(cn, admin_message)
		else
			server.player_msg(cn, normal_message)
		end
	end
end)

return {unload = function()
	messages = nil
	server.msg = native_msg
	server.player_msg = native_player_msg
end}
