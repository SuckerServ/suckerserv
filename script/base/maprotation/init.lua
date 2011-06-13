if server.use_server_map_rotation == 0 then
    return
end

dofile("script/base/maprotation/supported_maps.lua")
dofile("script/base/maprotation/core.lua")

local implementations = {
    standard = "script/base/maprotation/implementation/standard.lua",
    random   = "script/base/maprotation/implementation/random.lua",
    size     = "script/base/maprotation/implementation/size.lua"
}

local implementation_script = implementations[server.map_rotation_type]

if not implementation_script then
    server.log_error("Unknown value set for map_rotation_type")
    return
end

map_rotation.set_implementation(dofile(implementation_script))
map_rotation.reload()

