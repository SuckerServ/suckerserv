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

function formatcol(col, text)
    if text then return "\fs\f" .. col .. text .. "\fr" else return "\f" ..col end
end

function red(text) return formatcol(3,text) end
function orange(text) return formatcol(6, text) end
function green(text) return formatcol(0, text) end
function white(text) return formatcol(9, text) end
function yellow(text) return formatcol(2, text) end
function magenta(text) return formatcol(5, text) end
function blue(text) return formatcol(1, text) end

function string.split(s, pattern)
    local a = {}
    for x in string.gmatch(s, pattern) do
        a[#a+1] = x
    end
    return a
end

function tabulate(text)
    
    local output = ""
    local cols = {}
    local rows = text:split("[^\n]+")
    
    for i in ipairs(rows) do
        
        rows[i] = string.split(rows[i], "[%w%p/*]+")
        
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


-- Lua 5.1+ base64 v3.0 (c) 2009 by Alex Kloss <alexthkloss@web.de>
-- licensed under the terms of the LGPL2
-- character table string
local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'

-- encoding
function base64_encode(data)
    return ((data:gsub('.', function(x) 
        local r,b='',x:byte()
        for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
        return r;
    end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
        if (#x < 6) then return '' end
        local c=0
        for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
        return b:sub(c+1,c+1)
    end)..({ '', '==', '=' })[#data%3+1])
end

-- decoding
function base64_decode(data)
    data = string.gsub(data, '[^'..b..'=]', '')
    return (data:gsub('.', function(x)
        if (x == '=') then return '' end
        local r,f='',(b:find(x)-1)
        for i=6,1,-1 do r=r..(f%2^i-f%2^(i-1)>0 and '1' or '0') end
        return r;
    end):gsub('%d%d%d?%d?%d?%d?%d?%d?', function(x)
        if (#x ~= 8) then return '' end
        local c=0
        for i=1,8 do c=c+(x:sub(i,i)=='1' and 2^(8-i) or 0) end
        return string.char(c)
    end))
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

function strJoin(delimiter, list)
  local string = list[1]
  if #list >= 2 then
	  for i = 2, #list do 
		string = string .. delimiter .. list[i] 
	  end
  end
  return string
end