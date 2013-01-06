// server.cpp: little more than enhanced multicaster
// runs dedicated or as client coroutine

#include "cube.h"
#include <signal.h>
#include <boost/asio.hpp>
using namespace boost::asio;
#include <enet/time.h>
#include <iostream>

static void shutdown_from_signal(int i)
{
    server::shutdown();
}

#ifdef STANDALONE
void fatal(const char *s, ...) 
{
    stopgameserver(0); 
    defvformatstring(msg,s,s);
    printf("servererror: %s\n", msg); 
    exit(EXIT_FAILURE); 
}

void conoutfv(int type, const char *fmt, va_list args)
{
    string sf, sp;
    vformatstring(sf, fmt, args);
    filtertext(sp, sf);
    puts(sp);
}

void conoutf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(CON_INFO, fmt, args);
    va_end(args);
}

void conoutf(int type, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    conoutfv(type, fmt, args);
    va_end(args);
}
#endif

// all network traffic is in 32bit ints, which are then compressed using the following simple scheme (assumes that most values are small).

template<class T>
static inline void putint_(T &p, int n)
{
    if(n<128 && n>-127) p.put(n);
    else if(n<0x8000 && n>=-0x8000) { p.put(0x80); p.put(n); p.put(n>>8); }
    else { p.put(0x81); p.put(n); p.put(n>>8); p.put(n>>16); p.put(n>>24); }
}
void putint(ucharbuf &p, int n) { putint_(p, n); }
void putint(packetbuf &p, int n) { putint_(p, n); }
void putint(vector<uchar> &p, int n) { putint_(p, n); }

int getint(ucharbuf &p)
{
    int c = (char)p.get();
    if(c==-128) { int n = p.get(); n |= char(p.get())<<8; return n; }
    else if(c==-127) { int n = p.get(); n |= p.get()<<8; n |= p.get()<<16; return n|(p.get()<<24); } 
    else return c;
}

// much smaller encoding for unsigned integers up to 28 bits, but can handle signed
template<class T>
static inline void putuint_(T &p, int n)
{
    if(n < 0 || n >= (1<<21))
    {
        p.put(0x80 | (n & 0x7F));
        p.put(0x80 | ((n >> 7) & 0x7F));
        p.put(0x80 | ((n >> 14) & 0x7F));
        p.put(n >> 21);
    }
    else if(n < (1<<7)) p.put(n);
    else if(n < (1<<14))
    {
        p.put(0x80 | (n & 0x7F));
        p.put(n >> 7);
    }
    else 
    { 
        p.put(0x80 | (n & 0x7F)); 
        p.put(0x80 | ((n >> 7) & 0x7F));
        p.put(n >> 14); 
    }
}
void putuint(ucharbuf &p, int n) { putuint_(p, n); }
void putuint(packetbuf &p, int n) { putuint_(p, n); }
void putuint(vector<uchar> &p, int n) { putuint_(p, n); }

int getuint(ucharbuf &p)
{
    int n = p.get();
    if(n & 0x80)
    {
        n += (p.get() << 7) - 0x80;
        if(n & (1<<14)) n += (p.get() << 14) - (1<<14);
        if(n & (1<<21)) n += (p.get() << 21) - (1<<21);
        if(n & (1<<28)) n |= -1<<28;
    }
    return n;
}

template<class T>
static inline void putfloat_(T &p, float f)
{
    lilswap(&f, 1);
    p.put((uchar *)&f, sizeof(float));
}
void putfloat(ucharbuf &p, float f) { putfloat_(p, f); }
void putfloat(packetbuf &p, float f) { putfloat_(p, f); }
void putfloat(vector<uchar> &p, float f) { putfloat_(p, f); }

float getfloat(ucharbuf &p)
{
    float f;
    p.get((uchar *)&f, sizeof(float));
    return lilswap(f);
}

template<class T>
static inline void sendstring_(const char *t, T &p)
{
    while(*t) putint(p, *t++);
    putint(p, 0);
}
void sendstring(const char *t, ucharbuf &p) { sendstring_(t, p); }
void sendstring(const char *t, packetbuf &p) { sendstring_(t, p); }
void sendstring(const char *t, vector<uchar> &p) { sendstring_(t, p); }

