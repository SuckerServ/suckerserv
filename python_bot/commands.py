server = {
    'ping':     lambda server_write, **_: server_write('code:sendmsg("pong!")'),
	'players': lambda server_write, **_: server_write('\
		      code:local msg = ""\
		      for i, cn in ipairs(server.clients()) do\
			  msg = msg .. server.player_name(cn) .. " "\
		      end\
		      if msg == "" then\
			  sendmsg("no players connected")\
		      else\
			  sendmsg(msg)\
		      end\
	'),
	'restart': lambda server_write, **_: server_write('code:sendmsg("restarting server..."); server.restart_now()'),
	'uptime': lambda server_write, **_: server_write('code:sendmsg("Uptime: " .. server.format_duration(server.uptime/1000))'),
}

def f(server_write, args, nickname, **_):
    server_write('''code:\
	server.msg("REMOTE ADMIN (%s): %s");\
	sendmsg("REMOTE ADMIN (%s): %s")\
	''' % (nickname, args, nickname, args))
	
server['say'] = f


def f(server_write, args, nickname, **_):
    server_write('''\
			code:local cn = tonumber('%s')\
			if server.valid_cn(cn) then\
				server.kick(cn, 9999, "%s", "kicked by a remote admin.")\
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args, nickname)
server['kick'] = f

def f(server_write, args, **_):
    server_write('''\
			code:local cn = tonumber('%s')\
			if server.valid_cn(cn) then\
				sendmsg("Name: "..server.player_name(cn).." Frags: "..server.player_frags(cn).." Deaths: "..server.player_deaths(cn).."  Accuracy: "..server.player_accuracy(cn)..string.char(37).." Ping: "..server.player_ping(cn).." Country: "..geoip.ip_to_country(server.player_ip(cn)))\
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args)
server['stats'] = f

def f(server_write, args, **_):
    server_write('''\
			code:local cn = tonumber('%s'); local min = tonumber('%s');\
			if server.valid_cn(cn) then\
				server.mute(cn, min)\
				sendmsg("muted " .. server.player_name(cn) .. " for " .. min .. " minutes")
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % (args.split()[0],args.split()[1]))
server['mute'] = f

def f(server_write, args, **_):
    server_write('''\
			code:local cn = tonumber('%s');\
			if server.valid_cn(cn) then\
				server.unmute(cn)\
				sendmsg("unmuted " .. server.player_name(cn))
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args)
server['unmute'] = f



def f(server_write, args, **_):
    server_write('''\
			code:local cn = tonumber('%s');\
			if server.valid_cn(cn) then\
				server.disconnect(cn, 10, "disconnect by a admin")\
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args)
server['disconnect'] = f


def f(server_write, args, **_):
    server_write('''\
			code:local cn = tonumber('%s');\
			if server.valid_cn(cn) then\
				server.spec(cn)\
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args)
server['spec'] = f

def f(server_write, args, **_):
	server_write('''\
			code:local cn = tonumber('%s');\
			if server.valid_cn(cn) then\
				server.slay(cn)\
				sendmsg("player slayed")\
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args)
server['slay'] = f
			

def f(server_write, args, **_):
    server_write('''\
			code:local cn = tonumber('%s');\
			if server.valid_cn(cn) then\
				server.unspec(cn)\
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args)
server['unspec'] = f

def f(server_write, args, **_):
    server_write('''\
			code:server.pausegame(1)\
			''')
server['pause'] = f

def f(server_write, args, **_):
    server_write('''\
			code:server.pausegame(false)\
			''')
server['resume'] = f

def f(server_write, args, **_):
	server_write('''\
			code:server.specall(true)\
			''')
server['specall'] = f

def f(server_write, args, **_):
	server_write('''\
			code:server.unspecall(true)\
			''')
server['unspecall'] = f

def f(server_write, args, **_):
	server_write('''\
			code:server.recorddemo\
			''')
server['recorddemo'] = f

def f(server_write, args, **_):
	server_write('''\
			code:server.stopdemo\
			''')
server['stopdemo'] = f

def f(server_write, args, **_):
    server_write('''\
			code:local cn = tonumber('%s');\
			if server.valid_cn(cn) then\
				sendmsg("IP: "..server.player_ip(cn))\
			else\
				sendmsg("player not found")\
			end\
			\
			\
			''' % args)
server['ip'] = f

def f(server_write, args, **_):
    server_write('''\
			code:\
				server.changemap("%s")\
			''' % args)
server['map'] = f

def f(server_write, args, **_):
    server_write('''\
			code:\
				server.changetime(tonumber('%s')*1000*60)\
			''' % args)
server['changetime'] = f

def f(server_write, args, **_):
    server_write('''\
			code:server.reloadscripts(1)\
			''')
server['reload'] = f		