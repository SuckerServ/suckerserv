#ifndef HOPMOD_NET_ADDRESS_PREFIX_HPP
#define HOPMOD_NET_ADDRESS_PREFIX_HPP

#include "address.hpp"
#include "address_mask.hpp"

namespace hopmod{
namespace ip{

class address_prefix
{
public:
    typedef char cstring_buffer[19];

    address_prefix();
    address_prefix(address, address_mask);
    address_prefix(const address_prefix &, address_mask);

    static address_prefix parse(const char *);

    address value()const { return m_addr_prefix; }
    address_mask mask()const { return m_mask; }

    void to_string(cstring_buffer *)const;
    std::string to_string()const;

    static address_prefix common_prefix(const address_prefix &, const address_prefix &);
    address_prefix common_prefix(const address_prefix &)const;

    address_prefix & operator<<=(address_mask);
    bool operator==(const address_prefix &)const;
private:
    address m_addr_prefix;
    address_mask m_mask;
};

} //namespace ip
} //namespace hopmod

#endif
