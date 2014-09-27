
-- stats Module
server.stats_db_filename = "log/stats.sqlite"
server.stats_query_backend = "sqlite3"
server.stats_use_auth = 0
server.stats_auth_domain = ""
server.stats_tell_auth_name = 0
server.stats_debug = 0
server.stats_use_sqlite = 1
server.stats_sqlite_exclusive_locking = 0
server.stats_sqlite_synchronous = 1
server.stats_use_json = 0
server.stats_overwrite_name_with_authname = 0
server.stats_use_mysql = 0
server.stats_servername = ""
server.stats_mysql_hostname = "localhost"
server.stats_mysql_port = 3306
server.stats_mysql_database = "sauerstats"
server.stats_mysql_username = ""
server.stats_mysql_password = ""
server.stats_mysql_install = 0
server.stats_use_psql = 0
server.stats_psql_hostname = "localhost"
server.stats_psql_port = 5432
server.stats_psql_database = "sauerstats"
server.stats_psql_username = ""
server.stats_psql_password = ""
server.stats_psql_install = 0
server.stats_enabled_gamemodes =
{
    "ffa",
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
    "effic ctf",
    "protect",
    "insta protect",
    "effic protect",
    "hold",
    "insta hold",
    "effic hold",
    "insta collect",
    "effic collect",
    "collect",
}

-- Teambalance Modules
-- by adding bots
server.teambalance_using_moveblock = 1
server.teambalance_bot_skill_low = 65
server.teambalance_bot_skill_high = 85
-- by adding bots (alternative)
--server.teambalance_using_moveblock = 1
server.teambalance_using_player_moving_balancer_when_leaving_spec = 1
server.teambalance_using_text_addin = 0
--server.teambalance_bot_skill_low = 65
--server.teambalance_bot_skill_high = 85
-- by moving players
--server.teambalance_using_moveblock = 1
--server.teambalance_using_player_moving_balancer_when_leaving_spec = 1
-- passive balancer
--server.teambalance_using_moveblock = 1
--server.teambalance_using_player_moving_balancer_when_leaving_spec = 1
--server.teambalance_using_text_addin = 0

-- nameprotection
server.name_reservation_domain = ""

-- resize_server/mastermode Module
server.resize_server_mastermode_size = 50
server.resize_server_mastermode_using_mastermode = 2

-- resize_server/gamemode Module
server.resize_server_gamemode_size_regen_capture = 12
server.resize_server_gamemode_size_capture = 12
server.resize_server_gamemode_size_effic_ctf = 10
server.resize_server_gamemode_size_insta_ctf = 10
server.resize_server_gamemode_size_ctf = 10
server.resize_server_gamemode_size_effic_protect = 10
server.resize_server_gamemode_size_insta_protect = 10
server.resize_server_gamemode_size_protect = 10
server.resize_server_gamemode_size_effic_hold = 10
server.resize_server_gamemode_size_insta_hold = 10
server.resize_server_gamemode_size_hold = 10
server.resize_server_gamemode_size_teamplay = 8
server.resize_server_gamemode_size_ffa = 6
server.resize_server_gamemode_size_effic_team = 8
server.resize_server_gamemode_size_effic = 6
server.resize_server_gamemode_size_tactics_team = 8
server.resize_server_gamemode_size_tactics = 6
server.resize_server_gamemode_size_insta_team = 8
server.resize_server_gamemode_size_insta = 6
server.resize_server_gamemode_size_coop_edit = 14

-- disallow_mastermodes_for_admins
server.disallow_mastermode_open_for_admins = 0
server.disallow_mastermode_veto_for_admins = 0
server.disallow_mastermode_locked_for_admins = 0
server.disallow_mastermode_private_for_admins = 0

-- auto invmaster Module
server.auto_invmaster_domains = {}

-- auto invadmin Module
server.auto_invadmin_domains = {}

-- mute spectators Module
server.mute_spectators_enabled_by_default = 0

-- limit spec Module
server.spectating_limit = server.mins(30)

-- spec inactives Module
server.inactivity_check_time = server.mins(1)
server.inactivity_time = server.secs(90)
server.inactivity_death_only = 0

-- limit ping Module
server.ping_check_time = server.secs(45)
server.ping_limit = 600
server.ping_pj_limit = 50

-- no ties Module
server.no_ties_enabled_by_default = 0

-- suddendeath Module
server.suddendeath_enabled_by_default = 0

-- change_default_maptime Module
server.default_maptime = server.mins(15)

-- change_default_mastermode
server.default_mastermode = 2


-- cheatdetect Modules
server.cd_accuracy_min_acc = 75
server.cd_accuracy_min_played_time = server.mins(1)
server.cd_accuracy_min_frags = 1

-- mute Module
server.mute_default_time = server.mins(60)

-- mapsucks command
server.mapsucks_ratio = 2
server.mapsucks_lower_time = 1

server.auth_domains = {"suckerserv"} -- Domain for privileges with new authserver

-- camping penalty timeout
server.camping_penalty_timeout = 11

-- votekick command
server.votekick_min_votes_required = 2