void getstring(char *text, ucharbuf &p, int len)
{
    char *t = text;
    do
    {
        if(t>=&text[len]) { text[len-1] = 0; return; }
        if(!p.remaining()) { *t = 0; return; } 
        *t = getint(p);
    }
    while(*t++);
}

void filtertext(char *dst, const char *src, bool whitespace, int len)
{
    for(int c = *src; c; c = *++src)
    {
        if(c == '\f')
        {
            if(!*++src) break;
            continue;
        }
        if(c == ' ' ? whitespace : isprint(c))
        {
            *dst++ = c;
            if(!--len) break;
        }
    }
    *dst = '\0';
}

enum { ST_EMPTY, ST_LOCAL, ST_TCPIP };

struct client                   // server side version of "dynent" type
{
    int type;
    int num;
    ENetPeer *peer;
    string hostname;
    void *info;
};

vector<client *> clients;

ENetHost *serverhost = NULL;
size_t bsend = 0, brec = 0;
size_t tx_packets = 0 , rx_packets = 0, tx_bytes = 0, rx_bytes = 0;
int laststatus = 0; 
ENetSocket pongsock = ENET_SOCKET_NULL, lansock = ENET_SOCKET_NULL;

io_service main_io_service;

io_service & get_main_io_service()
{
    return main_io_service;
}

ip::udp::socket serverhost_socket(main_io_service);
ip::udp::socket info_socket(main_io_service);
ip::udp::socket laninfo_socket(main_io_service);
ip::tcp::socket masterserver_client_socket(main_io_service);

deadline_timer update_timer(main_io_service);
deadline_timer register_timer(main_io_service);
deadline_timer netstats_timer(main_io_service);

void stopgameserver(int)
{
    kicknonlocalclients(DISC_NONE);
    
    if(serverhost)
    {
        enet_host_flush(serverhost);
        enet_host_destroy(serverhost);
        serverhost = NULL;
    }
    
    if(pongsock != ENET_SOCKET_NULL) enet_socket_destroy(pongsock);
    if(lansock != ENET_SOCKET_NULL) enet_socket_destroy(lansock);
    pongsock = lansock = ENET_SOCKET_NULL;
    
    update_timer.cancel();
    register_timer.cancel();
    netstats_timer.cancel();
}

void process(ENetPacket *packet, int sender, int chan);
//void disconnect_client(int n, int reason);

void *getclientinfo(int i) { return !clients.inrange(i) || clients[i]->type==ST_EMPTY ? NULL : clients[i]->info; }
ENetPeer *getclientpeer(int i) { return clients.inrange(i) && clients[i]->type==ST_TCPIP ? clients[i]->peer : NULL; }
int getnumclients()        { return clients.length(); }
uint getclientip(int n)    { return clients.inrange(n) && clients[n]->type==ST_TCPIP ? clients[n]->peer->address.host : 0; }

void sendpacket(int n, int chan, ENetPacket *packet, int exclude)
{
    if(n<0)
    {
        server::recordpacket(chan, packet->data, packet->dataLength);
        loopv(clients) if(i!=exclude && server::allowbroadcast(i)) sendpacket(i, chan, packet);
        return;
    }
    switch(clients[n]->type)
    {
        case ST_TCPIP:
        {
            enet_peer_send(clients[n]->peer, chan, packet);
            bsend += packet->dataLength;
            tx_bytes += packet->dataLength;
            tx_packets++;
            break;
        }

    }
}

ENetPacket *sendf(int cn, int chan, const char *format, ...)
{
    int exclude = -1;
    bool reliable = false;
    if(*format=='r') { reliable = true; ++format; }
    packetbuf p(MAXTRANS, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);
    va_list args;
    va_start(args, format);
    while(*format) switch(*format++)
    {
        case 'x':
            exclude = va_arg(args, int);
            break;

        case 'v':
        {
            int n = va_arg(args, int);
            int *v = va_arg(args, int *);
            loopi(n) putint(p, v[i]);
            break;
        }

        case 'i': 
        {
            int n = isdigit(*format) ? *format++-'0' : 1;
            loopi(n) putint(p, va_arg(args, int));
            break;
        }
        case 'f':
        {
            int n = isdigit(*format) ? *format++-'0' : 1;
            loopi(n) putfloat(p, (float)va_arg(args, double));
            break;
        }
        case 's': sendstring(va_arg(args, const char *), p); break;
        case 'm':
        {
            int n = va_arg(args, int);
            p.put(va_arg(args, uchar *), n);
            break;
        }
    }
    va_end(args);
    ENetPacket *packet = p.finalize();
    sendpacket(cn, chan, packet, exclude);
    return packet->referenceCount > 0 ? packet : NULL;
}

