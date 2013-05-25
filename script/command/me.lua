--[[
    Say what you are doing actually 
    By LoveForever
    escape function added by killme_nl
]]
local function cs_escape(msg)
  --function to escape newlines and message coloring
  msg = string.gsub(msg, "\f", "f")
  msg = string.gsub(msg, "^f", "f")
  msg = string.gsub(msg, "\n", "n")
  msg = string.gsub(msg, "^n", "n")
  return msg
end

return function(cn, ...)
  local text = table.concat({...}, " ")
  text = cs_escape(text)

  server.msg(string.format(server.me_message, server.player_displayname(cn), text))
end

