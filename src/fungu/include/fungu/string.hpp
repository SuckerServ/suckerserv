/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_STRING_HPP
#define FUNGU_STRING_HPP

#include <string>

#define FUNGU_LITERAL_STRING(str) str,str+sizeof(str)-2
#define FUNGU_EMPTY_STRING "\0"+1,"\0"

namespace fungu{
    
class const_string
{
public:
    typedef const char value_type;
    typedef value_type * const_iterator;
    typedef std::size_t index_type;
    
    const_string();
    const_string(const_iterator firstc, const_iterator lastc);
    const_string(const std::string & src);
    const_string(const char * raw_string);
    const_string(const const_string & src);
    const_string(const std::pair<const char *, const char *> &);

    static const_string literal(const char * literalString);

    const_iterator begin()const;
    const_iterator end()const;
    static const_iterator null_const_iterator();

    std::size_t length()const;

    const_string substring(const_iterator first, const_iterator last)const;
    std::string copy()const;
    
    const char * c_str()const;
    const std::string & std_string()const;
    
    bool operator<(const const_string & operand)const;
    bool operator==(const const_string & operand)const;
private:
    std::string m_copy;
    const_iterator m_firstc;
    const_iterator m_lastc;
};

const_string join(const const_string &, const const_string &);
const_string join(const_string *, std::size_t);

} //namespace fungu

#endif
