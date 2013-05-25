
return function(cn,tcn)
  if not tcn then
    for p in server.gclients() do
      if p:vars().reserved_name then
        server.player_msg(cn, green(p:name()) .. " (" .. p.cn .. "): " .. yellow(p:vars().reserved_name))
      elseif p:vars().stats_auth_name then
        server.player_msg(cn, green(p:name()) .. " (" .. p.cn .. "): " .. yellow(p:vars().stats_auth_name))
      end
    end
  else
    if not server.valid_cn(tcn) then
      tcn = server.name_to_cn_list_matches(cn,tcn)

      if not tcn then
        return
      end
    end

    if server.player_vars(tcn).reserved_name then
      server.player_msg(cn, green(server.player_name(tcn)) .. " (" .. tcn .. "): " .. yellow(server.player_vars(tcn).reserved_name))
    elseif server.player_vars(tcn).stats_auth_name then
      server.player_msg(cn, green(server.player_name(tcn)) .. " (" .. tcn .. "): " .. yellow(server.player_vars(tcn).stats_auth_name))
    end
  end

end
