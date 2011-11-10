--[[ 
    IP-Functions
    
    (c) 2011 by Thomas
--]]

function check_ip(ip)
    local i = 1
    local octets = 1
    local octet = ""
    local err = ""
    local errors = 0
    local blocks = { }
    while (i <= string.len(ip) + 1) do
        local c = string.sub(ip, i, i)
        if c == "." or string.len(ip)+1 == i then
            octets = octets + 1
            local block = tonumber(octet)
            if block == nil or block > 255 or block < 0 or octets > 5 then 
                errors = errors + 1
                err = err .. " \f3>" .. (block or "?") .. "<\f8" 
            else
                blocks[#blocks + 1] = octet
                err = err .. "\f0 " .. block .. "\f8"
            end
            octet = ""
        else
            octet = octet .. c
        end
        i = i + 1
    end
    
    if errors > 0 then
        return { err }
    end
    
    return { 0, blocks }
end

function ip2long(ip_addr)
    local blocks = check_ip(ip_addr)[2] or error("Invalid IP-Address")
    return (blocks[1] << 24) | (blocks[2] << 16) | (blocks[3] << 8 ) | blocks[4]
end

local function us2signed(num)
    return _if(num < 0, 256 + num, num)
end

function long2ip(addr)
    if not addr or type(addr) ~= "number" then error("Invalid Long-IP-Address") end
    local a = (addr & (0xff << 24)) >> 24
    local b = (addr & (0xff << 16)) >> 16
    local c = (addr & (0xff << 8)) >> 8
    local d = addr & 0xff
    return string.format("%i.%i.%i.%i", us2signed(a), us2signed(b), us2signed(c), us2signed(d))
end

function ip_inrange(ip1, ip2)
    local ip2 = strsplit("/", ip2)
    local mask = math.pow(2, 32 - (tonumber(ip2[2]) or 32)) - 1
    return ip2long(ip1) & ~mask == ip2long(ip2[1]) & ~mask
end
-- examples: 
--  ip_inrange("192.168.63.255", "192.168.0.0/18") --> true
--  ip_inrange("192.168.64.0", "192.168.0.0/18")   --> false