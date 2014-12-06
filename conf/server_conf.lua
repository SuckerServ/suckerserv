--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Cube 2: Sauerbraten Game Server Configuration                               --
-- Based on HopMod                                                             --
--                                                                             --
-- Visit http://piernov.org/dokuwiki/en:suckerserv:start for a full list of    --
-- configuration variables.                                                    --
--                                                                             --
--------------------------------------------------------------------------------------------------------------------------------------------------------------

-- A server name for players to identify your server.
server.servername = "SuckerServ-master"

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Default connection information:
--   Game Server socket binds to UDP 0.0.0.0:28785
--   Game Server Info socket binds to UDP 0.0.0.0:<serverport+1> (28786)
--------------------------------------------------------------------------------------------------------------------------------------------------------------

-- Server's IP address
--server.serverip = "0.0.0.0"

-- Game server port.
--server.serverport = 28785

-- Register your server with the master server every 1 hour to appear on the public server list.
server.publicserver = 1

-- Set the maximum number of client connections allowed
server.maxclients = 8

-- +1 slot each spectator
server.specslots = 0

-- Number of reserved connection slots for admin players
-- Admin use: /connect <serverip> [<serverport>] <admin_password>
-- Connecting admin players will have invisible admin status.
server.reserved_slots = 1
server.reserved_slots_password = "" -- Allow reserved slot usage without giving them admin privilege

-- Message of the day. This message is sent on player connection.
server.motd = "%{blue}Running %{orange}SuckerServ-master"

-- The admin password. Same password used by all admin players to gain admin privilege, by typing /setmaster <admin_password>
server.admin_password = ""

-- The master password. Same password used by all master players to gain master privilege, by typing /setmaster <master_password>. Only useful if allow_setmaster is 0
server.master_password = ""

-- Uncomment and set a server password to lock the game server and require connecting players to send the server password to get access.
-- Note: The Cube 2 client doesn't provide a password dialog GUI: players must use the command line to connect to a server with a given password.
--server.server_password = ""

-- Time in ms to wait at intermession before changing map — affect also mapbattle module
server.intermission_time = 30000

-- Toogles wether teamkilling the flag runner in CTF modes should disallow the teamkiller from stealing the flag
server.ctf_teamkill_penalty = true

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Authkey configuration for Name protection & Admin
--------------------------------------------------------------------------------------------------------------------------------------------------------------

server.admin_domains = {"suckerserv:admin"}            -- Domain for admin
server.invadmin_domains = {"suckerserv:admin"}         -- Same
server.auto_invadmin_domains = {"suckerserv:admin"}    -- Same, if you want to be auto-invadmin at connection

server.master_domains = {"suckerserv:master"}          -- Domain for master
server.invmaster_domains = {"suckerserv:master"}       -- Same
server.auto_invmaster_domains = {"suckerserv:master"}  -- Same, if you want to be auto-invadmin at connection

server.name_reservation_domain = "suckerserv"        -- Domain for name protect
server.auth_domains = {"suckerserv"}                   -- Domain for privileges with new authserver

server.module("auth/name")                          -- Module for name protect
server.module("auth/invmaster")                     -- Module for auto-invmaster
server.module("auth/invadmin")                      -- Module for auto-invadmin
--server.module("auth/privileges")                   -- New privileges module to set inv-master/admin with new authserver. Disable auth/invadmin and auth/invmaster before enabling this 

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Jabber Bot
--------------------------------------------------------------------------------------------------------------------------------------------------------------

--server.xmpp_jid = ""
--server.xmpp_password = ""
--server.xmpp_debug = 0
--server.xmpp_muc_jid = ""
--server.xmpp_muc_nick = "SuckServ-Bot"
--server.xmpp_muc_password = ""
--server.xmpp_bot_command_name = "--"

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Stats
--------------------------------------------------------------------------------------------------------------------------------------------------------------

server.stats_use_sqlite = 1                    -- Use a SQLite3 Database: default option, only if you can't connect to a MySQL server, because some functions are missing
server.stats_use_json = 0                      -- Use a JSON Database: very incomplete, don't use
server.stats_use_mysql = 0                     -- Use a MySQL Database: Best option if you have a MySQL server, otherwise, choose SQLite3
server.stats_use_psql = 0                      -- Use a PostGreSQL database, actually not working.
server.stats_query_backend = "sqlite3"         -- Used database : mysql, sqlite3, json, psql

