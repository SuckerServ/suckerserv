#include "address_mask.hpp"
#include <bitset>
#include <cmath>
#include <cassert>

namespace hopmod{
namespace ip{

address_mask::address_mask(std::size_t bits)
{
    assert(bits >= 0 && bits < 33);
    m_value = static_cast<address::integral_type>((pow(2, bits)-1)) << (32 - bits);
}

address::integral_type address_mask::value()const
{
    return m_value;
}

std::size_t address_mask::bits()const
{
    std::bitset<address::bits> bits(m_value);
    return bits.count();
}

address_mask address_mask::operator<<(std::size_t n)const
{
    assert(n <= 32);
    address_mask new_mask(0);
    new_mask.m_value = (m_value << n) & address::UPPER_LIMIT;
    return new_mask;
}

bool address_mask::operator==(address_mask x)const
{
    return m_value == x.m_value;
}

} //namespace ip
} //namespace hopmod