ENetPacket *sendfile(int cn, int chan, stream *file, const char *format, ...)
{
    if(cn < 0)
    {
#ifdef STANDALONE
        return NULL;
#endif
    }
    else if(!clients.inrange(cn)) return NULL;

    int len = file->size();
    if(len <= 0) return NULL;

    packetbuf p(MAXTRANS+len, ENET_PACKET_FLAG_RELIABLE);
    va_list args;
    va_start(args, format);
    while(*format) switch(*format++)
    {
        case 'i':
        {
            int n = isdigit(*format) ? *format++-'0' : 1;
            loopi(n) putint(p, va_arg(args, int));
            break;
        }
        case 's': sendstring(va_arg(args, const char *), p); break;
        case 'l': putint(p, len); break;
    }
    va_end(args);

    file->seek(0, SEEK_SET);
    file->read(p.subbuf(len).buf, len);

    ENetPacket *packet = p.finalize();
    if(cn >= 0) sendpacket(cn, chan, packet, -1);
#ifndef STANDALONE
    else sendclientpacket(packet, chan);
#endif
    return packet->referenceCount > 0 ? packet : NULL;
}

void disconnect_client_now(int n, int reason)
{
    if(!clients.inrange(n) || clients[n]->type!=ST_TCPIP) return;
    enet_peer_reset(clients[n]->peer);
    server::clientdisconnect(n, reason);
    clients[n]->type = ST_EMPTY;
    clients[n]->peer->data = NULL;
    server::deleteclientinfo(clients[n]->info);
    clients[n]->info = NULL;
}

void disconnect_client(int n, int reason)
{
    if(!clients.inrange(n) || clients[n]->type!=ST_TCPIP) return;
    enet_peer_disconnect(clients[n]->peer, reason);
    server::clientdisconnect(n, reason);
    clients[n]->type = ST_EMPTY;
    clients[n]->peer->data = NULL;
    server::deleteclientinfo(clients[n]->info);
    clients[n]->info = NULL;
}

void kicknonlocalclients(int reason)
{
    loopv(clients) if(clients[i]->type==ST_TCPIP) disconnect_client(i, reason);
}

void process(ENetPacket *packet, int sender, int chan)   // sender may be -1
{
    packetbuf p(packet);
    server::parsepacket(sender, chan, p);
    if(p.overread()) { disconnect_client(sender, DISC_EOP); return; }
}

void localclienttoserver(int chan, ENetPacket *packet)
{
    client *c = NULL;
    loopv(clients) if(clients[i]->type==ST_LOCAL) { c = clients[i]; break; }
    if(c) process(packet, c->num, chan);
}

client &addclient()
{
    loopv(clients) if(clients[i]->type==ST_EMPTY)
    {
        clients[i]->info = server::newclientinfo();
        return *clients[i];
    }
    client *c = new client;
    c->num = clients.length();
    c->info = server::newclientinfo();
    clients.add(c);
    return *c;
}

int localclients = 0, nonlocalclients = 0;

bool hasnonlocalclients() { return nonlocalclients!=0; }
bool haslocalclients() { return localclients!=0; }

#ifdef STANDALONE
bool resolverwait(const char *name, ENetAddress *address)
{
    return enet_address_set_host(address, name) >= 0;
}

int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &remoteaddress)
{
    int result = enet_socket_connect(sock, &remoteaddress);
    if(result<0) enet_socket_destroy(sock);
    return result;
}
#endif

ENetAddress serveraddress = { ENET_HOST_ANY, ENET_PORT_ANY };

static ENetAddress pongaddr;

void sendserverinforeply(ucharbuf &p)
{
    ENetBuffer buf;
    buf.data = p.buf;
    buf.dataLength = p.length();
    enet_socket_send(pongsock, &pongaddr, &buf, 1);
}

