return {
server_start_message = "--> Successfully loaded Suckerserv.",

client_connect = "(%{green}%{time}%{white}) Connection from %{yellow}%{country}%{white}: %{blue}%{name} %{magenta}(%{cn}) %{white}(%{priv}%{white})",
client_connect_admin = "%{white}IP: %{blue}%{ip}%{white}",
client_disconnect = "(%{green}%{time}%{white}) Disconnected: %{blue}%{name}%{white} %{magenta}(%{cn})",
client_disconnect_admin = " %{white}(IP: %{blue}%{ip}%{white})",

connect_info = "%{red}>>> %{white}Type %{magenta}#%{white}help to %{yellow}get %{white}the %{blue}list %{white}of %{green}commands%{white}.",

client_crcfail_player = "%{red}>>> %{blue}You %{yellow}are using %{white}a modified %{green}map!",
client_crcfail = "%{red}>>> %{white}Player %{blue}%{name} %{yellow}is using %{white}a modified %{green}map!",

clearbans = "%{red}>>> %{green}Cleared all %{blue}bans.",

awards_stats = "%{red}>>> %{white}Awards: %{stats_message}",
awards_flags = "%{red}>>> %{white}Best Flags: %{flagstats_message}",

inactivitylimit = "%{red}>>> %{white}Server moved you to spectators, because you seem to be inactive - type '/spectator 0' to rejoin the game.",

command_disabled = "%{red}>>> ERROR: %{white}Command disabled",
command_permission_denied = "%{red}>>> ERROR: %{white}Permission Denied",
command_internal_error = "%{red}>>> ERROR: %{white}Internal error",
command_syntax_error = "%{red}>>> ERROR: %{white}Command syntax error: %{err}",

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

unban_message = "%{red}>>> %{blue}%{name} %{white}unbanned %{ip}, check with #banlist",
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
killingspree_firstkill = "%{blue}%%{name} %{white}made the %{orange}FIRST KILL%{white}!",
killingspree_stopped = "%{yellow}%{victim}%{white} was stopped by %{orange}%{actor}!!",

remoteadmin = "%{red}>>> %{white}Remote Admin (%{admin}): %{blue}%{msg}",
server_info = "%{red}>>> %{white}Server Info: %{blue}%{msg}",

restart_warning = "%{red}>>> %{magenta}The server is set for a %{red}restart %{magenta}at the end of this game.",
restart_cancelled = "%{red}>>> %{magenta}Server restart cancelled.",

priv_message = "%{red}>>> %{white}PM from %{blue}%{name}%{white}: %{green}%{msg}",

cheater_spam = "%{red}>>> %{white}Don't %{blue}spam %{white}with the #cheater command or you will be %{blue}ignored",
cheater_thanks = "%{red}>>> %{white}Thank you for your report, hopefully an admin will check this out very soon.",
cheater_admin = "%{red}>>> %{blue}%{actor} %{white}reported %{orange}%{victim} %{white}as %{red}cheating.",

player_list = "%{red}>>> %{blue}Name %{white}%{name} %{blue}City %{white}%{city} %{blue}Country %{white}%{country} %{blue}Frags %{white}%{frags} %{blue}Deaths %{white}%{deaths} %{blue}Accuracy %{white}%{acc}%%",

stats_current = "Current game stats for %{name}:",
stats_player = "%{red}>>> %{blue}Score %{white}%{score} %{blue}Frags %{white}%{frags} %{blue}Deaths %{white}%{deaths} %{blue}Accuracy %{white}%{acc}%%",
stats_teamkills = "%{red}>>> %{blue}Teamkills%{white}: %{tk}"


}
