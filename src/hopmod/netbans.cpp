#include "net/address.hpp"
#include "net/address_mask.hpp"
#include "net/address_prefix.hpp"
#include "netbans.hpp"
#include <netinet/in.h>
#include <vector>
#include <typeinfo>

namespace hopmod{
namespace netbans{

class netban
{
private:
    address ip;
    address_mask mask;
public:
    bool check(const address::integral_type ip_in)const
    {
        const address::integral_type m = mask.value(); // less function calls
        return ((ip_in & m) == (ip.value() & m));
    }

    netban(address ip, address_mask mask) : ip(ip), mask(mask) {}
};

static std::vector<netban> netbans;

bool check_ban(address::integral_type ip)
{
    ip = ntohl(ip);

    for(std::vector<netban>::iterator it = netbans.begin(); it != netbans.end(); ++it)
    {
        if(it->check(ip)) return true;
    }

    return false;
}

void add_ban(const char * addr)
{
    address_prefix prefix;

    try
    {
        prefix = prefix.parse(addr);
    }
    catch(std::bad_cast &)
    {
        return;
    }

    netbans.push_back(netban(prefix.value(), prefix.mask()));
}

void clear_bans()
{
    netbans.clear();
}

} //namespace netbans
} //namespace hopmod
