--[[
	A player command to raise privilege to (invisble) master
]]

return function(cn, pw)
  local domains = table_unique(server["invmaster_domains"]) or table_unique(server["master_domains"])

  if not domains then
    server.log_error("invmaster command: no domains set")
    return
  end

  if server.player_priv_code(cn) > 0 then
    server.unsetpriv(cn)
  elseif pw then
    if server.check_admin_password(pw) then
      server.set_invisible_master(cn)
    end
  else
    local sid = server.player_sessionid(cn)

    for _, domain in pairs(domains) do
      auth.send_request(cn, domain, function(cn, user_id, domain, status)
        if sid == server.player_sessionid(cn) and status == auth.request_status.SUCCESS then
          server.set_invisible_master(cn)

          if not name then
              server.log(string.format("%s(%i) claimed (inv)master.", server.player_name(cn), cn))
          else
              server.log(string.format("%s playing as %s(%i) used auth to claim (inv)master.", name, server.player_name(cn), cn))
          end
        end
        
        set_invmaster(cn, user_id)
      end)
    end
  end
end

