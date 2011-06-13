
local library = require("cubescript_library")

-- Give the cubescript environment direct access to the core table
setmetatable(library, {
    __index = function(_, value)
        return core[value] or core.vars[value] or _G[value]
    end
})

find_script = library.find_script
exec = library.exec
pexec = library.pexec
exec_if_found = library.exec_if_found
search_paths = library.exec_search_paths
exec_handler = library.exec_type

function add_exec_search_path(path)
    search_paths[#search_paths + 1] = path
end

exec_handler["vars"] = library.exec_cubescript


function cubescript.eval_string(code)
    return cubescript.eval(code .. "\n", library)
end

cubescript.library = library

--[[ Backwards compatibility stuff (deprecated) ]]

core.parse_list = library.parse_array

execIfFound = exec_if_found

library.global = function(name, value)
    local property_value = value
    core.vars[name] = function(value)
        if value then
            property_value = value
            event_listener.trigger_event("varchanged", name)
            return value
        else
            return property_value
        end
    end
end

library.interval = function(time, func)
    if type(func) == "string" then
        func = library.func("", func)
    end
    return server.interval(time, func) 
end

library.sleep = function(time, func)
    if type(func) == "string" then
        func = library.func("", func)
    end
    return server.sleep(time, func)
end

