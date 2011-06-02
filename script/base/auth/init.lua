script("base/auth/core.lua")
script("base/auth/masterauth.lua")

local auth_conf = server.find_script("conf/auth")
if auth_conf then
    script(auth_conf)
end