server.stats_mysql_hostname = "localhost"      -- MySQL server hostname
server.stats_mysql_port = "3306"               -- MySQL server port
server.stats_mysql_username = "suckerserv"     -- MySQL database username
server.stats_mysql_password = "suckerserv"     -- MySQL database password
server.stats_mysql_database = "suckerserv"     -- MySQL database name
server.stats_mysql_install = true              -- Switch to false after first launch

server.stats_psql_hostname = "localhost"       -- PostGreSQL server hostname
server.stats_psql_port = "5432"                -- PostGreSQL server port
server.stats_psql_username = "suckerserv"      -- PostGreSQL database username
server.stats_psql_password = "suckerserv"      -- PostGreSQL database password
server.stats_psql_database = "suckerserv"      -- PostGreSQL database name
server.stats_psql_install = true               -- Switch to false after first launch

server.stats_servername = "SuckerServ"         -- Server name, for displaying in scoreboard
server.stats_use_auth = 1                      -- Use auth with stats
server.stats_auth_domain = "suckerserv"        -- Domain for auth
server.stats_overwrite_name_with_authname = 1  -- Replace current name with authkey's name in stats
server.stats_tell_auth_name = 1                -- Display authname at intermission and when authkey is validated


--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Map rotation
--------------------------------------------------------------------------------------------------------------------------------------------------------------

-- Possible values for map_rotation_type include: standard, random, size
server.map_rotation_type = "standard"

-- The map rotation lists
exec("conf/maps.lua")

-- The server's preferred game mode
server.default_gamemode = "ffa"

-- Change back to the default game mode when the server goes empty
server.default_game_on_empty = 1

-- For the next map, choose an appropiate sized map from a map rotation, depending on the number of players connected.
-- The small map set is used when the player count less than or equal to <small_gamesize>, else the big map set is used.
server.small_single_game = 5
server.small_team_game = 5

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Map vote restrictions
--------------------------------------------------------------------------------------------------------------------------------------------------------------

server.allow_mapvote = 1

server.allowed_gamemodes = {
    "ffa",
    "coop edit",
    "teamplay",
    "instagib",
    "instagib team",
    "efficiency",
    "efficiency team",
    "tactics",
    "tactics teams",
    "capture",
    "regen capture",
    "ctf",
    "insta ctf",
    "protect",
    "insta protect",
    "hold",
    "insta hold",
    "efficiency ctf",
    "efficiency protect",
    "efficiency hold",
    "insta collect",
    "efficiency collect",
    "collect",
}

-- Block votes for unknown maps (known maps are released maps found in the official game distribution)
server.mapvote_disallow_unknown_map = 1

-- Block votes for maps not in the game-mode's map rotation
server.mapvote_disallow_excluded_map = 0

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Master restrictions
--------------------------------------------------------------------------------------------------------------------------------------------------------------

server.default_mastermode = 0         -- Change default mastermode
server.allow_setmaster = 1            -- Allow /setmaster 1 command to be used for gaining master
server.allow_mastermode_veto = 1      -- Allow master to set veto mastermode 
server.allow_mastermode_locked = 1    -- Allow master to set locked mastermode
server.allow_mastermode_private = 0   -- Allow master to set private mastermode
server.allow_reconnect_with_private_mastermode = 1 -- Allow a player to reconnect when private mastermode is set
server.reset_mastermode = 1           -- Reset mastermode when last master leaves / drops privileges

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Web admin configuration
--------------------------------------------------------------------------------------------------------------------------------------------------------------

-- The web admin control panel url is http://localhost:28788/admin
-- There is no login required when access is through localhost

-- To create a new web admin user run this shell command: source bin/env.sh; bin/utils/luapp bin/utils/web_admin.lua <username> <password>
-- Copy and paste the output here into the web_admins list:
server.web_admins = {}

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Player Commands configuration
--------------------------------------------------------------------------------------------------------------------------------------------------------------

