
#include <fungu/string.hpp>
#include <fungu/stringutils.hpp>
#include <cassert>
#include <cmath>
#include <string>
#include <cstring>

namespace fungu{

static const_string::const_iterator addr(const std::string & str, std::size_t index)
{
    return &str[index];
}

const_string::const_string()
 :m_firstc("\0"+1),m_lastc("\0")
{
    
}

const_string::const_string(const_iterator firstc, const_iterator lastc)
 :m_firstc(firstc),m_lastc(lastc)
{
    assert( firstc <= lastc || firstc-1==lastc /* empty string condition */);
    m_copy = std::string(m_firstc, m_lastc+1);
    m_firstc = &(*m_copy.begin());
    m_lastc = &(*m_copy.rbegin());
}

const_string::const_string(const std::string & src)
 :m_copy(src),
  m_firstc(&((const std::string &)m_copy)[0]),
  m_lastc(&((const std::string &)m_copy)[m_copy.length()-1])
{
    
}

const_string::const_string(const char * raw_string)
 :m_copy(raw_string),
  m_firstc(&((const std::string &)m_copy)[0]),
  m_lastc(&((const std::string &)m_copy)[m_copy.length()-1])
{
    
}

const_string::const_string(const const_string & src)
 :m_copy(src.m_copy),
  m_firstc(src.m_firstc),
  m_lastc(src.m_lastc)
{
    if (src.m_copy.empty() && src.length() > 0) {
        m_copy = std::string(m_firstc, m_lastc+1);
        m_firstc = &(*m_copy.begin());
        m_lastc = &(*m_copy.rbegin());
    } else {
        m_firstc = addr(m_copy,0) + (src.m_firstc - addr(src.m_copy,0));
        m_lastc = addr(m_copy,0) + (src.m_lastc - addr(src.m_copy,0));
    }
}

const_string::const_string(const std::pair<const char *, const char *> & src)
 :m_firstc(src.first), m_lastc(src.second)
{
    
}

const_string const_string::literal(const char * literalString)
{
    return const_string(literalString, literalString + std::char_traits<char>::length(literalString) - 1);
}

const_string::const_iterator const_string::begin()const
{
    return m_firstc;
}

const_string::const_iterator const_string::end()const
{
    return m_lastc + 1;
}

const_string::const_iterator const_string::null_const_iterator()
{
    return NULL;
}

std::size_t const_string::length()const
{
    return m_lastc - m_firstc +1;
}

const_string const_string::substring(const_iterator first, const_iterator last)const
{
    assert( first <= last &&
            first >= m_firstc && 
            first <= m_lastc &&
            last <= m_lastc );
    
    const_string tmp(m_copy);
    tmp.m_firstc = first;
    tmp.m_lastc = last;
    return tmp;
}

std::string const_string::copy()const
{
    return std::string(begin(), end());
}

bool const_string::operator<(const const_string & operand)const
{
    return string_detail::less_than(*this, operand);
}

bool const_string::operator==(const const_string & operand)const
{
    return string_detail::equals(*this, operand);
}

const char * const_string::c_str()const
{
    return begin();
}

const_string join(const_string * a, std::size_t n)
{
    std::string output;
    for(int i = 0; i < n; i++)
    {
        const_string s(a[i]);
        output.append(s.begin(), s.end());
    }
    return output;
}

const_string join(const const_string & a, const const_string & b)
{
    const_string arrayOfStrings[2];
    arrayOfStrings[0] = a;
    arrayOfStrings[1] = b;
    return join(arrayOfStrings, 2);
}

} //namespace fungu
