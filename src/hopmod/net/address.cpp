#include "address.hpp"
#include "address_mask.hpp"
#include <cstdio>
#include <cassert>

namespace hopmod{
namespace ip{

address::address()
 :m_value(0)
{
    
}

address::address(integral_type value)
 :m_value(value)
{
    
}

address::address(octet_type b1, octet_type b2, octet_type b3, octet_type b4)
{
    //FIXME byte ordering
    m_value_part[0] = b4;
    m_value_part[1] = b3;
    m_value_part[2] = b2;
    m_value_part[3] = b1;
}

address::integral_type address::value()const
{
    return m_value;
}

void address::to_string(cstring_buffer * output)const
{
    sprintf(*output, "%i.%i.%i.%i", m_value_part[3], m_value_part[2], m_value_part[1], m_value_part[0]);
}

std::string address::to_string()const
{
    cstring_buffer output;
    to_string(&output);
    return output;
}

address address::operator<<(std::size_t n)const
{
    assert(n <= 32);
    return (m_value << n) & UPPER_LIMIT;
}

address address::operator>>(std::size_t n)const
{
    assert(n <= 32);
    return m_value >> n;
}

address address::operator&(address_mask mask)const
{
    return m_value & mask.value();
}

bool address::operator==(address x)const
{
    return m_value == x.m_value;
}

bool address::operator!=(address x)const
{
    return m_value != x.m_value;
}

bool address::first_bit()const
{
    return static_cast<bool>(m_value & 0x80000000);
}

} //namespace ip
} //namespace hopmod
