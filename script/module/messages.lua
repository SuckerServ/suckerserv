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
	white = "7",
}

local native_msg = server.msg
local native_player_msg = server.player_msg

function interp(s, tab)
  return (s:gsub('(%%%b{})', function(w) return tab[w:sub(3, -2)] or w end))
end
getmetatable("").__mod = interp

local function parse_message(cn, text)
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

return {unload = function()
	messages = nil
	server.msg = native_msg
	server.player_msg = native_player_msg
end}
