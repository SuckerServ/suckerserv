local _ = require "underscore"

function format_filesize(bytes)
    bytes = tonumber(bytes)
    if bytes < 1024 then return bytes .. "B"
    elseif bytes < (1024*1024) then return round(bytes/1024) .. "KB"
    else return round(bytes/(1024*1024)) .. "MB"
    end
end

function format_duration(seconds)
    local hours = math.floor(seconds / 3600)
    seconds = seconds - (hours * 3600)
    local mins = math.floor(seconds / 60)
    seconds = seconds - (mins * 60)
    return string.format("%02i:%02i:%02i",hours,mins,seconds)
end

server.format_duration = format_duration

function tabulate(text)
    
    local output = ""
    local cols = {}
    local rows = _.to_array(string.gmatch(text, "[^\n]+"))
    
    for i in ipairs(rows) do
        
        rows[i] = _.to_array(string.gmatch(rows[i], "[%w%p/*]+"))
        
        for i,col in ipairs(rows[i]) do
            cols[i] = math.max(#col, cols[i] or 0)
        end
    end
    
    for i,row in ipairs(rows) do
    
        for i2, col in ipairs(row) do
            output = output .. col .. string.rep(" ",(cols[i2] - #col)+1)
        end
        
        output = output .. "\n"
    end
    
    return output
end

function print_list(...)
    local output = ""
    for _, item in ipairs(arg) do
        item = tostring(item)
        if #item > 0 then
            if #output > 0 then output = output .. ", " end
            output = output .. item
        end
    end
    return output
end

do
    local Chars = {}
    for Loop = 0, 255 do
       Chars[Loop+1] = string.char(Loop)
    end
    
    local String = table.concat(Chars)
    local Built = {['.'] = Chars}

    local AddLookup = function(CharSet)
       local Substitute = string.gsub(String, '[^'..CharSet..']', '')
       local Lookup = {}
       for Loop = 1, string.len(Substitute) do
           Lookup[Loop] = string.sub(Substitute, Loop, Loop)
       end
       Built[CharSet] = Lookup

       return Lookup
    end

    function string.random(Length, CharSet)
       -- Length (number)
       -- CharSet (string, optional); e.g. %l%d for lower case letters and digits
       local CharSet = CharSet or '.'
       if CharSet == '' then
          return ''
       else
          local Result = {}
          local Lookup = Built[CharSet] or AddLookup(CharSet)
          local Range = table.getn(Lookup)
          math.randomseed( os.time() )
          for Loop = 1,Length do

             Result[Loop] = Lookup[math.random(1, Range)]
          end

          return table.concat(Result)
       end
    end
end

function strSplit(str, delim, maxNb)
    -- Eliminate bad cases...
    if string.find(str, delim) == nil then
        return { str }
    end
    if maxNb == nil or maxNb < 1 then
        maxNb = 0    -- No limit
    end
    local result = {}
    local pat = "(.-)" .. delim .. "()"
    local nb = 0
    local lastPos
    for part, pos in string.gfind(str, pat) do
        nb = nb + 1
        result[nb] = part
        lastPos = pos
        if nb == maxNb then break end
    end
    -- Handle the last field
    if nb ~= maxNb then
        result[nb + 1] = string.sub(str, lastPos)
    end
    return result
end

