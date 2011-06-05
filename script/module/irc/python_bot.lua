--[[

HopMod Module for the Python-IRC-Bot
(c) 2009-2010 by Thomas, PunDit, Michael

The SocketListener will wait on the serverport + 10 (for example 28785 + 10 = 28795) for the bot

--]]

-- begin of irc functions

local function irc_color_white(str) return string.format("0%s", str) end
local function irc_color_black(str) return string.format("1%s", str) end
local function irc_color_navy(str) return string.format("2%s", str) end
local function irc_color_green(str) return string.format("3%s", str) end
local function irc_color_red(str) return string.format("4%s", str) end
local function irc_color_brown(str) return string.format("5%s", str) end
local function irc_color_purple(str) return string.format("6%s", str) end
local function irc_color_orange(str) return string.format("7%s", str) end
local function irc_color_yellow(str) return string.format("8%s", str) end
local function irc_color_light_green(str) return string.format("9%s", str) end
local function irc_color_light_blue(str) return string.format("10%s", str) end
local function irc_color_neon(str) return string.format("11%s", str) end
local function irc_color_blue(str) return string.format("12%s", str) end
local function irc_color_pink(str) return string.format("13%s", str) end
local function irc_color_grey(str) return string.format("14%s", str) end
local function irc_color_light_grey(str) return string.format("15%s", str) end

local function irc_bold(str) return string.format("%s", str) end

-- end of irc functions

-- begin of security functions

local function check_inject(str)
	local i = 0
	local quotes = 0
	local comment_found = false
	while i < #str do
		local  char = string.sub(str, i, i)
		local _char = string.sub(str, i, i+1)
		if char == '"' or char == "'" then quotes = quotes + 1 end
		if _char == "--" then comment_found = true end	
		i = i + 1
	end
	return _if((quotes % 2 ~= 0 or comment_found), "sendmsg('code wasnt executed.')", str)
end

-- end of security functions

if server.irc_socket_password == nil then
	server.irc_socket_password = ""
end

server.sleep(1000, function() -- wait a second before starting bot-listener

if server.irc_socket_password == "" then error("please set a password for the python_bot module!") return end

require("net")

local irc_bot = net.tcp_acceptor("0.0.0.0", server.serverport+10)

local client_bot = net.tcp_client()

local chan = ""
local allow_stream = false

irc_bot:listen()

function sendmsg(msg) -- required to send an response to the client-bot
	if not allow_stream then return end
    if chan ~= "" then
		chan = chan .. " "
    end
    client_bot:async_send(chan..msg.."\r\n", function(success) end)
    chan = ""
end



local function process_irc_command(data)
	local data = string.sub(data, 0, string.len(data)-2) -- skip \n
    if string.find(data, "pass:") then
		local pass = string.gsub(data, "pass:", "")
		if pass ~= server.irc_socket_password then
			allow_stream = false
			sendmsg("pass:wrong password")
			client_bot:close()
		else
			allow_stream = true
		end
	end



	if not allow_stream == true then return end

	-- Please do not add command events above of 'allow_stream == true', because they could be executed without password input.
	-- Add them here


	if string.find(data, "code:") then
		local xchan = strSplit(data, " ")
		chan = xchan[1]
		  
		local tmp = ""
		for i, t in ipairs(xchan) do
			if i > 1 then
				tmp = tmp .. " " .. t
			end
		end
		
		local code = string.gsub(tmp, "code:", "")
		local tmp_tonumber = tonumber; tonumber = function(str) local num = tmp_tonumber(str); if num == nil then return -1 else return num end end; local tmp_os_exec = os.execute; os.execute = nil; local pcallret, success, errmsg = pcall(loadstring(check_inject(code))); os.execute = tmp_os_exec; tonumber = tmp_tonumber
				
		if success ~= nil then
			sendmsg("command failed.")
			server.log("irc error -> " .. success) 
		else
			server.log(string.format("irc -> executed: [[ %s ]]", code))
		end
	end

	if data == "ping" then
		sendmsg("pong")
	end
end


local function ircreadloop()
	client_bot:async_read_until("\n", function(data)
		if data then
			process_irc_command(data)
			ircreadloop()
		end
	end)
end

local function accept_next(irc_bot)
	irc_bot:async_accept(function(client)
		client_bot = client

		--sendmsg("1:connection accepted, waiting for password.")
		allow_stream = false
		ircreadloop()
			
		accept_next(irc_bot)
    end)
end

accept_next(irc_bot)

end)


-- begin of game events

server.event_handler("connect", function (cn)

    local ip = server.player_ip(cn)
    local country = geoip.ip_to_country(ip) 

    if country == "" then country = "unknown" end

    sendmsg(string.format(irc_color_red("CONNECT: ")..irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("from ")..irc_color_blue("%s"),server.player_name(cn),cn,country)) 
end)


server.event_handler("disconnect", function (cn,reason)

    local reason_tag = ""
    local ip = ""

    if reason ~= "normal" then
        ip = "(" .. server.player_ip(cn) .. ")"
        reason_tag = " because: " .. reason
    end

    if server.player_connection_time(cn) < 2 then
	return
    end

    if reason == "" then
		reason = "normal"
    end

    sendmsg(string.format(irc_color_red("DISCONNECT: ")..irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("REASON: ")..irc_color_blue("%s"), server.player_name(cn), cn, reason))
end)

