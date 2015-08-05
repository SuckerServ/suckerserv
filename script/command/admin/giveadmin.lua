--[[

	A player command to raise player's privilege to admin

]]

local trigger_event
local id_event

local function init()
    trigger_event, id_event = server.create_event_signal("giveadmin-command")
end

local function unload()
    server.cancel_event_signal(id_event)
end

local usage = "#giveadmin <cn>"

local function run(cn, target)
    if not target then
        return false, usage
    end

    if not server.valid_cn(target) then
        return false, "CN is not valid"
    end

    server.unsetmaster()
    server.player_msg(target, "giveadmin", {name = server.player_displayname(cn) })
    server.admin_log(string.format("GIVEADMIN: %s gave admin to %s", server.player_displayname(cn), server.player_displayname(target)))
    server.setadmin(target)

    trigger_event(cn, target)
end

return {init = init,run = run,unload = unload}
