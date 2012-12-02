
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

function table_size(t)
  local max = 0
  for k,v in pairs(t) do
    max = max + 1
  end
  return max
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

function list_to_set(list)
    local set = {}
    for _,value in ipairs(list) do set[value] = true end
    return set
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

function table.missing_fields(t, ...)
    table.remove(arg)
    local missing = {}
    for index in ipairs({...}) do
        if not t[arg[index]] then
            missing[#missing + 1] = name
        end        
    end
    return missing
end

function table_append(subject, extra_elements)
    for i = 1, table.maxn(extra_elements) do
        subject[#subject + 1] = extra_elements[i]
    end
end

