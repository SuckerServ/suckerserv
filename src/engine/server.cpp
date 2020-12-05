// server.cpp: little more than enhanced multicaster
// runs dedicated or as client coroutine

#include "cube.h"
#include <signal.h>
#include <asio.hpp>
#include <asio/high_resolution_timer.hpp>

using namespace asio;
#include <hopmod/utils/time.hpp>
#include <hopmod/net/address.hpp>
#include <hopmod/net/address_mask.hpp>
#include <hopmod/net/address_prefix.hpp>
#include <hopmod/netbans.hpp>
#include <enet/time.h>
#include <iostream>

static int check_net_bans(ENetHost * host, ENetEvent * event)
{
    if(!host) return 1;
    if(hopmod::netbans::check_ban(host->receivedAddress.host)) return 1;
    return 0;
}

static int check_net_bans(uint ip)
{
    if(hopmod::netbans::check_ban(ip)) return 1;
    return 0;
}

static void shutdown_from_signal(int i)
{
    extern void delclients();
    delclients();
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
#endif

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

size_t info_queries = 0;
size_t tx_info_bytes = 0;
size_t rx_info_bytes = 0;

io_service main_io_service;

io_service & get_main_io_service()
{
    return main_io_service;
}

ip::udp::socket serverhost_socket(main_io_service);
ip::udp::socket info_socket(main_io_service);
ip::udp::socket laninfo_socket(main_io_service);

high_resolution_timer update_timer(main_io_service);
high_resolution_timer netstats_timer(main_io_service);

void stopgameserver(int)
{
    kicknonlocalclients(DISC_NONE);

    std::error_code error;

    serverhost_socket.cancel(error);
    if(error)
        std::cerr<<"Error while trying to close game server socket: "<<error.message()<<std::endl;

    info_socket.close(error);
    if(error)
        std::cerr<<"Error while trying to close server info socket: "<<error.message()<<std::endl;

    laninfo_socket.close(error);
    if(error)
        std::cerr<<"Error while trying to close the server info lan socket: "<<error.message()<<std::endl;

    if(serverhost)
    {
        enet_host_flush(serverhost);
        enet_host_destroy(serverhost);
        serverhost = NULL;
    }

    if(pongsock != ENET_SOCKET_NULL) enet_socket_destroy(pongsock);
    if(lansock != ENET_SOCKET_NULL) enet_socket_destroy(lansock);
    pongsock = lansock = ENET_SOCKET_NULL;

    update_timer.cancel(error);
    if(error)
        std::cerr<<"Error while trying to stop the update timer: "<<error.message()<<std::endl;

    netstats_timer.cancel(error);
    if(error)
        std::cerr<<"Error while trying to stop the net stats timer: "<<error.message()<<std::endl;

}

void process(ENetPacket *packet, int sender, int chan);

int getservermtu() { return serverhost ? serverhost->mtu : -1; }
void *getclientinfo(int i) { return !clients.inrange(i) || clients[i]->type==ST_EMPTY ? NULL : clients[i]->info; }
ENetPeer *getclientpeer(int i) { return clients.inrange(i) && clients[i]->type==ST_TCPIP ? clients[i]->peer : NULL; }
int getnumclients()        { return clients.length(); }
uint getclientip(int n)    { return clients.inrange(n) && clients[n]->type==ST_TCPIP ? clients[n]->peer->address.host : 0; }

static int demooverride = 0;

void sendpacket(int n, int chan, ENetPacket *packet, int exclude)
{
    if(!demooverride?n<0:demooverride==1) server::recordpacket(chan, packet->data, packet->dataLength);
    if(n<0)
    {
        loopv(clients) if(i!=exclude && server::allowbroadcast(i)) sendpacket(i, chan, packet);
        return;
    }
    if(n >= server::spycn) server::real_cn(n);
    ASSERT(n >= 0 && n < MAXCLIENTS);
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
	case 'd': demooverride = +1; break;
	case 'D': demooverride = -1; break;
    }
    va_end(args);
    ENetPacket *packet = p.finalize();
    sendpacket(cn, chan, packet, exclude);
    demooverride = 0;
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

    int len = (int)min(file->size(), stream::offset(INT_MAX));
    if(len <= 0 || len > 16<<20) return NULL;

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

void delclient(client *c);

void disconnect_client_now(int n, int reason)
{
    if(!clients.inrange(n) || clients[n]->type!=ST_TCPIP) return;
    enet_peer_reset(clients[n]->peer);
    server::clientdisconnect(n, reason);
    delclient(clients[n]);
}

void disconnect_client(int n, int reason)
{
    if(!clients.inrange(n) || clients[n]->type!=ST_TCPIP) return;
    enet_peer_disconnect(clients[n]->peer, reason);
    server::clientdisconnect(n, reason);
    delclient(clients[n]);
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

int localclients = 0, nonlocalclients = 0;

bool hasnonlocalclients() { return nonlocalclients!=0; }
bool haslocalclients() { return localclients!=0; }

client &addclient(int type)
{
    client *c = NULL;
    loopv(clients) if(clients[i]->type==ST_EMPTY)
    {
        c = clients[i];
        break;
    }
    if(!c)
    {
        c = new client;
        c->num = clients.length();
        clients.add(c);
    }
    c->info = server::newclientinfo();
    c->type = type;
    switch(type)
    {
        case ST_TCPIP: nonlocalclients++; break;
        case ST_LOCAL: localclients++; break;
    }
    return *c;
}

void delclient(client *c)
{
    if(!c) return;
    switch(c->type)
    {
        case ST_TCPIP: nonlocalclients--; if(c->peer) c->peer->data = NULL; break;
        case ST_LOCAL: localclients--; break;
        case ST_EMPTY: return;
    }
    c->type = ST_EMPTY;
    c->peer = NULL;
    if(c->info)
    {
        server::deleteclientinfo(c->info);
        c->info = NULL;
    }
}

void delclients()
{
    loopv(clients) delclient(clients[i]);
}

#ifdef STANDALONE
bool resolverwait(const char *name, ENetAddress *address)
{
    return enet_address_set_host(address, name) >= 0;
}

int connectwithtimeout(ENetSocket sock, const char *hostname, const ENetAddress &remoteaddress)
{
    return enet_socket_connect(sock, &remoteaddress);
}
#endif

ENetAddress serveraddress = { ENET_HOST_ANY, ENET_PORT_ANY };

ENetAddress pongaddr;

void sendserverinforeply(ucharbuf &p)
{
    tx_info_bytes += p.length();
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
int curtime = 0, lastmillis = 0, elapsedtime = 0, totalmillis = 0;
#endif

ullong startup = 0;

void update_server(const std::error_code & error);
bool serverhost_service();

static void update_time()
{
    int millis = (int)getmilliseconds();
    elapsedtime = millis - totalmillis;
    static int timeerr = 0;
    int scaledtime = server::scaletime(elapsedtime) + timeerr;
    curtime = scaledtime/100;
    timeerr = scaledtime%100;
    if(server::ispaused()) curtime = 0;
    lastmillis += curtime;
    totalmillis = millis;
}

void sched_next_update()
{
    std::chrono::duration<long int, std::ratio<1l, 1000000000l> > expires_from_now = update_timer.expires_from_now();

    if(expires_from_now < std::chrono::duration<long int, std::ratio<1l, 1000000000l> >::zero())
    {
        update_timer.expires_from_now(std::chrono::duration<long int, std::milli>(5));
        update_timer.async_wait(update_server);
    }
}

void update_server(const std::error_code & error = std::error_code())
{    
    if(nonlocalclients > 0) sched_next_update();

    update_time();

    server::serverupdate();

    if(serverhost) serverhost_service();
}

void serverhost_process_event(ENetEvent & event)
{
    switch(event.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
        {
            client &c = addclient(ST_TCPIP);
            c.peer = event.peer;
            c.peer->data = &c;
            string hn;
            copystring(c.hostname, (enet_address_get_host_ip(&c.peer->address, hn, sizeof(hn))==0) ? hn : "unknown");

            printf("client connected (%s)\n", c.hostname);

            update_server();

            int reason = server::clientconnect(c.num, c.peer->address.host);
            if(reason) disconnect_client(c.num, reason);
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
            delclient(c);
            break;
        }
        default:
            break;
    }
}

void serverhost_receive(std::error_code error,const size_t s);

bool serverhost_service()
{
    ENetEvent event;
    int status;
    while((status = enet_host_service(serverhost, &event, 0)) == 1)
    {
        serverhost_process_event(event);
    }

    if(status == -1) return false;

    if(server::sendpackets()) enet_host_flush(serverhost); //treat EWOULDBLOCK as packet loss

    return true;
}

void serverhost_receive(std::error_code error,const size_t s)
{
    if(error) return;
    if(serverhost_service())
    {
        serverhost_socket.async_receive(null_buffers(), serverhost_receive);
    }
}

void serverinfo_input(int fd)
{
    ENetBuffer buf;
    uchar pong[MAXTRANS];

    buf.data = pong;
    buf.dataLength = sizeof(pong);
    int len = enet_socket_receive(fd, &pongaddr, &buf, 1);
    if(len < 0) return;

    info_queries++;
    rx_info_bytes += len;

    if(check_net_bans(pongaddr.host)) return;

    ucharbuf req(pong, len), p(pong, sizeof(pong));
    p.len += len;
    server::serverinforeply(req, p);
}

void info_input_handler(std::error_code ec, const size_t s)
{
    if(ec) return;
    serverinfo_input(pongsock);
    info_socket.async_receive(null_buffers(), info_input_handler);
}

void laninfo_input_handler(std::error_code ec, const size_t s)
{
    if(ec) return;
    serverinfo_input(lansock);
    laninfo_socket.async_receive(null_buffers(), laninfo_input_handler);
}

void netstats_handler(const std::error_code & ec)
{
    if(ec) return;

    bool has_stats = nonlocalclients || serverhost->totalSentData || serverhost->totalReceivedData;

    if(has_stats)
    {
        printf("client traffic: %d remote clients, %.1f send, %.1f rec (KiB/s)\n", nonlocalclients, serverhost->totalSentData/60.0f/1024, serverhost->totalReceivedData/60.0f/1024);
        serverhost->totalSentData = 0;
        serverhost->totalReceivedData = 0;
    }

    if(info_queries)
    {
        printf("info traffic: %llu queries/sec %.2f send, %.2f rec (KiB/s)\n", (ullong)info_queries,
            tx_info_bytes/60.0f/1024, rx_info_bytes/60.0f/1024);

        info_queries = 0;
        rx_info_bytes = 0;
        tx_info_bytes = 0;
    }

    netstats_timer.expires_from_now(std::chrono::duration<int, std::ratio<60> >(1));
    netstats_timer.async_wait(netstats_handler);
}

void flushserver(bool force)
{
    if(server::sendpackets(force) && serverhost) enet_host_flush(serverhost);
}

static bool dedicatedserver = false;

bool isdedicatedserver() { return dedicatedserver; }

void rundedicatedserver()
{
    dedicatedserver = true;
    #ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    #endif

    server::started();

    printf("dedicated server started, waiting for clients...\n*READY*\n\n");
    fflush(stdout);
    fflush(stderr);

    sched_next_update();

    netstats_timer.expires_from_now(std::chrono::duration<int, std::ratio<60> >(1));
    netstats_timer.async_wait(netstats_handler);

    try
    {
        main_io_service.run();
    }
    catch(const std::system_error & se)
    {
        std::cerr<<se.what()<<std::endl;
        throw;
    }
}

bool servererror(bool dedicated, const char *desc)
{
    printf("servererror: %s\n", desc);
    delclients();
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
    serverhost->duplicatePeers = 10;
    serverhost->intercept = check_net_bans;
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
    serverhost_socket.async_receive(null_buffers(), serverhost_receive);

    info_socket.assign(ip::udp::v4(), pongsock);
    info_socket.async_receive(null_buffers(), info_input_handler);

    laninfo_socket.assign(ip::udp::v4(), lansock);
    laninfo_socket.async_receive(null_buffers(), laninfo_input_handler);

    return true;
}

void initserver(bool listen, bool dedicated)
{
    startup = getnanoseconds();
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

bool restart_program;

int main(int argc, char* argv[])
{
    restart_program = false;

    
    if(enet_initialize()<0) fatal("Unable to initialise enet");
    atexit(enet_deinitialize);
    enet_time_set(0);

    for(int i = 1; i<argc; i++) if(argv[i][0]!='-' || !serveroption(argv[i])) gameargs.add(argv[i]);
    game::parseoptions(gameargs);

    initserver(true, true);

    if(restart_program)
    {
        execv(argv[0], argv);
    }

    return 0;
}

