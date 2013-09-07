local io = io
local string = string
local table = table
local pairs = pairs
local setmetatable = setmetatable
local load = load
local type = type
local pcall = pcall
local print = print
local tostring = tostring

local _M = {}

local READ_SIZE = 8192

local journal_writer_class
local journal_reader_class
local format_string, format_array, format_table, format_value
local reader_open, write, close, load_journal

function journal_open(filename, mode, class)
    local object = {}
    local error_message
    object._file, error_message = io.open(filename, mode)
    if not object._file then
        return nil, error_message
    end
    object._file:setvbuf("no")
    setmetatable(object, {__index = class})
    return object
end

function reader_open(filename)
    return journal_open(filename, "r", journal_reader_class)
end

function writer_open(filename)

    local journal = journal_open(filename, "a+", journal_writer_class)
    
    local file_size = journal._file:seek("end")
    if file_size == 0 then
        journal._file:write("return {")
    end
    
    return journal
end

function write(object, ...)
    object._file:write(format_array({...}) .. ",\n")
end

function format_string(value)
    return string.format("%q", value)
end

function format_array(values)
    local formatted_values = {}
    for index = 1, #values do
        formatted_values[#formatted_values + 1] = format_value(values[index])
    end
    return "{" .. table.concat(formatted_values, ",") .. "}"
end

function format_table(values)
    if #values > 0 then
        return format_array(values)
    end
    local formatted_fields = {}
    for key, value in pairs(values) do
        formatted_fields[#formatted_fields + 1] = "[" .. format_value(key) .. "]=" .. format_value(value)
    end
    return "{" .. table.concat(formatted_fields, ",") .. "}"
end

function format_value(value)
    
    local format_functions = {
        ["nil"] = function() return "nil" end,
        ["boolean"] = tostring,
        ["number"] = tostring,
        ["string"] = format_string,
        ["table"] = format_table
    }
    
    local format_function = format_functions[type(value)]
    if not format_function then
        return nil
    end
    
    return format_function(value)
end

function close(object)
    object._file:close()
    object._file = nil
end

journal_writer_class = {
    write = write,
    close = close
}

function load_journal(filename)
    local journal, error_message = reader_open(filename)
    if not journal then
        return nil, error_message
    end
    local file = journal._file
    local chunk, error_message = load(function()
        if not file then 
            return nil
        end
        local data = file:read(READ_SIZE)
        if data then
            return data
        else
            file:close()
            file = nil
            return "}"
        end
    end, string.format("journal:%s", filename))
    if not chunk then
        return nil, "journal is corrupted"
    end
    local pcall_success, data = pcall(chunk)
    if not pcall_success then
        return nil, string.format("unable to load journal: %s", data)
    end
    return data
end

_M.load = load_journal
_M.writer_open = writer_open

return _M
