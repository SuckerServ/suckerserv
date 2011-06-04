irc_nickname = 'Suckerbot'

irc_identity = {
    'gamesurge': {
        'host': '195.8.250.180',
        'port': 6667,
        'channels': (
            '#suckerserv',
        ),
        'line_rate': 2.0,
		'default_channel': '#suckerserv',
    },
}['gamesurge']

servers = {
    'server1': {
        'host':          'localhost',
        'port':           28795,
        'password':       '',
        'format':         '\00301%s\003',
        'reconnect_time': 60,
    },
    'server2': {
        'host':          'localhost',
        'port':           30010,
        'password':       '',
        'format':         '\00307%s\003',
        'reconnect_time': 60,
    }
}

stats_url = ''

