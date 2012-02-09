#ifndef HOPMOD_NET_ADDRESS_MASK_HPP
#define HOPMOD_NET_ADDRESS_MASK_HPP

#include "address.hpp"

namespace hopmod{
namespace ip{

class address_mask
{
public:
    address_mask(std::size_t);
    address::integral_type value()const;
    std::size_t bits()const;
    address_mask operator<<(std::size_t)const;
    bool operator==(address_mask)const;
private:
    address::integral_type m_value;
};

} //namespace ip
} //namespace hopmod

#endif
