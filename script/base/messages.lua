--[[

  Module for customizing messages
  By piernov <piernov@piernov.org>

]]

local luatz = {}

messages = {}

messages.languages = {
	FR = "fr",
	US = "en",
	default = "en",
}

for country, language in pairs(messages.languages) do
	messages[language] = require("script/base/languages/" .. language)
--	require("languages/" .. language)
end

messages.USE_LUATZ = 1
messages.TIME_STRING = "%H:%M:%S"
messages.DEFAULT_TIMEZONE = "GMT"
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

if messages.USE_LUATZ then
	luatz.time = require("luatz.gettime").gettime
	luatz.get_tz = require("luatz.tzcache").get_tz
end

local native_msg = server.msg
local native_player_msg = server.player_msg

function interp(s, tab)
	return (s:gsub('(%%%b{})', function(w) return tab[w:sub(3, -2)] or w end))
end
getmetatable("").__mod = interp

function server.player_lang(cn)
	if not server.player_vars(cn).language then
		server.player_vars(cn).language = messages.languages[server.mmdatabase:lookup_ip(server.player_ip(cn), "country", "iso_code")] or messages.languages["default"]
	end

	return server.player_vars(cn).language
end

function server.parse_message(cn, text, vars)
  local defmsgs = messages[messages.languages["default"]]
  local msgs = defmsgs
  if messages[server.player_lang(cn)] ~= nil then
    msgs = messages[server.player_lang(cn)]
  end

	if type(text) == "table" then
    if msgs[text[1]] ~= nil and msgs[text[1]][text[2]] ~= nil then
      text = msgs[text[1]][text[2]]
    elseif defmsgs[text[1]] ~= nil and defmsgs[text[1]][text[2]] ~= nil then
      text = defmsgs[text[1]][text[2]]
    else
      text = text[1] + " " + text[2]
    end
	else
    if msgs[text] ~= nil then
      text = msgs[text]
    elseif defmsgs[text] ~= nil then
      text = defmsgs[text]
    end
	end

	if server.enable_timezone == 1 then
		if messages.USE_LUATZ then
			messages.FORMAT_TABLE.time = os.date("!"..messages.TIME_STRING, server.player_vars(cn).timezone:localise(luatz.time()))
		else
			local fd = io.popen("TZ='%{timezone}' date +%{timestring}" % {timezone = server.player_vars(cn).timezone, timestring = messages.TIME_STRING} )
			messages.FORMAT_TABLE.time = fd:read('*l'):gsub('\n*$', '')
			fd:close()
		end
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

local function player_timezone(cn)
        local timezone
	if server.enable_timezone == 1 then
		local ret, val = pcall(function() return server.mmdatabase:lookup_ip(server.player_ip(cn), "location", "time_zone") end)
		if ret and val ~= "" then
			timezone = val
		else
			timezone = messages.DEFAULT_TIMEZONE
		end
		if messages.USE_LUATZ then
			timezone = luatz.get_tz(timezone)
		end
	end
        return timezone
end

server.event_handler("connect", function(cn)
	server.player_vars(cn).timezone = player_timezone(cn)
end)

server.event_handler("disconnect", function(cn)
	for _, dest_cn in ipairs(server.clients()) do
		if server.player_priv_code(dest_cn) == server.PRIV_ADMIN then
			server.player_msg(dest_cn, server.parse_message(dest_cn, "client_disconnect", {name = server.player_name(cn), cn = cn}) .. server.parse_message(dest_cn, "client_disconnect_admin", {ip = server.player_ip(cn)}))
		else
			server.player_msg(dest_cn, "client_disconnect", {name = server.player_name(cn), cn = cn})
		end
	end
end)

local function init()
	for p in server.gclients() do
		p:vars().timezone = player_timezone(cn)
	end
end

init()

return {unload = function()
	messages = nil
	server.msg = native_msg
	server.player_msg = native_player_msg
end}
