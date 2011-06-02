
-- Copied from http://lua-users.org/wiki/TableUtils
-- Count the number of times a value occurs in a table 
function table_count(tt, item)
    local count
    count = 0
    for _,xx in pairs(tt) do
        if item == xx then count = count + 1 end
    end
    return count
end

function empty(table)
    local isEmpty = true
    for _ in pairs(table) do 
        isEmpty = false 
        return
    end
    return isEmpty
end

-- Copied from http://lua-users.org/wiki/TableUtils
-- Remove duplicates from a table array (doesn't currently work
-- on key-value tables)
function table_unique(tt)
    local newtable
    newtable = {}
    for _,xx in ipairs(tt) do
        if(table_count(newtable, xx) == 0) then
            newtable[#newtable+1] = xx
        end
    end
    return newtable
end

-- Copied from the lua-users wiki
function table.deepcopy(object)
    local lookup_table = {}
    local function _copy(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end
        local new_table = {}
        lookup_table[object] = new_table
        for index, value in pairs(object) do
            new_table[_copy(index)] = _copy(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return _copy(object)
end

function map_to_array(map)
    local result = {}
    for _,v in pairs(map) do
        table.insert(result, v)
    end
    return result
end

function list_to_set(list)
    local set = {}
    for _,value in ipairs(list) do set[value] = true end
    return set
end

function validate_table(subject_table, schema)
    
    local function check_type(object, typename)
        if type(object) ~= typename then error("expecting " .. typename) end
    end
    
    check_type(subject_table, "table")
    
    for _, element in ipairs(schema) do
        
        local id = element[1]
        local typeinfo = element[2]
        
        if not id then error("error in table schema") end
        
        local lookup = subject_table[id]
        
        if lookup == nil then
            error("missing " .. id)
        end
        
        if typeinfo then
            if type(typeinfo) == "string" then
                check_type(lookup, typeinfo)
            elseif type(typeinfo) == "table" then
                validate_table(lookup, typeinfo)
            else
                error("error in table schema")
            end
        end
    end
end

function read_only(table)
    local proxy = {}
    setmetatable(proxy, {
        __index = function(_, key)
            return table[key]
        end,
        __newindex = function()
            error("read-only")
        end
    })
    return proxy
end

function table.wrapped_index(t, index, offset)
    return t[index % #t + (offset or 0)]
end

function table.has_fields(t, ...)
    for index in ipairs(arg) do
        if not t[arg[index]] then
            print("not have " .. arg[index])
            return false
        end
    end
    return true
end

function table.missing_fields(t, ...)
    table.remove(arg)
    local missing = {}
    for index in ipairs(arg) do
        if not t[arg[index]] then
            missing[#missing + 1] = name
        end        
    end
    return missing
end

