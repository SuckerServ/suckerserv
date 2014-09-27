return {
server_start_message = "-> Démarrage de Suckerserv réussi",

client_connect = "(%{green}%{time}%{white}) Connecté de %{yellow}%{country}%{white} : %{blue}%{name} %{magenta}(%{cn}) (%{priv})",
client_connect_admin = "IP: %{magenta}%{ip}",
client_disconnect = "(%{green}%{time}%{white}) Déconnecté : %{blue}%{name}%{white} %{magenta}(%{cn})%{white}",
client_disconnect_admin = " (IP: %{magenta}%{ip})",

connect_info = "%{red}>>> %{yellow}Écrivez %{magenta}#%{white}help pour obtenir la %{blue}liste %{white}des %{green}commandes",

client_crcfail_player = "%{red}>>> %{blue}Vous %{white}utilisez une carte modifiée !",
client_crcfail = "%{red}>>> %{white}Le joueur %{blue}%{name} %{white}utilise une carte modifiée !",

clearbans = "%{red}>>> %{white}Suppression de tous les %{blue}bannissements.",

stats_disabled = "%{red}>>> %{orange}L'enregistrement des statistiques est désactivé pour cette partie",
stats_enabled = "%{red}>>> %{green}Statistiques activées.",
 
stats_reload_disabled = "%{red}>>> L'enregistrement des statistiques a été désactivé pour cette partie",
}