server.enable_commands({
    "cheater",
    "votekick",
    "specall",
    "unspecall",
    "uptime",
    "reload",
    "changetime",
    "players",
    "names",
    "givemaster",
    "mute",
    "unmute",
    "ban",
    "unban",
    "persist",
    "versus",
    "warning",
    "msg",
    "stats",
    "nextmap",
    "eval",
    "group",
    "specmsg",
    "slay",
    "recorddemo",
    "giveadmin",
    "forcespec",
    "unforcespec",
    "banlist",
    "admin",
    "invadmin",
    "master",
    "invmaster",
    "forgive",
    "mapsucks",
    "clanwar",
    "rename",
    "disconnect",
    "info",
    "help",
    "delgban",
    "gbans",
    "spy",
    "version",
    "privileges",
    "sendmap",
    "noedit",
    "editmute",
})

server.disable_commands({
    "noties",
    "suddendeath",
    "nodamage",
})

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Modules
--------------------------------------------------------------------------------------------------------------------------------------------------------------

--server.module("stats")                                 -- Record game statistics to a database (it is needed for the stats and names commands)

--server.module("display/ownage")                        -- Display player achievement messages
--server.module("display/awards")                        -- Show End Stats Game of ended map during the intermission

--server.module("detect/camping")                        -- Name and shame players who are found to be camping
--server.module("detect/camping_penalty")                -- Players who are camping go in specs for 10 seconds (penality)

--server.module("balance/teams/by_adding_bots")          -- Use bots to balance teams
--server.module("balance/teams/by_moving_players")       -- Player moving team balancing
--server.module("balance/teams/passive")                 -- Player moving team when they are death and cry Balance

--server.module("balance/server_size/by_spec_count")     -- Increase the server capacity; depends on the spectator count
--server.module("balance/server_size/by_mastermode")     -- Make server bigger when it goes into locked mode so that many more spectators can connect (default resize is 50 players)
--server.module("balance/server_size/by_gamemode")       -- Increase/ Decrease server; depends on the current gamemode

--server.module("override/gameduration")                 -- Change the usual 10 minutes game duration to 15 minutes
--server.module("override/default_mastermode")           -- Change the default mastermode (to locked by default)

--server.module("recordgames")                           -- Auto record demos of every game

--server.module("limit/inactivity")                      -- Move inactive (dead or not moving) players to spectators
--server.module("limit/spectating")                      -- Disconnect spectators at "intermission", after min. 30 minutes, when server is full and they haven't been active in chat for 5 minutes
--server.module("limit/ping")                            -- Move lagging players to spectators after 2 warnings


--server.module("gamemode/no_ties")                      -- Prevent ties by continouse increasing the maptime by one minute until there is a clear ranking order (it is needed for the noties command)
--server.module("gamemode/suddendeath")                  -- Like no ties, but it stops the game, immediately, when a team scores one more time (it is needed for the suddendeath btw. sd and nosd commands)

--server.module("jabber/lua_jabber_bot")                 -- A ugly and unstable Jabber Bot
--server.module("name_lock")                             -- Prevent players from renaming
--server.module("mute_spectators")                       -- Mute all spectators, requiered for #mutespecs and #unmutespecs commands

--server.module("teamkill_protect")			 -- Disallow teamkills if the teamkiller tk/frags ratio is above 0.25

--server.module("mapbattle")                             -- Vote for map at intermission
--server.module("mapsucks")                              -- Module for #mapsucks
--server.module("flagrun")                               -- Record and display best flagrun time

--------------------------------------------------------------------------------------------------------------------------------------------------------------
--IRC MODULES
--------------------------------------------------------------------------------------------------------------------------------------------------------------

--server.irc_socket_password = ""
--server.module("irc/python_bot")                        -- Required for the external irc-bot, dont forget to set a connection pass above!

--After that you need to configure the config.py file. You can find it in the /python_bot folder. Have fun!

--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Message customization
--------------------------------------------------------------------------------------------------------------------------------------------------------------
-- Take the message's var you want customize from script/modules/declare/messages.vars
-- Exemple: There's a "global forgive_actor_forgiven_message" in messages.vars
--          So the line to put here, for exemple, is → forgive_actor_forgiven_message (concat "You've been forgiven by" (blue "%s"))
