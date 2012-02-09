local SAVECONF_FILENAME = "conf/saved.conf.lua"

if server.file_exists(SAVECONF_FILENAME) then
    dofile(SAVECONF_FILENAME)
end

function server.saveconf()
    
    local file = io.open(SAVECONF_FILENAME, "w+")
    
    local varnames = server.parse_list(server.saveconf_vars)
    
    for _, varname in ipairs(varnames) do
         file:write(string.format("server.%s = \"%s\"\n", varname, tostring(server[varname])))
    end
    
    file:close()
    return true
end

function server.saveconf_get_conf()

    local output = {}    
    local varnames = server.parse_list(server.saveconf_vars)
    
    for _, varname in ipairs(varnames) do
         output[varname] = server[varname]
    end
    
    return output
end

