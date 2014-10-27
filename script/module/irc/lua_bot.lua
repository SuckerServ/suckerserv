--[[
   
	Allows the server to be monitored and administered from IRC. This is 
	experimental and will change quite a bit.
    
    Copyright (C) 2009	Charles Christopher(PunDit) 
    
    MAINTAINERS
       	PunDit 
        
        * Watch out for message flooding
        
    TODO
		* Message throttling
		* Autorejoin Channels	
		* Support multiple channels
		* Support for all hopmod events
		* Command responses for commands with no event
		
]]

require "net"

local WAIT_TO_RECONNECT = 30000

local function noop() end -- Use this function instead of defining new empty callback functions
local debug_mode = server.irc_debug == 1

local function print_debug(msg)
    if debug_mode then
        print(string.format("IRC DEBUG %s", msg))
    end
end

print_debug("[Startup] : Loading irc/lua_bot module")

local irc = {}
irc.client = net.tcp_client()

--Connect and send required handshake information.
function irc:connectServer(client)

	irc.client:close()
	irc.client = net.tcp_client()
    
	irc.client:async_connect(server.irc_network, server.irc_network_port, function(errmsg) 
		
        if errmsg then
            irc:handleError(errmsg)
            return
        end
		
        local localAddress = irc.client:local_endpoint()
        print_debug(string.format("[SOCKET] : Local socket address %s:%s", localAddress.ip, localAddress.port))
        
        irc.client:async_send("NICK "..server.irc_bot_name.."\n", function(errmsg)
        
            if errmsg then
                irc:handleError(errmsg)
                return 
            end
            
            local username = server.irc_network_username..string.random(15,"%l%d")
            print_debug("[username] : Setting username to " .. username)
            
            irc.client:async_send("USER  ".. username .." 8 *  : HopBot\n", function(errmsg)
                if errmsg then
                    irc:handleError(errmsg) 
                    return
                end
            end)
            
            print_debug("[connect Server] : Connected to " .. server.irc_network)
            
            irc:readData(irc.client) 
		end)
	end)
end

--Main Data Loop
function irc:readData(client)
	irc.client:async_read_until("\n", function(data)
        if data then
            irc:processData(client, data)
            irc:readData(irc.client)
        else
            irc:handleError("Read error")
        end
	end)
end

--Process data read from the server.
function irc:processData(client, data)

    print_debug("[Server Output] : " .. data)
    
	-- If a ping is detected respond with a pong
	if data.find(data,"PING") then
		local pong = string.gsub(data,"PING","PONG",1)
		if debug_mode then print ("DEBUG [processData:PING] : "..pong) end
		irc.client:async_send(pong, noop)
		return
	end
    
	-- If mode changes in channel then assume operator status has changed. Request userlist
	if data.find(data,"MODE") then
		irc.client:async_send("NAMES "..server.irc_channel.."\n", noop)
        return
    end
    
	-- Parse user list and store it as the operator list
	if data.find(data,"353") then
        irc.operators = string.match(data,"%S* 353 (.*)")
        return
    end
    
	-- After a notice is detected then start the join operation
	if data.find(data,"NOTICE") then
		server.sleep(10000, function()
            irc.client:async_send("JOIN "..server.irc_channel.."\n", noop)
		end)
		return
	end 
    
	-- If the link is closed start over
	if data.find(data,"Closing Link") then
		irc.handleError("Link Closed")
		return
	end
    
	-- Detect a command and hand it off to the command processor
	if string.match(data,":(.+)!.+ PRIVMSG (.+) :"..server.irc_bot_command_name.." (.+)") then
		local nick, channel, command = string.match(data,":(.+)!.+ PRIVMSG (.+) :"..server.irc_bot_command_name.." (.+)")
        irc.processCommand(nick, channel, command)
		return
	end
end 

--Process any commands
function irc.processCommand(nick, channel, command)
    
    print_debug(string.format("[processCommand] : %s <%s> %s", channel or "<error>", nick or "<error>", command or "<error>"))
    
    local arguments = command:split("[^ \r\n]+")
    local commandName = arguments[1]
    
    local function handleUnknownCommand()
        irc:toChannel("\0036IRC\003         \0034ERROR\003  \0034Unrecognized command\003")
    end
    
    local commandFunction = irc.commands[commandName] or handleUnknownCommand
    table.remove(arguments, 1) -- Remove the command name from the list of arguments
    
    for index, value in ipairs(arguments) do
        local number = tonumber(value)
        if number then arguments[index] = number end
    end
    
    irc.command_nick = nick
    irc.command_channel = channel
    irc.command_command = command
    
    local status = catch_error(commandFunction, unpack(arguments))
    
    if not status then
        irc:toChannel("\0036IRC\003         \0034ERROR\003  \0034An error occured while executing the command\003")
    end
