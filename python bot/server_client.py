import sys
import struct
from twisted.internet import defer, protocol, reactor
from twisted.protocols import basic
from twisted.python import log

CHANNEL_DELIM = '\xf0'

class HMServerClient(basic.LineOnlyReceiver):

    def connectionMade(self):
        self.factory.controller.servers[self.factory.name]['client'] = self
        if self.factory.password:
            self.sendLine('pass:%s' % self.factory.password)

    def lineReceived(self, data):
        data = data.strip()
        if data == 'pass:wrong password':
            self.factory.stop_reconnecting = True
            return
        if self.factory.controller.irc_client and data:
            channel = None
            message = data
            if data[0] == CHANNEL_DELIM:
                channel, _, message = data[1:].partition(' ')
            elif not self.factory.admin_channel:
                channel = self.factory.controller.irc_identity.get('default_channel')
            message = '%s: %s' % (self.factory.formatted_name, message)

            if not channel:
                self.factory.controller.irc_client.write(message, self.factory.admin_channel)
            else:
                self.factory.controller.irc_client.msg(channel, message)

class HMServerFactory(protocol.ClientFactory):

    protocol          = HMServerClient
    stop_reconnecting = False
    reconnect_time    = 5
    admin_channel     = False

    def __init__(self, controller, name, formatted_name, password):
        self.controller     = controller
        self.name           = name
        self.formatted_name = formatted_name
        self.password       = password

    def clientConnectionLost(self, connector, reason):
        if not self.stop_reconnecting:
            connector.connect()
        self.controller.servers[self.name]['client'] = None

    def clientConnectionFailed(self, connector, reason):
        if not self.stop_reconnecting:
            reactor.callLater(self.reconnect_time, lambda: connector.connect())
        self.controller.servers[self.name]['client'] = None

    def __repr__(self):
        return 'HMServerFactory "%s"' % self.name

