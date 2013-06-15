#ifndef __IGAME_H__
#define __IGAME_H__

// the interface the engine uses to run the gameplay module

namespace game
{
    extern void parseoptions(vector<const char *> &args);
} 
 
namespace server
{
    extern void *newclientinfo();
    extern void deleteclientinfo(void *ci);
    extern void serverinit();
    extern int reserveclients();
    extern int numchannels();
    extern void clientdisconnect(int n,int reason);
    extern int clientconnect(int n, uint ip);
    extern void localdisconnect(int n);
    extern void localconnect(int n);
    extern bool allowbroadcast(int n);
    extern void recordpacket(int chan, void *data, int len);
    extern void parsepacket(int sender, int chan, packetbuf &p);
    extern void sendservmsg(const char *s);
    extern bool sendpackets(bool force = false);
    extern void serverinforeply(ucharbuf &req, ucharbuf &p);
    extern void serverupdate();
    extern bool servercompatible(char *name, char *sdec, char *map, int ping, const vector<int> &attr, int np);
    extern int laninfoport();
    extern int serverinfoport(int servport = -1);
    extern int serverport(int infoport = -1);
    extern const char *defaultmaster();
    extern int masterport();
    extern void processmasterinput(const char *cmd, int cmdlen, const char *args);
    extern void started();
    extern void shutdown();
    extern bool ispaused();
    extern int scaletime(int t);
    extern bool ctftkpenalty;
    extern bool spec_slots;
    extern bool anti_cheat_enabled;
    extern int anti_cheat_system_rev;
    extern uint mcrc;
    extern int gamespeed;
    extern void real_cn(int &n);
    extern int spycn;
}

#endif
