--[[
	MMDB Initialisation
]]--

local mmdb = require("mmdb")
server.mmdatabase = mmdb.load_mmdb_database(server.mmdb_file)

return {unload = function()
    server.mmdatabase:close()
    server.mmdatabase = nil
end}
