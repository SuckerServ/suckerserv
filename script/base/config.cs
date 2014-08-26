# Declarations and default values for the Core Configuration Variables

global motd "Running Hopmod"

global display_country_on_connect 1
global display_city_on_connect 0
global display_rank_on_connect 0

global enable_timezone 1

global mmdb_file "share/GeoLite2-City.mmdb"

global allow_mapvote 1
global mapvote_disallow_unknown_map 0
global mapvote_disallow_excluded_map 0

global allowed_gamemodes [
    "ffa"
    "coop edit"
    "teamplay"
    "instagib"
    "insta team"
    "efficiency"
    "effic team"
    "tactics"
    "tac teams"
    "capture"
    "regen capture"
    "ctf"
    "insta ctf"
    "protect"
    "insta protect"
    "hold"
    "insta hold"
    "effic ctf"
    "effic protect"
    "effic hold"
    "insta collect"
    "effic collect"
    "collect"
]

global use_server_maprotation 1
global map_rotation_type "standard"

global small_single_game 5
global small_team_game 5

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
flood_protect_disc_timewindow 30000
flood_protect_disc_max 3

global shell_label "server"

global publicserver 1
global masterserver "sauerbraten.org"
global masterserverport 28787
global masterservers ["master.crapmod.net 28787" "master.sauerbraten.org 28787"]

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

