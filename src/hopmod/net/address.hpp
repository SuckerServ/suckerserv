#ifndef HOPMOD_NET_ADDRESS_HPP
#define HOPMOD_NET_ADDRESS_HPP

#include <string>

namespace hopmod{
namespace ip{

class address_mask;

class address
{
public:
    typedef unsigned long integral_type;
    typedef unsigned char octet_type;
    typedef char cstring_buffer[16];

    static const std::size_t bits = 32;
    static const std::size_t octet_parts = 4;

    static const integral_type BROADCAST = 0xFFFFFFFF;
    static const integral_type UPPER_LIMIT = 0xFFFFFFFF;

    address();
    address(integral_type);
    address(octet_type, octet_type, octet_type, octet_type);

    address::integral_type value()const { return m_value; }

    void to_string(cstring_buffer *)const;
    std::string to_string()const;

    address operator<<(std::size_t)const;
    address operator>>(std::size_t)const;
    address operator&(address_mask)const;

    bool operator==(address)const;
    bool operator!=(address)const;

    bool first_bit()const;
private:
    union
    {
        integral_type m_value;
        struct{
            octet_type m_value_part[octet_parts];
        };
    };
};

} //namespace ip
} //namespace hopmod

#endif
