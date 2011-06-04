import functools

from twisted.internet import protocol, reactor
from twisted.python import log
from twisted.web import client
from twisted.words.protocols import irc

import commands

PREFIXES = '!.@'
CHANNEL_DELIM = '\xf0'

class HMIRCClient(irc.IRCClient):

    max_per_msg = 450

    def __init__(self):
        self.commands = {
            'all':     self.__command_all,
            'servers': self.__command_servers,
            'stats':   self.__command_stats,
        }
		
    def __command_all(self, args, channel, **_):
        for server in self.factory.controller.servers.itervalues():
            if server.get('client'):
                server['client'].sendLine('%s%s %s' % (CHANNEL_DELIM, channel, args))

    def __command_servers(self, write, **_):
        servers = map(lambda x: x['client'].factory, filter(lambda x: x.get('client'), self.factory.controller.servers.itervalues()))
        servers = map(lambda x: x.formatted_name, sorted(servers, key=lambda x: x.name))
        write('Servers: ' + ', '.join(servers or ('none!',)))

    def __command_stats(self, write, args, **_):
        url = self.factory.stats_url + '&player=' + args
        if url:
            client.getPage(url).addCallback(write)


    def connectionMade(self):
        irc.IRCClient.connectionMade(self)
        self.factory.controller.irc_client = self
        self.admins = set()

    def signedOn(self):
        self.lineRate = self.factory.controller.irc_identity.get('line_rate')
        for i in self.factory.controller.irc_identity.get('admin_channels', ()):
            self.join(i)
        for i in self.factory.controller.irc_identity['channels']:
            self.join(i)

    def noticed(self, user, channel, msg):
        pass

    def joined(self, channel):
        log.msg('Joined %s' % channel)

    def left(self, channel):
        log.msg('Left %s' % channel)

    def ctcpQuery(self, user, channel, messages):
        pass

    def irc_RPL_NAMREPLY(self, prefix, params):
        chantype, channel, users = params[1:]
        self.admins |= set([user[1:] for user in users.split() if user[0] == '@'])

    def userLeft(self, user, channel):
        self.admins.discard(user)

    def userQuit(self, user, channel):
        self.admins.discard(user)

    def userKicked(self, user, channel, kicker, message):
        self.admins.discard(user)

    def userRenamed(self, oldname, newname):
        if oldname in self.admins:
            self.admins.remove(oldname)
            self.admins.add(newname)

    def modeChanged(self, user, channel, set, modes, args):
        for n, mode in enumerate(modes):
            if mode == 'o' and args[n] != self.nickname:
                (self.admins.discard, self.admins.add)[set](args[n])

    def write(self, message, admin_channel=False):
        if not message:
            return
        if admin_channel:
            for channel in self.factory.controller.irc_identity.get('admin_channels', ()):
                self.msg(channel, message)
        else:
            for channel in self.factory.controller.irc_identity['channels']:
                self.msg(channel, message)

				

		

    def msg(self, channel, message):
        for line in message.splitlines():
            irc.IRCClient.msg(self, channel, line)

    def privmsg(self, user, channel, msg):
        servers = dict([(a,b) for a,b in self.factory.controller.servers.iteritems() if b.get('client')])
        if msg[0] in PREFIXES:

            no_prefix = msg.lstrip(PREFIXES)
            command, _, args = no_prefix.partition(' ')
            command = command.lower()
            nickname = user.split('!')[0]
            target = nickname if channel == self.nickname else channel

            command_args = {
                'args':     args,
                'channel':  target,
                'write':    functools.partial(self.msg, target),
            }

            if command in self.commands:
                reactor.callLater(0.0, self.commands[command], **command_args)
                return

            if user.split('!')[0] in self.admins and command in servers:
                server_command, _, n_args = args.partition(' ')

                if server_command in commands.server:
                    command_args['nickname']     = nickname
                    command_args['args']         = n_args
                    def server_write(message):
                        servers[command]['client'].sendLine(('%s%s %s' % (CHANNEL_DELIM, target, message)).replace('\n', ' '))
                    command_args['server_write'] = server_write

                    reactor.callLater(0.0, commands.server[server_command], **command_args)
                    log.msg('%s -> %s: %s' % (user, command, args))

        if channel[0] != '#':
            log.msg('%s: %s' % (user, msg))

class HMIRCFactory(protocol.ClientFactory):

    protocol = HMIRCClient

    def __init__(self, controller, nickname):
        self.controller = controller
        self.nickname   = nickname

    def buildProtocol(self, addr):
        p          = self.protocol()
        p.nickname = self.nickname
        p.factory  = self
        return p

    def clientConnectionLost(self, connector, reason):
        connector.connect()

    def clientConnectionFailed(self, connector, reason):
        reactor.stop()

    def __repr__(self):
        return 'HMIRCFactory "%s"' % self.nickname