server.event_handler("kick", function(cn, bantime, admin, reason)
    
    local reason_tag = ""
    if reason ~= "" then reason_tag = "for " .. reason end
    
    local action_tag = "kicked"
    if tonumber(bantime) < 0 then action_tag = "kicked and permanently banned" end
    
    sendmsg(string.format(irc_color_red("%s(cn:%i/ip:%s) was %s by %s reason: %s"),irc_bold(server.player_name(cn)),cn,server.player_ip(cn),action_tag,irc_bold(admin),reason_tag))
end)

server.event_handler("rename",function(cn, oldname, newname)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("renamed to ")..irc_color_blue("%s"),oldname,cn,newname))
end)

server.event_handler("reteam",function(cn, oldteam, newteam)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("switched to team ")..irc_color_blue("%s"),server.player_name(cn),cn,newteam))
end)

server.event_handler("text", function(cn, msg)

    if server.player_isbot(cn) then return end
	
    -- Hide player commands
    if string.match(msg, "^#.*") then 
        return 
    end
    
    local mute_tag = ""
    if server.is_muted(cn) then mute_tag = "(muted)" end
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i): ")..irc_color_blue("%s%s"),server.player_name(cn),cn,mute_tag,msg))
end)

server.event_handler("sayteam", function(cn, msg)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i)[team]: ")..irc_color_blue("%s"),server.player_name(cn),cn,msg))
end)

server.event_handler("mapvote", function(cn, map, mode)
    sendmsg(string.format(irc_color_red("MAPVOTE: ")..irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("MODE: ")..irc_color_blue("%s ")..irc_color_green("MAP: ")..irc_color_blue("%s"),server.player_name(cn),cn,mode,map))
end)

local MAPNAME = ""

server.event_handler("mapchange", function(map, mode)
    MAPNAME = map
    local playerstats = ""
    local sc = tonumber(server.speccount)
    local pc = tonumber(server.playercount) - sc
    playerstats = " " .. #server.clients() .. " players"
    if sc > 0 then playerstats = playerstats .. " " .. tostring(sc) .. " spectators" end
    
    if sendmsg ~= nil then
	sendmsg(string.format(irc_color_red("NEWGAME: ")..irc_color_green("MODE: ")..irc_color_blue("%s ")..irc_color_green("MAP: ")..irc_color_blue("%s")..irc_color_blue(" %i")..irc_color_green(" Players"), map, mode, #server.clients()))
    end
end)

server.event_handler("setmastermode", function(cn, oldmode, newmode)
    sendmsg(string.format("\0034MM\003    Mastermode changed to %s",newmode))
end)

server.event_handler("masterchange", function(cn, value)

    local action_tag = "claimed"
    if tonumber(value) == 0 then action_tag = "relinquished" end

    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("claimed ")..irc_color_blue("%s"), server.player_name(cn), cn, action_tag, server.player_priv(cn)))
end)

server.sleep(1, function()
	auth.listener("", function(cn, user_id, domain, status)
		if status ~= auth.request_status.SUCCESS then return end
		sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("claimed ")..irc_color_blue("master ")..irc_color_green("as ")..irc_color_green("'")..irc_color_blue("%s")..irc_color_green("'"), server.player_name(cn), cn, user_id))
	end)
end)

server.event_handler("spectator", function(cn, value)
    
    local action_tag = "joined"
    if tonumber(value) == 0 then action_tag = "left" end
    
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("%s ")..irc_color_blue("spectators"),server.player_name(cn),cn,action_tag))
end)

server.event_handler("gamepaused", function() sendmsg(irc_color_green("game is ")..irc_color_blue("paused")) end)
server.event_handler("gameresumed", function() sendmsg(irc_color_green("game is ")..irc_color_blue("resumed")) end)


server.event_handler("addbot", function(cn,skill,owner)
    local addedby = "server"
    if cn ~= -1 then addedby = server.player_name(cn) end
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("added a bot with the skill of ")..irc_color_blue("%i"), addedby, cn, skill))
end)

server.event_handler("delbot", function(cn)
    sendmsg(string.format(irc_color_blue("%s ")..irc_color_green("(%i) ")..irc_color_green("deleted a bot"),server.player_name(cn),cn))
end)

server.sleep(1, function()
	server.event_handler("beginrecord", function(id,filename)
		sendmsg(string.format("recording demo (%s)",filename))
	end)

	server.event_handler("endrecord", function(id, size)
	   sendmsg(string.format("end of demorecord (%s file size)",format_filesize(tonumber(size))))
	end)
end)

server.event_handler("checkmaps", function(cn)
	local modified_clients = server.modified_map_clients()
    for sessionid, cn in pairs(modified_clients) do
		sendmsg(string.format(irc_color_red("%s(cn:%i/map:%s/ip:%s) is using a modified map"),server.player_name(cn),cn, server.map, server.player_ip(cn)))
    end
end)

-- end of game events