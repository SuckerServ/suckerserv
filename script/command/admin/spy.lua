-- (C) 2011 by Thomas

return function(cn, enter)
    
    if enter == nil or enter == "on" then
        enter = 1 
    end
    
    server.setspy(cn, enter == 1)
    
end, "[1|0]", "Enter or leave spy mode"