function supported_map(map)
    local f = io.open(string.format("mapinfo/%s.ents", map), "r")
    if f ~= nil then 
        io.close(f) return true 
    else 
        return false
    end
end