require "script/module/jabber/verse"
require "verse.client"

local st = require("util.stanza");
local xmpp_bot = verse.new()
local to_server_fifo_location="conf/to_server.fifo"
local bot_fifo_location="conf/bot.fifo"

xmpp_jid = arg[1]
xmpp_password = arg[2]
xmpp_muc_jid = arg[3]
xmpp_muc_password = arg[4]
xmpp_muc_nick = arg[5]
xmpp_debug = arg[6]

if xmpp_muc_password=="no_password" then
    xmpp_muc_password = ""
else
    xmpp_muc_password = xmpp_muc_password
end

local debug_mode = xmpp_debug == 1

-- Print debug messages
local function print_debug(msg)
    if debug_mode then
        print(string.format("[XMPP DEBUG] %s", msg))
    end
end

-- Send a message to the MUC
local function send_reply(msg)
    reply =	verse.message{ to = xmpp_muc_jid, type = "groupchat" }
    reply:tag("body"):text(msg)
    xmpp_bot:send(reply)
end

-- Join a MUC
local function room_join(stream, room, password, nick)
    if not nick or not room or not stream then
        print_debug("room_join needs the following arguments : stream room nick")
    end
    s = st.presence{from=stream.username.."@"..stream.host.."/"..stream.resource, to = room.."/"..nick}
    s:tag("x", {xmlns="http://jabber.org/protocol/muc"})
    if password then
        s:tag("password"):text(password)
    end
    print("XML SEND", s)
    stream:send(s)
end

-- Message handler hook
xmpp_bot:hook("stanza", function(stanza)
    local body = stanza:get_child("body");
    local event = {
        delay = stanza:get_child("delay", "urn:xmpp:delay");
        sender = {  nick = select(3, jid.split(stanza.attr.from));
                    room = jid.bare(stanza.attr.from) };
        body = (body and body:get_text()) or nil;
        stanza = stanza;
    };
    if stanza.name == "message" and stanza.attr.type == "groupchat" then
        if not event.body or not event.sender.nick or event.delay or event.sender.nick == xmpp_muc_nick then return; end
        data="<room>" .. event.sender.room .. "</room>" .. "<nick>" .. event.sender.nick .. "</nick>" .. "<body>" .. event.body .. "</body>"
        print_debug(data)
        local to_server_fifo = io.open(to_server_fifo_location, "w")
        to_server_fifo:write(data)
        to_server_fifo:close()
    end
end);

-- Call room_join function when ready
xmpp_bot:hook("ready", function()
    room_join(xmpp_bot, xmpp_muc_jid, xmpp_muc_password, xmpp_muc_nick);
end);

-- Initiate Connection
xmpp_bot:connect_client(xmpp_jid, xmpp_password);

-- Small loop for initializing connection
i=0
while i < 20 do
    i = i+1
    verse.step()
end

-- Tell the lua_jabber_bot.lua module that the bot is ready
local to_server_fifo = io.open(to_server_fifo_location, "w")
to_server_fifo:write("READY")
to_server_fifo:close()

-- Open the FIFO for recieving event from the server
bot_fifo = assert(io.open(bot_fifo_location, "r"))

-- Main loop
while true do
    verse.step()
    local t = bot_fifo:read("*l")
    if t then
        if t == "[HALT] Server shutting down" then send_reply(t); break; end
        send_reply(t)
    end
    os.execute("sleep " .. 0.01)
end

verse.quit()
