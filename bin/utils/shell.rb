#!/usr/bin/env ruby

require 'net/http'
require 'readline'

def execute_command(http, command)
    
    req = Net::HTTP::Post.new("/serverexec")
    req.content_type=("text/x-cubescript")
    req.content_length = command.length()
    req.body = command
    
    res = http.request(req)
    
    return res.read_body
end

def reconnect(hostname, port, command)
    
    sleep 1
    puts "Attempting to reconnect..."
    
    if command == "restart" || command == "restart_now" || command == "reloadscripts"
        command = nil
    end
    
    start_shell(hostname, port, command)
end

def start_shell(hostname, port, command)
    
    Net::HTTP.version_1_2
    Net::HTTP.start(hostname, port){|http|
        
        puts "Connected to #{hostname}:#{port}"
        
        label = execute_command(http, "try [get shell_label] [result \"server\"]")
        
        if command
            puts "Sending command..."
            puts execute_command(http, command)
        end
        
        while true
            
            command = Readline::readline("#{label}> ")
            if command.nil?
                puts "\n"
                exit
            end
            
            if command.length > 0
                Readline::HISTORY.push(command)
                puts execute_command(http, command)
            end
            
        end
    }

rescue EOFError
    puts "Error: lost connection with remote host."
    reconnect(hostname, port, command)
    
rescue Errno::ECONNREFUSED
    puts "Error: connection refused."
    
rescue Errno::ENETUNREACH
    puts "Error: #{hostname} is unreachable."
    
rescue Errno::EINVAL
    puts "Error: invalid argument"

rescue Errno::EPIPE
    puts "Error: server has shutdown."
    reconnect(hostname, port, command)

rescue Errno::ECONNRESET
    puts "Error: connection reset."
    reconnect(hostname, port, command)
    
rescue IOError => e
    puts "Error: #{$!}"

rescue Interrupt
    puts ""
    
end

DEFAULT_PORT = 28788
portfile = nil

if File.readable? "log/sauer_server_http.port"
    f = File.open("log/sauer_server_http.port")
    portfile = Integer(f.readline()) rescue nil
end

start_shell(ARGV[0] || "127.0.0.1", ARGV[1] || portfile || DEFAULT_PORT, nil)
