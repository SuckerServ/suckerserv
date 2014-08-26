--[[
   
	Writes to a FIFO the server events and listen to another for interacting with the Jabber bot.
        This is *VERY* unstable and experimental.
        Based on the irc/lua_bot module.
        Many commands don't work, and all users can issue admin's commands.

]]

require "mmdb"
require "net"

local function noop() end -- Use this function instead of defining new empty callback functions
local debug_mode = server.xmpp_debug == 1

if server.xmpp_muc_password=="" then
	xmpp_muc_password = "no_password"
else
	xmpp_muc_password = server.xmpp_muc_password
end

-- Print debug messages
local function print_debug(msg)
    if debug_mode then
        print(string.format("[XMPP DEBUG] %s", msg))
    end
end

-- Initializing the module
print_debug("[Startup] : Loading lua_jabber_bot module")

-- Send a message to the bot
local function send_reply(msg)
    local file = io.open("conf/bot.fifo", "w")
    file:write(msg)
    file:close()
end

-- Process any commands
local function processCommand(nick, room, body)
    local command = body:match(server.xmpp_bot_command_name .. "(.+)")
    if not command then return; end

    print_debug(string.format("[processCommand] : %s <%s> %s", room or "<error>", nick or "<error>", command or "<error>"))
    
    local arguments = command:split("[^ \r\n]+")
    local commandName = arguments[1]
    
    local function handleUnknownCommand()
        send_reply("[ERROR] Unrecognized command " .. commandName)
    end
    
    local commandFunction = xmpp_commands[commandName] or handleUnknownCommand
    table.remove(arguments, 1) -- Remove the command name from the list of arguments
    
    for index, value in ipairs(arguments) do
        local number = tonumber(value)
        if number then arguments[index] = number end
    end
    
    xmpp_nick = nick
    xmpp_room = room
    xmpp_command = command
    
    local status = return_catch_error(commandFunction, unpack(arguments))
    
    if not status then
        send_reply("[ERROR] An error occured while executing the command")
    end
end

-- Command to display players
function playerList()

	player_list = {}
	counter = nil 
	output = ""
    
    for player in server.gplayers('all') do
        if counter == nil then counter = 0 end
        counter = counter + 1
        output = output .. string.format("%s (%s) ", player:displayname(), player.cn)
        if counter == 5 then send_reply("[WHO] "..output); counter = 0 end
    end

    if counter == nil then 
        send_reply("[WHO] Apparently no one is connected.") 
    else
        if counter < 5 and counter > 0 then send_reply("[WHO] "..output); end
    end
end

function commandList()
    output = ""
    for k in pairs(xmpp_commands) do
        output = output .. string.format("%s ", k)
    end
    send_reply("[HELP] Available commands: "..output)
end

-- Command list
xmpp_commands = {
    ['kick']        = function(cn, reason) server.kick(cn, 0, xmpp_nick, reason) end,
    ['version']     = function() send_reply("SuckerServ/HopMod JabberBot 0.99 (#suckerserv@irc.gamesurge.net)") end,
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
    ['say']		= sendAdminMessage,
    ['clearbans']	= server.clearbans,
    ['permban']		= server.permban,
    ['unsetban']	= server.unsetban,
    ['recorddemo']	= server.recorddemo,
    ['stopdemo']	= server.stopdemo,
    ['who']		= playerList,
    ['help']		= commandList
}

-- Handle Game Message Events
server.event_handler("connect", function (cn)
    local ip = server.player_ip(cn)
    local country = mmdb.lookup_ip(ip, "country", "names", "en")
    send_reply(string.format("[CONNECT] %s(%i) %s",server.player_name(cn),cn,country)) 
end)

server.event_handler("disconnect", function (cn,reason)

    local reason_tag = ""
    local ip = ""

    if reason ~= "normal" then
        ip = "(" .. server.player_ip(cn) .. ")"
        reason_tag = " because: " .. reason
    end

    send_reply(string.format("[DISCONNECT] %s(%i) %s disconnected %s, time %s", server.player_name(cn), cn, ip, reason_tag, server.format_duration(server.player_connection_time(cn))))
end)

function log_usednames(cn)
    if server.find_names_by_ip then
        local current_name = server.player_name(cn)
        local names = server.find_names_by_ip(server.player_ip(cn), current_name)
        
        local namelist = ""
        
        for index, name in ipairs(names) do
            local sep = ""
            if #namelist > 0 then sep = ", " end
            namelist = namelist .. sep .. name
        end
        
        send_reply(string.format("[NAMES] Names used by %s(%i): %s", current_name, cn, namelist))
    end
   
end

server.event_handler("kick", function(cn, bantime, admin, reason)
    
    local reason_tag = ""
    if reason ~= "" then reason_tag = "for " .. reason end
    
    local action_tag = "kicked"
    if tonumber(bantime) < 0 then action_tag = "kicked and permanently banned" end
    
    send_reply(string.format("[KICK] %s(%i) was %s by %s %s",server.player_name(cn),cn,action_tag,admin,reason_tag), noop)
end)

