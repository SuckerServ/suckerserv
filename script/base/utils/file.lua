function server.read_whole_file(filename)

    local file, err = io.open(filename)
    if not file then
        server.log_error(err)
        return
    end
    
    return file:read("*a")
end
