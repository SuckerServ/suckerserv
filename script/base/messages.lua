--[[

  Module for customizing messages
  By piernov <piernov@piernov.org>

]]

messages = {}

messages.languages = {
	FR = "fr",
	US = "en",
	default = "fr",
}

for country, language in pairs(messages.languages) do
	messages[language] = require("script/base/languages/" .. language)
--	require("languages/" .. language)
end


--messages.FORMAT_STRING = "%{text}"
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

function server.player_lang(cn)
	if not server.player_vars(cn).language then
		server.player_vars(cn).language = messages.languages[mmdb.lookup_ip(server.player_ip(cn), "country", "iso_code")] or messages.languages["default"]
	end

	return server.player_vars(cn).language
end

function server.parse_message(cn, text, vars)
	text = messages[server.player_lang(cn)][text] or messages[messages.languages["default"]][text] or text

	if server.enable_timezone == 1 then
		local fd = io.popen("TZ='%{timezone}' date +%{timestring}" % {timezone = server.player_vars(cn).timezone, timestring = messages.TIME_STRING} )
		messages.FORMAT_TABLE.time = fd:read('*l'):gsub('\n*$', '')
		fd:close()
	end

	local format_table = messages.FORMAT_TABLE

	if vars then
		for k, v in pairs(vars) do
			format_table[k] = v
		end
	end

	return text % format_table
end

function server.player_msg(cn, text, vars)
	native_player_msg(cn, server.parse_message(cn, text, vars))
end

function server.msg(text, vars)
	for client in server.gclients() do
		client:msg(text, vars)
	end
end

server.event_handler("connect", function(cn)
    if server.enable_timezone == 1 then
        server.player_vars(cn).timezone = mmdb.lookup_ip(server.player_ip(cn), "location", "time_zone")
    end
end)

server.event_handler("disconnect", function(cn)
	for _, dest_cn in ipairs(server.clients()) do
		if server.player_priv_code(dest_cn) == server.PRIV_ADMIN then
			server.player_msg(dest_cn, server.parse_message("client_disconnect", {name = server.player_name(cn), cn = cn}) .. server.parse_message("client_disconnect_admin", {ip = server.player_ip(cn)}))
		else
			server.player_msg(dest_cn, "client_disconnect", {name = server.player_name(cn), cn = cn})
		end
	end
end)

return {unload = function()
	messages = nil
	server.msg = native_msg
	server.player_msg = native_player_msg
end}
