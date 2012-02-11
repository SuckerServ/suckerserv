# Declarations and default values for the Core Configuration Variables

global motd "Running Hopmod"

global show_country_message 1

global geoip_db_file "share/GeoIP.dat"
global geocity_db_file "share/GeoLiteCity.dat"

global allow_mapvote 1
global mapvote_disallow_unknown_map 0
global mapvote_disallow_excluded_map 0

global allowed_gamemodes [
    "ffa"
    "coop edit"
    "teamplay"
    "instagib"
    "instagib team"
    "efficiency"
    "efficiency team"
    "tactics"
    "tactics teams"
    "capture"
    "regen capture"
    "ctf"
    "insta ctf"
    "protect"
    "insta protect"
    "hold"
    "insta hold"
    "efficiency ctf"
    "efficiency protect"
    "efficiency hold"
    "insta collect"
    "efficiency collect"
    "collect"
]

global use_server_maprotation 1
global map_rotation_type "standard"

global small_single_game 5
global small_team_game 5

exec "script/base/maprotation/maps.cs"

global default_gamemode "ffa"
global default_game_on_empty 0

flood_protect_text 1000
flood_protect_sayteam 1000
flood_protect_mapvote 1000
flood_protect_switchteam 1000
flood_protect_switchname 1000
flood_protect_remip 10000
flood_protect_newmap 10000
flood_protect_spectator 10000

global shell_label "server"

global publicserver 1
global masterserver "sauerbraten.org"
global masterserverport 28787

global masterauth_banned []
global server_admin_password ""

global web_admins []

global mute_triggers [nigger nigga negro kike faggot motherfucker jude wichser kanake polake kinderficker "scheiss auslaender"]

global saveconf_vars [saveconf_vars servername motd maxclients admin_password master_password]

global command_prefixes "! @ [#]"

global admin_password ""
global master_password ""
global allow_setmaster 0

global irc_socket_password ""

global ban_lists [
    "http://hopmod.googlecode.com/svn/data/bans.json"
]