#define DEFAULTCLIENTS 8

int uprate = 0, maxclients = DEFAULTCLIENTS;
string serverip = "";
int serverport = server::serverport();

#ifdef STANDALONE
int curtime = 0, lastmillis = 0, totalmillis = 0;
#endif

static void check_peer_timeout(ENetHost * host, ENetPeer * peer)
{
    /*
        Most of the code in this function was copied from enet/protocol.c
    */
    
    ENetOutgoingCommand * outgoingCommand;
    ENetListIterator currentCommand, insertPosition;

    currentCommand = enet_list_begin (& peer -> sentReliableCommands);
    insertPosition = enet_list_begin (& peer -> outgoingReliableCommands);

    while (currentCommand != enet_list_end (& peer -> sentReliableCommands))
    {
       outgoingCommand = (ENetOutgoingCommand *) currentCommand;

       currentCommand = enet_list_next (currentCommand);
        
       if (ENET_TIME_DIFFERENCE (host -> serviceTime, outgoingCommand -> sentTime) < outgoingCommand -> roundTripTimeout)
         continue;

       if (peer -> earliestTimeout == 0 ||
           ENET_TIME_LESS (outgoingCommand -> sentTime, peer -> earliestTimeout))
         peer -> earliestTimeout = outgoingCommand -> sentTime;

       if (peer -> earliestTimeout != 0 &&
             (ENET_TIME_DIFFERENCE (host -> serviceTime, peer -> earliestTimeout) >= ENET_PEER_TIMEOUT_MAXIMUM ||
               (outgoingCommand -> roundTripTimeout >= outgoingCommand -> roundTripTimeoutLimit &&
                 ENET_TIME_DIFFERENCE (host -> serviceTime, peer -> earliestTimeout) >= ENET_PEER_TIMEOUT_MINIMUM)))
       {
            client *c = (client *)peer->data;
            disconnect_client_now(c->num, DISC_TIMEOUT);
            return;
       }

       if (outgoingCommand -> packet != NULL)
         peer -> reliableDataInTransit -= outgoingCommand -> fragmentLength;
          
       ++ peer -> packetsLost;

       outgoingCommand -> roundTripTimeout *= 2;

       enet_list_insert (insertPosition, enet_list_remove (& outgoingCommand -> outgoingCommandList));

       if (currentCommand == enet_list_begin (& peer -> sentReliableCommands) &&
           ! enet_list_empty (& peer -> sentReliableCommands))
       {
          outgoingCommand = (ENetOutgoingCommand *) currentCommand;

          peer -> nextTimeout = outgoingCommand -> sentTime + outgoingCommand -> roundTripTimeout;
       }
    }
}

static void check_timeouts()
{
    enet_host_flush(serverhost); // called for the serviceTime update
    
    for(ENetPeer * currentPeer = serverhost->peers; 
        currentPeer < &serverhost->peers[serverhost->peerCount]; 
        currentPeer++)
    {
        if (currentPeer -> state != ENET_PEER_STATE_CONNECTED)
            continue;
        
        check_peer_timeout(serverhost, currentPeer);
    }
}

void update_server(const boost::system::error_code & error)
{
    if(error) return;
    
    localclients = nonlocalclients = 0;
    loopv(clients) switch(clients[i]->type)
    {
        case ST_LOCAL: localclients++; break;
        case ST_TCPIP: nonlocalclients++; break;
    }
    
    int millis = (int)enet_time_get();
    curtime = millis - totalmillis;
    lastmillis = totalmillis = millis;
    
    server::serverupdate();
    
    update_timer.expires_from_now(boost::posix_time::milliseconds(5));
    update_timer.async_wait(update_server);
    
    check_timeouts();
}