end

--Command to display players
function irc:playerList()

	player_list = {}
	counter = nil 
	output = ""
    
    for player in server.gplayers('all') do
        if counter == nil then counter = 0 end
        counter = counter + 1
        output = output .. string.format("\00312%s\003(%s) ", player:displayname(), player.cn)
        if counter == 5 then irc:toChannel("\0036IRC\003         \0034-/WHO/-\003 is "..output); counter = 0 end
    end

    if counter == nil then 
        irc:toChannel("Apparently no one is connected.") 
    else
        if counter < 5 and counter > 0 then irc:toChannel("\0036IRC\003         \0034-/WHO/-\003 is "..output); end
    end
end

--Send message to channel.
function irc:toChannel(message)
	if message then
		irc.client:async_send( "PRIVMSG "..server.irc_channel.." : "..message.."\n", noop)
        print_debug(string.format("[Sending] : %s : %s", server.irc_channel, message))
		return
	end
end

--Handle any errors.
function irc:handleError(errmsg, retry)

    if not errmsg then return end
    retry = retry or WAIT_TO_RECONNECT
    
    irc.client:cancel()
    irc.client:close()
    print_debug("[handleError] : " .. errmsg)
    
    if retry ~= -1 then    
        server.sleep(retry, function() 
            irc:connectServer(irc.client)
        end)
    end
end

-- Initiate Connection
irc:connectServer(irc.client)

-- Handle Game Message Events
server.event_handler("connect", function (cn)
    local ip = server.player_ip(cn)
    local country =  server.mmdatabase:lookup_ip(ip, "country", "names", messages.languages["default"])
    irc:toChannel(string.format("\0039CONNECT\003    \00312%s(%i)\003 \0037%s\003",server.player_name(cn),cn,country)) 
end)

server.event_handler("disconnect", function (cn,reason)

    local reason_tag = ""
    local ip = ""

    if reason ~= "normal" then
        ip = "(" .. server.player_ip(cn) .. ")"
        reason_tag = " because: " .. reason
    end

    irc:toChannel(string.format("\0032DISCONNECT\003    \00312%s(%i)\003%s disconnected%s, time %s\n", server.player_name(cn), cn, ip, reason_tag, server.format_duration(server.player_connection_time(cn))))
end)

local function log_usednames(cn)

    if server.find_names_by_ip then
        local current_name = server.player_name(cn)
        local names = server.find_names_by_ip(server.player_ip(cn), current_name)
        
        local namelist = ""
        
        for index, name in ipairs(names) do
            local sep = ""
            if #namelist > 0 then sep = ", " end
            namelist = namelist .. sep .. name
        end
        
        irc:toChannel(string.format("\0039NAMES\003    Names used by \00312%s(%i)\003: %s\n", current_name, cn, namelist))
    end
   
end

server.event_handler("kick", function(cn, bantime, admin, reason)
    
    local reason_tag = ""
    if reason ~= "" then reason_tag = "for " .. reason end
    
    local action_tag = "kicked"
    if tonumber(bantime) < 0 then action_tag = "kicked and permanently banned" end
    
    irc:toChannel(string.format("\0034KICK\003    \00312%s(%i)\003 was \0037%s\003 by \0037%s\003 \0037%s\003\n",server.player_name(cn),cn,action_tag,admin,reason_tag), noop)
end)

server.event_handler("rename",function(cn, oldname, newname)
    irc:toChannel(string.format("\0032RENAME\003  \00312%s(%i)\003 renamed to \0037%s\003\n",oldname,cn,newname))
end)

server.event_handler("reteam",function(cn, oldteam, newteam)
    irc:toChannel(string.format("\0034CHANGETEAM\003    \00312%s(%i)\003 changed team to \0037%s\003\n",server.player_name(cn),cn,newteam))
end)

server.event_handler("text", function(cn, msg)
	
    -- Hide player commands
    if string.match(msg, "^#.*") then 
        return 
    end
    
    local mute_tag = ""
    if server.is_muted(cn) then mute_tag = "(muted)" end
    irc:toChannel(string.format("\0033CHAT\003    \00312%s(%i)\003%s  ~>  \0033%s\003\n",server.player_name(cn),cn,mute_tag,msg))
end)

server.event_handler("sayteam", function(cn, msg)
    irc:toChannel(string.format("\0033TEAMCHAT\003    \00312%s(%i)\003(team): %s\n",server.player_name(cn),cn,msg))
end)

server.event_handler("mapvote", function(cn, map, mode)
    irc:toChannel(string.format("\0033VOTE\003    \00312%s(%i)\003 suggests \0037%s\003 on map \0037%s\003",server.player_name(cn),cn,mode,map))
end)

