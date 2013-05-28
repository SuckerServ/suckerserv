--[[

  A player command to raise privilege to master

]]

return function(cn)
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
      end
    end)
  end
end