void serverhost_process_event(ENetEvent & event)
{
    switch(event.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
        {
            client &c = addclient();
            c.type = ST_TCPIP;
            c.peer = event.peer;
            c.peer->data = &c;
            char hn[1024];
            copystring(c.hostname, (enet_address_get_host_ip(&c.peer->address, hn, sizeof(hn))==0) ? hn : "unknown");
            printf("client connected (%s)\n", c.hostname);
            int reason = server::clientconnect(c.num, c.peer->address.host);
            if(!reason) nonlocalclients++;
            else disconnect_client(c.num, reason);
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            brec += event.packet->dataLength;
            rx_bytes += event.packet->dataLength;
            rx_packets++;
            client *c = (client *)event.peer->data;
            if(c) process(event.packet, c->num, event.channelID);
            if(event.packet->referenceCount==0) enet_packet_destroy(event.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT: 
        {
            client *c = (client *)event.peer->data;
            if(!c) break;
            server::clientdisconnect(c->num,DISC_NONE);
            nonlocalclients--;
            c->type = ST_EMPTY;
            event.peer->data = NULL;
            server::deleteclientinfo(c->info);
            c->info = NULL;
            break;
        }
        default:
            break;
    }
}

void service_serverhost()
{
    ENetEvent event;
    while(enet_host_service(serverhost, &event, 0) == 1)
    {
        serverhost_process_event(event);
    }
    
    if(server::sendpackets()) enet_host_flush(serverhost); //treat EWOULDBLOCK as packet loss
}

void serverhost_input(boost::system::error_code error,const size_t s)
{
    if(error) return;
    service_serverhost();
    serverhost_socket.async_receive(null_buffers(), serverhost_input);
}

void serverinfo_input(int fd)
{
    ENetBuffer buf;
    uchar pong[MAXTRANS];
    
    buf.data = pong;
    buf.dataLength = sizeof(pong);
    int len = enet_socket_receive(fd, &pongaddr, &buf, 1);
    if(len < 0) return;
    
    ucharbuf req(pong, len), p(pong, sizeof(pong));
    p.len += len;
    server::serverinforeply(req, p);
}

void info_input_handler(boost::system::error_code ec, const size_t s)
{
    if(!ec) serverinfo_input(pongsock);
    info_socket.async_receive(null_buffers(), info_input_handler);
}

void laninfo_input_handler(boost::system::error_code ec, const size_t s)
{
    if(!ec) serverinfo_input(lansock);
    laninfo_socket.async_receive(null_buffers(), laninfo_input_handler);
}

void netstats_handler(const boost::system::error_code & ec)
{
    if(ec) return;
    
    bool has_stats = nonlocalclients || serverhost->totalSentData || serverhost->totalReceivedData;
    
    if(has_stats) 
    {
        printf("status: %d remote clients, %.1f send, %.1f rec (K/sec)\n", nonlocalclients, serverhost->totalSentData/60.0f/1024, serverhost->totalReceivedData/60.0f/1024);
        serverhost->totalSentData = 0;
        serverhost->totalReceivedData = 0;
    }
    
    netstats_timer.expires_from_now(boost::posix_time::minutes(1));
    netstats_timer.async_wait(netstats_handler);
}

void flushserver(bool force)
{
    if(server::sendpackets(force) && serverhost) enet_host_flush(serverhost);
}

void rundedicatedserver()
{
    #ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    #endif
    
    server::started();
    
    printf("dedicated server started, waiting for clients...\n*READY*\n\n");
    fflush(stdout);
    fflush(stderr);
    
    update_timer.expires_from_now(boost::posix_time::milliseconds(5));
    update_timer.async_wait(update_server);
    
    netstats_timer.expires_from_now(boost::posix_time::minutes(1));
    netstats_timer.async_wait(netstats_handler);
    
    try
    {
        main_io_service.run();
    }
    catch(const boost::system::system_error & se)
    {
        std::cerr<<se.what()<<std::endl;
        throw;
    }
}

bool servererror(bool dedicated, const char *desc)
{
    printf("servererror: %s\n", desc); 
    server::shutdown();
    return false;
}

bool setuplistenserver(bool dedicated)
{
    ENetAddress address = { ENET_HOST_ANY, serverport <= 0 ? static_cast<enet_uint16>(server::serverport()) : static_cast<enet_uint16>(serverport) };
    if(serverip[0])
    {
        if(enet_address_set_host(&address, serverip)<0) conoutf(CON_WARN, "WARNING: server ip not resolved");
        else
        {
            serveraddress.host = address.host;
            char hn[1024];
            copystring(serverip, (enet_address_get_host_ip(&serveraddress, hn, sizeof(hn))==0 ? hn : "0.0.0.0"));
        }
    }
    else copystring(serverip,"0.0.0.0");
    
    serverhost = enet_host_create(&address, MAXCLIENTS, server::numchannels(), 0, uprate);
    if(!serverhost) return servererror(dedicated, "could not create server host");
    loopi(MAXCLIENTS) serverhost->peers[i].data = NULL;
    const char * _serverip = serverip[0] == '\0' ? "0.0.0.0" : serverip;
    std::cout<<"Game server socket listening on UDP "<<_serverip<<":"<<serverport<<std::endl;
    address.port = server::serverinfoport(serverport > 0 ? serverport : -1);
    pongsock = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    if(pongsock != ENET_SOCKET_NULL && enet_socket_bind(pongsock, &address) < 0)
    {
        enet_socket_destroy(pongsock);
        pongsock = ENET_SOCKET_NULL;
    }
    if(pongsock == ENET_SOCKET_NULL) return servererror(dedicated, "could not create server info socket");
    else enet_socket_set_option(pongsock, ENET_SOCKOPT_NONBLOCK, 1);
    std::cout<<"Game info socket listening on UDP "<<_serverip<<":"<<address.port<<std::endl;
    address.port = server::laninfoport();
    lansock = enet_socket_create(ENET_SOCKET_TYPE_DATAGRAM);
    if(lansock != ENET_SOCKET_NULL && (enet_socket_set_option(lansock, ENET_SOCKOPT_REUSEADDR, 1) < 0 || enet_socket_bind(lansock, &address) < 0))
    {
        enet_socket_destroy(lansock);
        lansock = ENET_SOCKET_NULL;
    }
    if(lansock == ENET_SOCKET_NULL) conoutf(CON_WARN, "WARNING: could not create LAN server info socket");
    else enet_socket_set_option(lansock, ENET_SOCKOPT_NONBLOCK, 1);
    
    serverhost_socket.assign(ip::udp::v4(), serverhost->socket);
    serverhost_socket.async_receive(null_buffers(), serverhost_input);
    
    info_socket.assign(ip::udp::v4(), pongsock);
    info_socket.async_receive(null_buffers(), info_input_handler);
    
    laninfo_socket.assign(ip::udp::v4(), lansock);
    laninfo_socket.async_receive(null_buffers(), laninfo_input_handler);
    
    return true;
}

void initserver(bool listen, bool dedicated)
{    
    struct sigaction terminate_action;
    sigemptyset(&terminate_action.sa_mask);
    terminate_action.sa_handler = shutdown_from_signal;
    terminate_action.sa_flags = 0;
    sigaction(SIGINT, &terminate_action, NULL);
    sigaction(SIGTERM, &terminate_action, NULL);
    
    server::serverinit();
    
    if(listen) setuplistenserver(dedicated);

    if(listen)
    {
        if(dedicated) rundedicatedserver();
#ifndef STANDALONE
        else conoutf("listen server started");
#endif
    }
}

bool serveroption(char *opt)
{
    switch(opt[1])
    {
        case 'u': uprate = atoi(opt+2); return true;
        case 'c': 
        {
            int clients = atoi(opt+2); 
            if(clients > 0) maxclients = min(clients, MAXCLIENTS);
            else maxclients = DEFAULTCLIENTS;
            return true;
        }
        case 'i': copystring(serverip, opt+2); return true;
        case 'j': serverport = atoi(opt+2); if(serverport<=0) serverport = server::serverport(); return true;
        default: return false;
    }
}

void flushserverhost()
{
    if(serverhost) enet_host_flush(serverhost);
}

vector<const char *> gameargs;

int prog_argc;
char * const * prog_argv;

int main(int argc, char* argv[])
{
    prog_argc = argc;
    prog_argv = argv;
 
    if(enet_initialize()<0) fatal("Unable to initialise network module");
    atexit(enet_deinitialize);
    enet_time_set(0);
    
    for(int i = 1; i<argc; i++) if(argv[i][0]!='-' || !serveroption(argv[i])) gameargs.add(argv[i]);
    game::parseoptions(gameargs);
    
    initserver(true, true);
    
    return 0;
}

