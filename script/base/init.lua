package.path = package.path .. ";script/package/?.lua;"
package.cpath = package.cpath .. ";lib/lib?.so"

dofile("script/base/pcall.lua")
dofile("script/base/core_function_overloads.lua")
dofile("script/base/event.lua")
dofile("script/base/server.lua")
dofile("script/base/cubescript.lua")
dofile("script/base/serverexec.lua")
-- The exec function becomes available after cubescript.lua has been executed

add_exec_search_path("conf")
add_exec_search_path("script")
add_exec_search_path("script/module")

exec("base/config.cs")
exec("base/config.lua")
exec("base/utils.lua")
exec("base/module.lua")
exec("base/logging.lua")
exec("base/restart.lua")
exec("base/player/utils.lua")
exec("base/player/vars.lua")
exec("base/player/object.lua")
exec("base/player/iterators.lua")
exec("base/player/private_vars.lua")
exec("base/player/command.lua")
exec("base/team/utils.lua")
exec("base/setmaster.lua")
exec("base/kickban.lua")
exec("base/server_message.lua")
exec("base/static_items.lua")

server.module("base/maprotation/init.lua")
server.module("base/normalize")
server.module("base/modified_map")
server.module("base/banner")
server.module("base/mute")
server.module("base/auth/init")
server.module("base/mapvote")
server.module("base/register_server")
server.module("base/web/init")
server.module("base/teamkills")
server.module("base/global_bans")

server.event_handler("started", function()
    
    server.reload_maprotation()
    
    require("geoip").load_geoip_database(server.geoip_db_file)
    
    server.log_status(server.server_start_message)
end)

server.event_handler("shutdown", function() 
    server.log_status("Server shutting down.")
end)

exec_if_found("conf/server.conf")
exec("base/saveconf.lua")

