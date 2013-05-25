local resuming = false

return function(cn, countdown)
  if not server.paused then
    server.player_msg(cn, "Game is not paused!")
    return
  elseif resuming then
    server.player_msg(cn, "Game is already being resumed!")
    return
  end

  if not countdown then
    server.pausegame(false)
  else
    local cdown = tonumber(countdown)

    if cdown < 1 then
      server.pausegame(false)
    else
      cdown = round(cdown, 0)

      server.interval(1000, function()
        if cdown == 0 then
          server.pausegame(false)
          return -1
        else
          server.msg(string.format(server.game_resume_sec, cdown, (cdown == 1) and "" or "s"))
          cdown = cdown - 1
        end
      end)
    end
  end
end