server.event_handler("mapchange", function(map, mode)
    
    local playerstats = ""
    local sc = tonumber(server.speccount)
    local pc = tonumber(server.playercount) - sc
    playerstats = tostring(pc) .. " players"
    if sc > 0 then playerstats = playerstats .. " " .. tostring(sc) .. " spectators" end
    
    irc:toChannel(string.format("\0032NEWMAP\003    New game: \0037%s\003 on \0037%s\003, \0037%s\003", mode, map, playerstats))
end)

server.event_handler("setmastermode", function(cn, oldmode, newmode)
    irc:toChannel(string.format("\0034MM\003    Mastermode changed to %s",newmode))
end)

server.event_handler("masterchange", function(cn, value)

    local action_tag = "claimed"
    if tonumber(value) == 0 then action_tag = "relinquished" end

    irc:toChannel(string.format("\0034MASTER\003    \00312%s(%i)\003 %s \0037%s\003", server.player_name(cn), cn, action_tag, server.player_priv(cn)))
end)


server.event_handler("spectator", function(cn, value)
    
    local action_tag = "joined"
    if tonumber(value) == 0 then action_tag = "left" end
    
    irc:toChannel(string.format("\0034SPEC\003    \00312%s(%i)\003 %s spectators",server.player_name(cn),cn,action_tag))
end)

server.event_handler("gamepaused", function() irc:toChannel("\0034PAUSE\003    game is paused")end)
server.event_handler("gameresumed", function() irc:toChannel("\0034RESM\003    game is resumed") end)

server.event_handler("addbot", function(cn,skill,owner)
    local addedby = "server"
    if cn ~= -1 then addedby = "\00312" .. server.player_name(cn) .. string.format("(%i)\003", cn) end
    irc:toChannel(string.format("\00315ADDBOT\003    %s added a bot (skill %i)", addedby, skill))
end)

server.event_handler("delbot", function(cn)
    irc:toChannel(string.format("\00315DBOT\003    \00312%s(%i)\003 deleted a bot\n",server.player_name(cn),cn))
end)

server.event_handler("beginrecord", function(id,filename)
    irc:toChannel(string.format("\00312DEMOSTART\003    Recording game to %s",filename))
end)

server.event_handler("endrecord", function(id, size)
    irc:toChannel(string.format("\00312DEMOEND\003    finished recording game (%s file size)\n",format_filesize(tonumber(size))))
end)

server.event_handler("mapcrcfail", function(cn) 
    irc:toChannel(string.format("\0034MCRC\003    \00312%s(%i)\003 has a modified map (%s %i). [ip: %s]\n",server.player_name(cn),cn, server.map, server.player_mapcrc(cn), server.player_ip(cn)))
    log_usednames(cn)
end)

server.event_handler("shutdown", function() irc:toChannel("\0034HALT\003    Server shutting down"); end)

server.event_handler("reloadhopmod", function() irc:toChannel("\0034RELOAD\003    Reloading hopmod...\n") end)

irc:toChannel("\00312START\003    Server started")

-- Auth Listener for masterauth events
auth.listener("", function(cn, user_id, domain, status)
    if status ~= auth.request_status.SUCCESS then return end
    local msg = string.format("\00312%s(%i)\003 successfully authed", server.player_name(cn), cn)
    irc:toChannel(" \0034AUTH\003    " .. msg)
end)

local function sendAdminMessage()
    local chat = string.match(irc.command_command, "say%s*(.+)\n")
    server.console(irc.command_nick, chat)
end

-- Command list
irc.commands = {
    ['kick']        = function(cn, reason) server.kick(cn, 0, irc.command_nick, reason) end,
    ['version']     = function() irc:toChannel("HopBot v.l01 support #hopmod@irc.gamesurge.net") end,
    ['spec']        = server.spec,
    ['unspec']      = server.unspec,
    ['mute']        = server.mute,
    ['unmute']      = server.unmute,
    ['setmaster']   = server.setmaster,
    ['setadmin']    = server.setadmin,
    ['unsetmaster'] = server.unsetmaster,
    ['slay']        = server.player_slay,
    ['changeteam']  = server.changeteam,
    ['pause']       = function() server.pausegame(true) end,
    ['unpause']     = function() server.pausegame(false) end,
    ['map']         = server.changemap,
    ['delbot']     	= server.delbot,
    ['restart']		= server.restart,
    ['kill']		= server.restart_now,
    ['shutdown']	= server.shutdown,
    ['reload']		= server.reloadscripts,
    ['say']			= sendAdminMessage,
    ['clearbans']	= server.clearbans,
    ['permban']		= server.permban,
    ['unsetban']	= server.unsetban,
    ['recorddemo']	= server.recorddemo,
    ['stopdemo']	= server.stopdemo,
    ['who']			= irc.playerList
}
