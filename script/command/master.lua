--[[

  A player command to raise privilege to master

]]

local trigger_event
local id_event

local function init()
    trigger_event, id_event = server.create_event_signal("master-command")
end

local function unload()
    server.cancel_event_signal(id_event)
end

local function run(cn)
  local domains = table_unique(server.parse_list(server["master_domains"]))

  if not domains then
    server.log_error("master command: no domains set")
    return
  end

  local sid = server.player_sessionid(cn)

  for _, domain in pairs(domains) do
    auth.send_request(cn, domain, function(cn, user_id, domain, status)
      if sid == server.player_sessionid(cn) and status == auth.request_status.SUCCESS then
        server.setmaster(cn)

        server.msg(string.format(server.claimmaster_message, server.player_displayname(cn), user_id))
        server.log(string.format("%s playing as %s(%i) used auth to claim master.", user_id, server.player_name(cn), cn))
        server.admin_log(string.format("%s playing as %s(%i) used auth to claim master.", user_id, server.player_name(cn), cn))

        trigger_event(cn, user_id)
      end
    end)
  end
end

return {init = init,run = run,unload = unload}
