package.path = package.path .. ";script/package/?.lua;"
package.cpath = package.cpath .. ";lib/lib?.so"

local fsutils = require "filesystem_utils"

add_exec_search_path = fsutils.add_exec_search_path
find_script = fsutils.find_script
exec = fsutils.exec
pexec = fsutils.pexec
exec_if_found = fsutils.exec_if_found
eval_lua = fsutils.eval_lua

add_exec_search_path("conf")
add_exec_search_path("script")
add_exec_search_path("script/module")

exec("script/base/pcall.lua")
exec("script/base/core_function_overloads.lua")
exec("script/base/event.lua")
exec("script/base/server.lua")
exec("script/base/serverexec.lua")


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
exec("base/cheat_detection.lua")

server.module("base/mmdb")
server.module("base/maprotation/init.lua")
server.module("base/normalize")
server.module("base/modified_map")
server.module("base/banner")
server.module("base/mute")
server.module("base/auth/init")
server.module("base/mapvote")
server.module("base/register_server")
server.module("base/web/init")
server.module("base/global_bans")
server.module("base/messages")

exec_if_found("conf/server_conf.lua")
exec("base/saveconf.lua")

server.event_handler("started", function()
    server.reload_maprotation()

    server.log_status(messages[messages.languages.default].server_start)
end)

server.event_handler("shutdown", function()
    server.log_status("Server shutting down.")
end)

