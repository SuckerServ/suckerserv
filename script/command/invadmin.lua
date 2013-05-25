--[[
	A player command to raise privilege to (invisble) admin
]]

return function(cn, pw)
  local domains = table_unique(server.parse_list(server["invadmin_domains"])) or table_unique(server.parse_list(server["admin_domains"]))

  if not domains then
    server.log_error("invadmin command: no domains set.")
    return
  end

  if server.player_priv_code(cn) > 0 then
    server.unsetpriv(cn)
  elseif pw then
    if server.check_admin_password(pw) then
      server.set_invisible_admin(cn)
    end
  else
    local sid = server.player_sessionid(cn)
    for _, domain in pairs(domains) do
      auth.send_request(cn, domain, function(cn, user_id, domain, status)
        if sid == server.player_sessionid(cn) or status == auth.request_status.SUCCESS then
          server.set_invisible_admin(cn)
  
          if not name then
            server.log(string.format("%s(%i) claimed (inv)admin.", server.player_name(cn), cn))
          else
            server.log(string.format("%s playing as %s(%i) used auth to claim (inv)admin.", name, server.player_name(cn), cn))
          end
        end
      end)
    end
  end
end

