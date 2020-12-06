return {
server_start = "--> Successfully loaded Suckerserv.",

client_connect = "(%{green}%{time}%{white}) Connection from %{yellow}%{country}%{white}: %{blue}%{name} %{magenta}(%{cn}) %{white}(%{priv}%{white})",
client_connect_admin = "%{white}IP: %{blue}%{ip}%{white}",
client_disconnect = "(%{green}%{time}%{white}) Disconnected: %{blue}%{name}%{white} %{magenta}(%{cn})",
client_disconnect_admin = " %{white}(IP: %{blue}%{ip}%{white})",

connect_info = "%{red}>>> %{white}Type %{magenta}#%{white}help to %{yellow}get %{white}the %{blue}list %{white}of %{green}commands%{white}.",

client_crcfail_player = "%{red}>>> %{blue}You %{yellow}are using %{white}a modified %{green}map!",
client_crcfail = "%{red}>>> %{white}Player %{blue}%{name} %{yellow}is using %{white}a modified %{green}map!",

clearbans = "%{red}>>> %{green}Cleared all %{blue}bans.",

awards_stats = "%{red}>>> %{white}Awards: %{stats}",
awards_flags = "%{red}>>> %{white}Best Flags: %{flagstats}",

inactivitylimit = "%{red}>>> %{white}Server moved you to spectators, because you seem to be inactive - type '/spectator 0' to rejoin the game.",

command_disabled = "%{red}>>> ERROR: %{white}Command disabled",
command_permission_denied = "%{red}>>> ERROR: %{white}Permission Denied",
command_internal_error = "%{red}>>> ERROR: %{white}Internal error",
command_syntax_error = "%{red}>>> ERROR: %{white}Command syntax error: %{err}",
command_not_found = "%{red}>>> ERROR %{white}Command not found",
command_error = "%{red}>>> ERROR: %{white}Command Error",

master_already = "%{red}>>> %{white}An %{orange}admin %{white}or %{green}master %{white}is already %{blue}here%{white}.",
setmaster_refused = "%{red}>>> %{orange}WARNING%{white}: no longer accepting your %{blue}setmaster %{white}requests",

demo_recording = "%{red}>>> %{white}Recording demo",

info_command = "%{red}>>> %{white}Serveur uptime : %{blue}%{uptime}%{white}. SuckerServ %{verstr}",
version = "%{white}rev%{blue}%{revision}%{white} compiled at %{green}%{version}%{white}",

help = "Command descriptions: #help <command>\n%{blue}List %{white}of %{green}commands%{white}",

stats_logged_in = "%{red}>>> %{white}You are logged in as %{blue}%{user_id}",

mapbattle_winner = "%{red}>>> %{white}Winner: %{blue}%{mapbattle_winner}",
mapbattle_vote = "%{red}>>> %{white}Vote for map %{blue}%{map1} %{white}or %{blue}%{map2} %{white}with %{green}1 %{white}or %{green}2",
mapbattle_vote_ok = "%{red}>>> %{blue}%{name} %{white}voted for %{blue}%{mapname}",
mapbattle_vote_already = "%{red}>>> You have already voted",
mapbattle_cant_vote = "%{red}>>> %{orange}Spectators are not allowed to vote!",

client_nameprotect = "%{red}>>> %{white}You are %{yellow}logged in %{white}as %{blue}%{user_id}%{white}.",
nameprotect_rename = "%{red}>>> %{white}You have used a reserved name of another player. Server %{blue}renamed %{white}you to %{blue}'unnamed'%{white}.",

flagrun = "%{red}>>> %{blue}%{name} %{white}scored in %{magenta}%{time} %{white}seconds. Best: %{blue}%{bestname} %{white}in %{magenta}%{besttime} %{white}seconds.",

giveadmin = "%{red}>>> %{blue}%{name} %{white}has passed admin privilege to you.",
claimmaster = "%{red}>>> %{blue}%{name} %{white}claimed %{green}master %{white}as %{magenta}'%{uid}'",
claimadmin = "%{red}>>> %{blue}%{name} %{white}claimed %{orange}admin %{white}as %{magenta}'%{uid}'",

votekick_passed = "%{red}>>> %{white}Vote passed to %{red}kick %{white}player %{blue}%{name}",
votekick_vote = "%{red}>>> %{green}%{actor} %{white}voted to %{red}kick %{orange}%{victim}%{white}. Votes: %{blue}%{nb} %{white}of %{magenta}%{total}",

unban = "%{red}>>> %{blue}%{name} %{white}unbanned %{ip}, check with #banlist",
no_matching_ban ="%{red}>>> %{white}No matching ban found: %{blue}%{ip}",

specall_command = "%{red}>>> %{white}All players have been %{blue}spectated",
unspecall_command = "%{red}>>> %{white}All players have been %{blue}unspectated",

warning_warn = "%{red}>>> %{last} Warning %{blue}%{name} %{white}%{text}",
last = "Last",

recorddemo_start = "%{red}>>> %{white}Start demo recording",

camping = "%{red}>>> %{blue}%{name} %{white}is camping!",
camping_penalty_announce = "%{red}>>> %{white}Player %{blue}%{name} %{white}is camping and has a %{blue}penalty %{white}of %{blue}10 secs",
camping_penalty_countdown = "%{red}>>> %{white}You will %{blue}play %{white}in %{blue}%{time} secs",

mapsucks = "%{red}>>> %{white}You think this map %{orange}sucks%{white}, like %{blue}%{nb} %{white}other players",
mapsucks_timelowered = "%{red}>>> %{magenta}Time lowered to %{blue}%{time} %{magenta}minute%{ts}: %{orange}%{nb}%{blue}/%{total} %{white}player%{ps} think%{vs} this map sucks",
mapsucks_announce = "%{red}>>> %{blue}%{name} thinks that this maps sucks. If you don't like it either, type %{blue}#mapsucks",
mapsucks_analysetext = "%{red}>>> If you don't like this map, type %{magenta}#%{blue}mapsucks",

overtime = "%{red}>>> %{orange}One minute overtime!",
suddendeath = "%{red}>>> %{orange}Suddendeath - next score wins!",

killingspree = {
	[5] = "%{blue}%{name} %{white}is on a %{orange}KILLING SPREE%{white}!",
	[10] = "%{blue}%{name} %{white}is on a %{orange}RAMPAGE%{white}!",
	[15] = "%{blue}%{name} %{white}is on a %{orange}DOMINATING%{white}!",
	[20] = "%{blue}%{name} %{white}is on a %{orange}UNSTOPPABLE%{white}!",
	[30] = "%{blue}%{name} %{white}is on a %{orange}GODLIKE%{white}!"
},
killingspree_multiple = {
	[2] = "%{yellow}You scored a %{orange}DOUBLE KILL!!",
	[3] = "%{yellow}You scored a %{orange}TRIPLE KILL!!",
	multiple = "%{yellow}You scored %{orange}%{nb} MULTIPLE KILLS!!"
},
killingspree_firstkill = "%{blue}%{name} %{white}made the %{orange}FIRST KILL%{white}!",
killingspree_stopped = "%{yellow}%{victim}%{white} was stopped by %{orange}%{actor}!!",

remoteadmin = "%{red}>>> %{white}Remote Admin (%{admin}): %{blue}%{msg}",
server_info = "%{red}>>> %{white}Server Info: %{blue}%{msg}",

restart_warning = "%{red}>>> %{magenta}The server is set for a %{red}restart %{magenta}at the end of this game.",
restart_cancelled = "%{red}>>> %{magenta}Server restart cancelled.",

priv = "%{red}>>> %{white}PM from %{blue}%{name}%{white}: %{green}%{msg}",

cheater_spam = "%{red}>>> %{white}Don't %{blue}spam %{white}with the #cheater command or you will be %{blue}ignored",
cheater_thanks = "%{red}>>> %{white}Thank you for your report, hopefully an admin will check this out very soon.",
cheater_admin = "%{red}>>> %{blue}%{actor} %{white}reported %{orange}%{victim} %{white}as %{red}cheating.",

player_list = "%{red}>>> %{blue}Name %{white}%{name} %{blue}Country %{white}%{country} %{blue}Frags %{white}%{frags} %{blue}Deaths %{white}%{deaths} %{blue}Accuracy %{white}%{acc}%",

stats_current = "Current game stats for %{name}:",
stats_player = "%{red}>>> %{blue}Score %{white}%{score} %{blue}Frags %{white}%{frags} %{blue}Deaths %{white}%{deaths} %{blue}Accuracy %{white}%{acc}%",
stats_teamkills = "%{red}>>> %{blue}Teamkills%{white}: %{tk}",

help_command = "#%{command_name} %{help_parameters}: %{help}",
help_unknown_command = "unknown command",
help_command_disabled = "this command is disabled",
help_access_denied = "access denied",
help_no_description = "no description found for #%{command_name}",


unrecognized_gamemode = "%{orange}Unrecognized game mode",

--Mute/Unmute
player_mute = "%{red}>>> You have been muted",
player_mute_reason = "%{red}>>> You have been muted because: %{reason}",
player_block_chat = "%{red}>>> Your chat messages are being blocked.",
player_unmute = "%{red}>>> %{white}You have been unmuted.",
player_muted_already = "%{red}>>> %{white}Player %{name} is %{red}already muted",
player_not_muted = "%{red}>>> %{white}Player %{name} is %{blue} not muted",
player_mute_admin = "%{red}>>> %{blue}%{name} %{white}has been %{blue}muted",
player_unmute_admin = "%{red}>>> %{blue}%{name} %{white}has been %{blue}unmuted",
--Mutespecs/Unmutespecs
spectator_muted = "%{red}Spec> %{name}: %{green}%{msg}",
all_spectator_muted = "%{red}>>> %{white}All spectators muted!",
all_spectator_unmuted = "%{red}>>> %{white}All spectators unmuted!",
missing_mute_spectator_module = "%{red}>>> mute_spectators module not loaded!",

--Specmsg
specmsg = "%{red}Spec> %{name}%{white}(%{magenta}%{cn}%{white}): %{green}%{msg}",

--Mapvote
mapvote_disabled = "%{red}>>> Map voting is disabled.",
mapvote_disallowed_gamemode = "%{red}>>> Vote rejected: %{yellow}%{mode} %{red} is a disallowed game mode",
mapvote_rejected_unknownmap = "%{red}>>> Vote rejected: unknown map: %{yellow}%{map}",
mapvote_outside_maprotation = "%{red}>>> Vote rejected: %{yellow}%{map} %{red}is not in the %{yellow}%{mode} %{red}map rotation",

--Nextmap
nextmap = "%{red}>>> %{white}The next map is %{blue}%{map1} %{white}or %{blue}%{map2}",

--Auth
auth_unknown_domain =  "%{red}>>> auth request failed: unknown domain",

--Privileges
player_privileges_list = "%{red}>>> %{blue}Name %{white}%{name}(%{cn}) %{blue}Privileges %{white}%{priv}",
invadmin_activation = "%{red}>>> %{white}Set to %{gray}invisible %{orange}admin",
invmaster_activation = "%{red}>>> %{white}Set to %{gray}invisible %{green}master",

--Givemaster
givemaster = "%{red}>>> %{blue}%{name} %{white}has passed master privilege to you.",

--Noedit
noedit_enabled = "%{red}>>> %{white}editing %{red}disabled",
noedit_disabled = "%{red}>>> %{white}editing %{blue}enabled",

--Editemute
editmute_enabled = "%{red}>>> %{white}edit mute %{red}enabled",
editmute_disabled = "%{red}>>> %{white}edit mute %{blue}disabled",

--Nodamage
nodamage_enabled = "%{red}>>> %{white}damage %{red}disabled",
nodamage_disabled = "%{red}>>> %{white}damage %{blue}enabled",

--Names
names_command = "%{red}>>> %{white}Names used by %{current_name}: %{yellow}%{namelist}",

--Forgive
forgive_propose = "%{red}>>> %{white}Type %{magenta}#%{blue}forgive %{white}if you want to forgive %{blue}%{name}%{white}'s %{red}teamkill",
forgive_analysetext = "%{red}>>> %{white}Type %{magenta}#%{blue}forgive %{white}if you want to forgive a %{red}teamkill",
forgive_not_teamkilled = "You have not been teamkilled",
forgive_actor_forgiven = "%{red}>>> %{blue}%{name} %{white}has forgiven your %{red}teamkill",
forgive_target_forgiven = "%{red}>>> %{white}You have forgiven %{blue}%{name}%{white}'s %{red}teamkill",

--Rename
player_renamed = "%{red}>>> %{white}You have been renamed to %{blue}%{new_name} %{white}by %{orange}%{admin_name}",

--Persists
persist_enabled = "%{red}>>> %{white}reshuffle teams at mapchange %{blue}enabled",
persist_disabled = "%{red}>>> %{white}reshuffle teams at mapchange %{blue}disabled",

--Clanwar
clanwar_already_running = "%{red}>>> %{orange}Already running",
clanwar_teams_locked = "%{red}>>> %{orange}Clanwar running: teams are locked",
clanwar_waiting = "%{red}>>> %{orange}Waiting until all Players loaded the Map.",
clanwar_demorecord = "%{red}>>> %{orange}Starting Demo Recording",
clanwar_countdown = "%{red}>>> %{orange}Starting game in %{cdown} second%{s}",
clanwar_started_message = "%{red}>>> %{orange}Game started!",

--Resume
game_resume_sec = "%{red}>>> game will %{blue}resume %{white}in %{blue} %{cdown}sec%{s}",

--Versus
versus_samecn = "%{red}>>> player 1 and player 2 have the same CN.",
versus_invalidcn = "%{red}>>> Invalid CN given for the first or second argument.",
versus_win = "%{red}>>> %{white}1-on-1 Game %{blue}ended %{white}- %{blue}%{winner}won the game",
versus_disconnect = "%{red}>>> %{white}Opponent %{blue}%{name} %{red}disconnected}%{white}. Pausing Game.",
versus_announce = "%{red}>>> %{white}1-on-1 - %{blue}%{player1} %{white}against %{blue}%{player2} %{white}mode: %{blue}%{mode} %{white}map: %{blue}%{map}",
versus_countdown ="%{red}>>> Loading the map in %{blue}%{cdown} %{white}seconds",

--Namelock
player_namelock = "%{red}>>> WARNING: %{white}names are locked!",

--Clantag
clantag_rename = "%{red}>>> Server has renamed you to 'unnamed'.",

--Stats
stats_total_player = "%{red}>>> %{blue}Games %{white}%{games} %{blue}Frags %{white}%{frags} %{blue}Deaths %{white}%{deaths} %{blue}Kpd %{white}%{kpd} %{blue}Accuracy %{white}%{acc}%% %{blue}Win %{white}%{win} %{blue}Rank %{white}%{rank}",

--Balance
balance_disallow = "%{red}>>> %{white}Team change %{blue}disallowed%{white}: %{team} team has %{blue}enough %{white}players.",
balance_switched = "%{red}>>> %{white}You %{blue}switched teams %{white}for balance.",
balance_allowed_teams = "%{red}>>> %{blue}Only %{white}teams %{blue}good %{white}and %{blue}evil %{white}are %{blue}allowed.",
balancebot_disabled = "%{red}>>> Auto Team Balancing has been %{blue}disabled%{white}. It will be re-enabled once the bots have been removed and/or the %{blue}mastermode %{white}has been set to OPEN(%{blue}0%{white}).",

--Spectating
speclimit = "%{red}>>> %{white}Server disconnected you, because all slots are being used and you seem to be inactive as spectator.",

--Ping
pinglimit = "%{red}>>> Server moved you to spectators, because your ping is too high for fair games. Fix it and type '/spectator 0' to rejoin the game."

}
