return function(cn)
  server.pausegame(true)
  server.msg(string.format(server.pause_message, server.player_name(cn)))
end
