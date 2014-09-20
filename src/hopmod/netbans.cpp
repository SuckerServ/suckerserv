#include "hopmod.hpp"
#include <iostream>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <typeinfo>
#include <cstring>
#include <ctime>
#include <netinet/in.h>

namespace hopmod{
namespace netbans{

typedef char reason_string[64];

static std::size_t MINBANMASK = 16;
static const char * const BANFILE = "log/network_bans.bin";

class netban
{
private:
    address ip;
    address_mask mask;
    int persist;
    reason_string reason;
    unsigned hits;
    std::time_t lasthit;
    netban(address ip, address_mask mask, bool persist = false) : ip(ip), mask(mask), persist(persist), hits(), lasthit() {}
public:
    address::integral_type ip_value()const { return ip.value(); }
    address::integral_type mask_value()const { return mask.value(); }
    unsigned hits_value()const { return hits; }
    bool persist_value()const { return !!persist; }

    void null_terminate_reason() { reason[sizeof(reason_string)-1] = '\0'; }

    void set_reason(const char * reason_in)
    {
        strncpy(reason, reason_in, sizeof(reason_string));
        null_terminate_reason();
    }

    const char * reason_value()const { return reason; }

    std::string to_string()const
    {
        std::string buf;
        std::stringstream sbuf;

        buf = ip.to_string();
        buf += "/";
        sbuf << mask.bits();
        buf += sbuf.str();

        return buf;
    }

    bool is_valid()const { return (ip_value() && mask_value() && mask.bits() >= MINBANMASK); }

    bool operator==(const netban & nb)const
    {
        return (ip.value() == nb.ip.value() && mask.value() == nb.mask.value());
    }

    bool check(const address::integral_type ip_in)
    {
        if((ip_in & mask.value()) == (ip.value() & mask.value()))
        {
            hits++;
            lasthit = std::time(NULL);

            return true;
        }

        return false;
    }

    netban(const char * addr, bool persist = false) : ip(), mask(0)
    {
        address_prefix prefix;

        try
        {
            prefix = prefix.parse(addr);
        }
        catch(std::bad_cast &)
        {
            std::cerr << "invalid ip-address (" << addr << ") passed to " << __FUNCTION__ << "()" << std::endl;
            memset(&prefix, 0, sizeof(prefix));
        }

        *this = netban(prefix.value(), prefix.mask(), persist);
    }
};

typedef std::vector<netban> netban_vector;

static netban_vector netbans;
static std::queue<netban_vector> stack;

bool check_ban(address::integral_type ip)
{
    ip = ntohl(ip);

    for(std::vector<netban>::iterator it = netbans.begin(); it != netbans.end(); ++it)
    {
        if(it->check(ip)) return true;
    }

    return false;
}

static netban *find_ban(netban nb, netban_vector::iterator *it_out = NULL)
{
    netban *result = NULL;

    if(!nb.is_valid()) return NULL;

    for(netban_vector::iterator it = netbans.begin(); it != netbans.end(); ++it)
    {
        if(*it == nb)
        {
            if(it_out) *it_out = it;
            result = &*it;
            break;
        }
    }

    return result;
}

void push_bans()
{
    stack.push(netbans);
    netbans.clear();
}

void pop_bans()
{
    if(stack.empty()) return;

    netbans = stack.front();
}

void merge_pushed_bans()
{
    if(stack.empty()) return;

    netban_vector oldbans = stack.front();

    for(netban_vector::reverse_iterator it = oldbans.rbegin(); it != oldbans.rend(); ++it)
    {
        netban *newban = find_ban(*it);

        if(newban || it->persist_value())
        {
            if(newban) *newban = *it; // update hits
            else netbans.push_back(*it); // re-add persist ban

            oldbans.erase(it.base());
        }
    }

    for(netban_vector::iterator it = oldbans.begin(); it != oldbans.end(); ++it)
    {
        std::cout << "removed " << it->to_string() << " (hits: " << it->hits_value() << ") from network bans" << std::endl;
    }
}

void add_ban(const char * addr, bool persist, const char * reason)
{
    netban nb(addr, persist);

    if(nb.is_valid())
    {
        if(find_ban(nb)) return;

        nb.set_reason(reason);
        netbans.push_back(nb);
    }
}

void del_ban(const char * addr)
{
    netban_vector::iterator it;

    if(find_ban(addr, &it))
    {
        netbans.erase(it);
        std::cout << "removed " << it->to_string() << " (hits: " << it->hits_value() << ") from network bans" << std::endl;
    }
}

void clear_bans()
{
    netbans.clear();
}

void ban_list()
{
    const char * p = netbans.size() != 1 ? "s" : "";
    std::cout << "listing " << netbans.size() << " network ban" << p << std::endl;

    for(netban_vector::iterator it = netbans.begin(); it != netbans.end(); ++it)
    {
        std::cout << "ip: " << it->to_string() << " hits: " << it->hits_value() << " reason: " << it->reason_value() << std::endl;
    }
}

void init()
{
    std::ifstream bans(BANFILE, std::ios::binary);
    if(!bans.is_open()) return;

    std::streampos size;

    size = bans.tellg();
    bans.seekg(0, std::ios::end);
    size = bans.tellg() - size;
    bans.seekg(0, std::ios::beg);

    if(size % sizeof(netban) != 0)
    {
        bans.close();
        std::cerr << "Network ban file (" << BANFILE << ") is corrupted. Removing it" << std::endl;
        remove(BANFILE);

        return;
    }

    for(std::size_t i = 0; i < (size / sizeof(netban)); ++i)
    {
        char buf[sizeof(netban)];
        netban * nb = reinterpret_cast<netban *>(buf);

        bans.read(buf, sizeof(netban));
        nb->null_terminate_reason();

        if (!nb->is_valid()) continue;

        netbans.push_back(*nb);
    }

    if(false && netbans.size())
    {
        const char * p = netbans.size() != 1 ? "s" : "";
        std::cout << "Loaded " << netbans.size() << " network ban" << p << std::endl;
    }

    bans.close();
}

void shutdown()
{
    std::ofstream bans(BANFILE, std::ios::binary);

    if(!bans.is_open()) return;

    for(netban_vector::iterator it = netbans.begin(); it != netbans.end(); ++it)
    {
        bans.write(reinterpret_cast<char *>(&*it), sizeof(netban));
    }

    bans.close();
}

} //namespace netbans
} //namespace hopmod
