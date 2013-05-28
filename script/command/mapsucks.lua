--[[

  A player command to lower time when mapsucks ratio is reached
  By piernov <piernov@piernov.org>

  May 25 2013 (gear4): shortened one-line "if"'s, localised and shortened local statements, optimised (?) matches for "sucks" etc., 

]]

return function(cn)
  if server.player_status_code(cn) ~= server.SPECTATOR then
    server.mapsucks.vote(cn)
  else
    return false, server.mapbattle_cant_vote_message
  end
end
