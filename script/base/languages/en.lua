return {
server_start_message = "-> Successfully loaded Suckerserv",

client_connect = "(%{green}%{time}%{white}) Connection from %{yellow}%{country}%{white}: %{blue}%{name} %{magenta}(%{cn}) (%{priv})",
client_connect_admin = "IP: %{magenta}%{ip}",
client_disconnect = "(%{green}%{time}%{white}) Disconnected: %{blue}%{name}%{white} %{magenta}(%{cn})%{white}",
client_disconnect_admin = " (IP: %{magenta}%{ip})",

connect_info = "%{red}>>> %{yellow}Type %{magenta}#%{white}help for a %{blue}list %{white}of %{green}commands",

client_crcfail_player = "%{red}>>> %{blue}You %{white}are using a modified map!",
client_crcfail = "%{red}>>> %{white}Player %{blue}%{name} %{white}is using a modified map!",

clearbans = "%{red}>>> %{white}Cleared all %{blue}bans",

stats_disabled = "%{red}>>> %{orange}Stats are disabled for this match",
stats_enabled = "%{red}>>> %{green}Stats enabled",

stats_reload_disabled = "%{red}>>> Sorry, stats have been disabled for this match",

}
