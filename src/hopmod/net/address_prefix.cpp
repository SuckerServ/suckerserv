#include "address_prefix.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <typeinfo>

namespace hopmod{
namespace ip{

address_prefix::address_prefix()
:m_addr_prefix(0), m_mask(0)
{

}

address_prefix::address_prefix(address addr_prefix, address_mask mask)
 :m_addr_prefix(addr_prefix), m_mask(mask)
{

}

address_prefix::address_prefix(const address_prefix & prefix, address_mask mask)
 :m_addr_prefix(prefix.m_addr_prefix & mask), m_mask(mask)
{

}

address_prefix address_prefix::parse(const char * str)
{
    char str_copy[19];
    char * str_copy_p = str_copy;
    const char * tokens[5] = {str_copy,"0","0","0",NULL};
    const char ** current_token = tokens;

    for(; *str && str_copy_p < str_copy + sizeof(str_copy) - 1; ++str, ++str_copy_p)
    {
        char c = *str;
        bool new_token = false;

        if(c == '.')
        {
            if(current_token == &tokens[3]) throw std::bad_cast();

            ++current_token;
            new_token = true;
        }
        else if(c == '/')
        {
            if( (current_token == &tokens[0] &&
                str_copy_p == &str_copy[0]) ||
                current_token == &tokens[4] ) throw std::bad_cast();

            current_token = &tokens[4];
            new_token = true;
        }

        if(new_token)
        {
            if(*(str+1)=='\0') throw std::bad_cast();
            *current_token = str_copy_p + 1;
            c = '\0';
        }

        if((c != '\0' && c < '0') || c > '9') throw std::bad_cast();

        *str_copy_p = c;
    }

    // str is too big, probably caused by leading zeros.
    if(*str && str_copy_p == str_copy + sizeof(str_copy) - 1) throw std::bad_cast();

    *str_copy_p = '\0';

    unsigned long a = atoi(tokens[0]);
    unsigned long b = atoi(tokens[1]);
    unsigned long c = atoi(tokens[2]);
    unsigned long d = atoi(tokens[3]);
    if( a > 255 || b > 255 || c > 255 || d > 255) throw std::bad_cast();

    unsigned long prefix = (a << 24) | (b << 16) | (c << 8) | d;
    int maskbits = tokens[4] ? atoi(tokens[4]) : 32 ;
    if(maskbits > 32 || maskbits == 0) throw std::bad_cast();

    return address_prefix(address(prefix), address_mask(maskbits));
}

void address_prefix::to_string(cstring_buffer * output)const
{
    address::cstring_buffer ip;
    m_addr_prefix.to_string(&ip);

    char mask[4];
    mask[0] = '\0';

    int bits = static_cast<int>(m_mask.bits());
    if(bits < 32) sprintf(mask, "/%i", bits);

    sprintf(*output, "%s%s", ip, mask);
}

std::string address_prefix::to_string()const
{
    cstring_buffer buffer;
    to_string(&buffer);
    return buffer;
}

address_prefix address_prefix::common_prefix(const address_prefix & first, const address_prefix & second)
{
    int n = std::min(first.m_mask.bits(), second.m_mask.bits());
    int common_mask_bits = n;
    for(int i = 1; i <= n; i++)
    {
        address_mask mask_i(i);
        if((first.m_addr_prefix & mask_i) != (second.m_addr_prefix & mask_i))
        {
            common_mask_bits = i - 1;
            break;
        }
    }
    address_mask new_mask(common_mask_bits);
    return address_prefix(first.m_addr_prefix & new_mask, new_mask);
}

address_prefix address_prefix::common_prefix(const address_prefix & second)const
{
    return common_prefix(*this, second);
}

address_prefix & address_prefix::operator<<=(address_mask mask)
{
    m_addr_prefix = m_addr_prefix << mask.bits();
    m_mask = m_mask << mask.bits();
    return *this;
}

bool address_prefix::operator==(const address_prefix & x)const
{
    address::integral_type mask = std::min(x.mask().value(), m_mask.value());
    return (m_addr_prefix.value() & mask) == (x.m_addr_prefix.value() & mask);
}

} //namespace ip
} //namespace hopmod
