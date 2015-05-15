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

exec("script/base/pcall.lua")
exec("script/base/core_function_overloads.lua")
exec("script/base/event.lua")
exec("script/base/server.lua")

server.is_authserver = true

exec("script/base/serverexec.lua")

exec("script/base/auth/server/config.lua")
exec("script/base/logging_base.lua")
exec("script/base/utils.lua")
exec("script/base/auth/server/core.lua")
exec("script/base/auth/server/db/init.lua")
exec("base/auth/server/web/init.lua")

exec_if_found("conf/authserver_conf.lua")
