import sys

from twisted.application import service, internet

from config import irc_identity, irc_nickname, servers, stats_url
from controller import Controller
from irc_client import HMIRCFactory
from server_client import HMServerFactory

application = service.Application('Hopmod irc bot')

c = Controller(irc_identity, servers)
f = HMIRCFactory(c, irc_nickname)
f.stats_url = stats_url

internet.TCPClient(irc_identity['host'], irc_identity['port'], f).setServiceParent(application)

for name, server in servers.iteritems():
    formatted_name = (server['format'] % name) if 'format' in server else name

    f = HMServerFactory(c, name, formatted_name, server['password'])
    if 'reconnect_time' in server:
        f.reconnect_time = server['reconnect_time']
    if 'admin_channel' in server:
        f.admin_channel  = server['admin_channel']

    internet.TCPClient(server['host'], server['port'], f).setServiceParent(application)

