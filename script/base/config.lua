stats_sub_command = {}

-- Map rotation
server.use_server_maprotation = 1
server.map_rotation_type = "standard"


-- Flood protect config

server.flood_protect_text = 1000
server.flood_protect_sayteam = 1000
server.flood_protect_mapvote = 1000
server.flood_protect_switchteam = 1000
server.flood_protect_switchname = 1000
server.flood_protect_remip = 10000
server.flood_protect_newmap = 10000
server.flood_protect_spectator = 10000
server.flood_protect_disc_timewindow = 30000
server.flood_protect_disc_max = 3

-- GeoIP module
server.mmdb_file = "share/GeoLite2-City.mmdb"




server.motd = "Running Hopmod"

server.display_country_on_connect = 1
server.display_city_on_connect = 0
server.display_rank_on_connect = 0

server.enable_timezone = 1

server.allow_mapvote = 1
server.mapvote_disallow_unknown_map = 0
server.mapvote_disallow_excluded_map = 0

server.allowed_gamemodes = {
    "ffa",
    "coop edit",
    "teamplay",
    "instagib",
    "insta team",
    "efficiency",
    "effic team",
    "tactics",
    "tac teams",
    "capture",
    "regen capture",
    "ctf",
    "insta ctf",
    "protect",
    "insta protect",
    "hold",
    "insta hold",
    "effic ctf",
    "effic protect",
    "effic hold",
    "insta collect",
    "effic collect",
    "collect",
}

server.small_single_game = 5
server.small_team_game = 5

server.default_gamemode = "ffa"
server.default_game_on_empty = 0

server.shell_label = "server"

server.publicserver = 1
server.masterserverport = 28787
server.masterservers = {{"master.sauerbraten.org", "28787"}}

server.masterauth_banned = {}
server.server_admin_password = ""

server.web_admins = {}

server.mute_triggers = {"nigger", "nigga", "negro", "kike", "faggot", "motherfucker", "jude", "wichser", "kanake", "polake", "kinderficker", "scheiss auslaender"}

server.saveconf_vars = {"saveconf_vars", "servername", "motd", "maxclients", "admin_password", "master_password"}

server.command_prefixes = "! @ [#]"

server.admin_password = ""
server.master_password = ""
server.allow_setmaster = 0

server.irc_socket_password = ""

server.ban_lists = {
    { url = "https://raw.githubusercontent.com/pisto/ASkidban/master/compiled/ipv4", format = "raw", name = "ASkidban", reason = "proxy" }
}
