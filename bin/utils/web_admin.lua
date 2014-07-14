local crypto = require "crypto"

local function generateSalt()
    math.randomseed(os.time())
    local chars = {}
    for _ = 1, 15 do 
        chars[#chars + 1] = math.random(97,122)
    end
    return string.char(table.unpack(chars))
end

local username = arg[1]
local password = arg[2]
local salt = generateSalt()

print(string.format("\"%s %s %s\"", username, crypto.tigersum(salt .. password), salt))

