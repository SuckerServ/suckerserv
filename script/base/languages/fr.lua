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

awards_stats = "%{red}>>> %{white}Awards : %{stats_message}",
awards_flags = "%{red}>>> %{white}Best Flags : %{flagstats_message}",

inactivitylimit = "%{red}>>> %{white}Server moved you to spectators, because you seem to be inactive - type '/spectator 0' to rejoin the game.",

command_disabled = "%{red}>>> ERREUR: %{white}Commande désactivée",
command_permission_denied = "%{red}>>> ERREUR: %{white}Permission refusée",

master_already = "%{red}>>> Un admin ou un master est déjà présent.", 

demo_recording = "%{red}>>> %{white}Enregistrement de la partie",

uptime = "%{red}>>> %{white}Serveur démarré depuis : %{blue}%{uptime}",

help = "Description d'une commande : #help <command>\n%{blue}Liste %{white}des %{green}commandes%{white}",

stats_logged_in = "%{red}>>> %{white}Vous êtes connecté en tant que %{blue}%{user_id}",

mapbattle_winner = "%{red}>>> %{white}Gagnante : %{blue}%{mapbattle_winner}",
mapbattle_vote = "%{red}>>> %{white}Votez pour la carte %{blue}%{map1} %{white}ou %{blue}%{map2} %{white}en écrivant %{green}1 %{white}ou %{green}2",

}
