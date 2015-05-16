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
command_internal_error = "%{red}>>> ERROR: %{white}Command syntax error: %{err}",

master_already = "%{red}>>> %{white}An %{orange}admin %{white}or %{green}master %{white}is already %{blue}here%{white}.",
setmaster_refused = "%{red}>>> %{orange}WARNING%{white}: no longer accepting your %{blue}setmaster %{white}requests",

demo_recording = "%{red}>>> %{white}Recording demo",

info_command = "%{red}>>> %{white}Serveur uptime : %{blue}%{uptime}%{white}. SuckerServ %{verstr}",
version = "%{white}rev%{blue}%{revision}%{white} compiled at %{green}%{version}%{white}",

help = "Command descriptions: #help <command>\n%{blue}List %{white}of %{green}commands%{white}",

stats_logged_in = "%{red}>>> %{white}You are logged in as %{blue}%{user_id}",

mapbattle_winner = "%{red}>>> %{white}Winner: %{blue}%{mapbattle_winner}",
mapbattle_vote = "%{red}>>> %{white}Vote for map %{blue}%{map1} %{white}or %{blue}%{map2} %{white}with %{green}1 %{white}or %{green}2",

client_nameprotect = "%{red}>>> %{white}You are %{yellow}logged in %{white}as %{blue}%{user_id}%{white}.",
nameprotect_rename = "%{red}>>> %{white}You have used a reserved name of another player. Server %{blue}renamed %{white}you to %{blue}'unnamed'%{white}."
}