server.event_handler("rename",function(cn, oldname, newname)
    send_reply(string.format("[RENAME] %s(%i) renamed to %s",oldname,cn,newname))
end)

server.event_handler("reteam",function(cn, oldteam, newteam)
    send_reply(string.format("[CHANGETEAM] %s(%i) changed team to %s",server.player_name(cn),cn,newteam))
end)

server.event_handler("text", function(cn, msg)
	
    -- Hide player commands
    if string.match(msg, "^#.*") then 
        return 
    end
    
    local mute_tag = ""
    if server.is_muted(cn) then mute_tag = "(muted)" end
    send_reply(string.format("[CHAT] %s(%i) %s  ~>  %s",server.player_name(cn),cn,mute_tag,msg))
end)

server.event_handler("sayteam", function(cn, msg)
    send_reply(string.format("[TEAMCHAT] %s(%i) (team): %s\n",server.player_name(cn),cn,msg))
end)

server.event_handler("mapvote", function(cn, map, mode)
    send_reply(string.format("[VOTE] %s(%i) suggests %s on map %s",server.player_name(cn),cn,mode,map))
end)

server.event_handler("mapchange", function(map, mode)
    
    local playerstats = ""
    local sc = tonumber(server.speccount)
    local pc = tonumber(server.playercount) - sc
    playerstats = tostring(pc) .. " players"
    if sc > 0 then playerstats = playerstats .. " " .. tostring(sc) .. " spectators" end
    
    send_reply(string.format("[NEWMAP] New game: %s on %s, %s", mode, map, playerstats))
end)

server.event_handler("setmastermode", function(cn, oldmode, newmode)
    send_reply(string.format("[MM] Mastermode changed to %s",newmode))
end)

server.event_handler("masterchange", function(cn, value)

    local action_tag = "claimed"
    if tonumber(value) == 0 then action_tag = "relinquished" end

    send_reply(string.format("[MASTER] %s(%i) %s %s", server.player_name(cn), cn, action_tag, server.player_priv(cn)))
end)


server.event_handler("spectator", function(cn, value)
    
    local action_tag = "joined"
    if tonumber(value) == 0 then action_tag = "left" end
    
    send_reply(string.format("[SPEC] %s(%i) %s spectators",server.player_name(cn),cn,action_tag))
end)

server.event_handler("gamepaused", function() send_reply("[PAUSE] game is paused")end)
server.event_handler("gameresumed", function() send_reply("[RESM] game is resumed") end)

server.event_handler("addbot", function(cn,skill,owner)
    local addedby = "server"
    if cn ~= -1 then addedby = server.player_name(cn) .. string.format("(%i)", cn) end
    send_reply(string.format("[ADDBOT] %s added a bot (skill %i)", addedby, skill))
end)

server.event_handler("delbot", function(cn)
    send_reply(string.format("[DBOT] %s(%i) deleted a bot",server.player_name(cn),cn))
end)

server.event_handler("beginrecord", function(id,filename)
    send_reply(string.format("[DEMOSTART] Recording game to %s",filename))
end)

server.event_handler("endrecord", function(id, size)
    send_reply(string.format("[DEMOEND] finished recording game (%s file size)",format_filesize(tonumber(size))))
end)

server.event_handler("mapcrcfail", function(cn) 
    send_reply(string.format("[MCRC] %s(%i) has a modified map (%s %i). [ip: %s]",server.player_name(cn),cn, server.map, server.player_mapcrc(cn), server.player_ip(cn)))
    log_usednames(cn)
end)

server.event_handler("shutdown", function() 
    send_reply("[HALT] Server shutting down")
end)

server.event_handler("reloadhopmod", function() send_reply("[RELOAD] Reloading hopmod...") end)

-- Auth Listener for masterauth events
auth.listener("", function(cn, user_id, domain, status)
    if status ~= auth.request_status.SUCCESS then return end
    local msg = string.format("%s(%i) successfully authed", server.player_name(cn), cn)
    send_reply("[AUTH] " .. msg)
end)

local function sendAdminMessage()
    local chat = string.match(xmpp.command, "say%s*(.+)\n")
    server.console(xmpp.nick, chat)
end

os.execute("lua script/module/jabber/bot_listener.lua " .. server.xmpp_jid .. " " .. server.xmpp_password .. " " .. server.xmpp_muc_jid .. " " .. xmpp_muc_password .. " " .. server.xmpp_muc_nick .. " " .. server.xmpp_debug .. " &")

filename="conf/to_server.fifo"
server_fifo = assert(io.open(filename, "r"))

-- Main loop
server.interval(500, function ()
    local t = server_fifo:read("*l")
    if t=="READY" then send_reply("[START] Server started"); return; end
	if not t then return; end
    local room = t:match("<room>(.+)</room>")
    local nick = t:match("<nick>(.+)</nick>")
    local body = t:match("<body>(.+)</body>")
	processCommand(nick, room, body)
end)
