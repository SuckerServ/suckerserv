function server.read_whole_file(filename)

    local file, err = io.open(filename)
    if not file then
        server.log_error(err)
        return
    end
    
    local whole = file:read("*a")
    io.close(file)
    return whole
end
