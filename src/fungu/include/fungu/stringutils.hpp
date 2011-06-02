/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */
#ifndef FUNGU_STRINGUTILS_HPP
#define FUNGU_STRINGUTILS_HPP

#include "string.hpp"
#include <string>
#include <string.h>
#include <boost/functional/hash.hpp>
#include <ostream>
#include <istream>

namespace fungu{

namespace string_detail{

template<typename T>
struct const_string_traits
{
    typedef typename T::value_type char_type;
    typedef typename T::const_iterator const_iterator;
    static const_iterator begin(const T & str){return str.begin();}
    static const_iterator end(const T & str){return str.end();}
};

template<> 
struct const_string_traits<const char *>
{
    typedef char char_type;
    typedef const char * const_iterator;
    static const_iterator begin(const char * str){return str;}
    static const_iterator end(const char * str){return str+strlen(str);}
};

template<typename ST1,typename ST2>
int comparison(ST1 str1,ST2 str2)
{
    std::size_t str1_len=0;
    std::size_t str2_len=0;
    
    typename const_string_traits<ST1>::const_iterator str1_it =
        const_string_traits<ST1>::begin(str1);
    typename const_string_traits<ST1>::const_iterator str1_end =
        const_string_traits<ST1>::end(str1);
    
    typename const_string_traits<ST2>::const_iterator str2_it =
        const_string_traits<ST2>::begin(str2);
    typename const_string_traits<ST2>::const_iterator str2_end =
        const_string_traits<ST2>::end(str2);
    
    for( ; 
        str1_it!=str1_end && str2_it!=str2_end; 
        ++str1_it, ++str2_it,++str1_len, ++str2_len )
    {
        if( *str1_it != *str2_it )
        {
            if( *str1_it < *str2_it ) return -1;
            else return 1;
        }
    }
    
    if(str1_it!=str1_end) str1_len++;
    else if(str2_it!=str2_end) str2_len++;
    
    if(str1_len==str2_len) return 0;
    else
    {
        if(str1_len < str2_len) return -1;
        else return 1;
    }
}

template<typename ST1,typename ST2>
inline bool equals(ST1 str1,ST2 str2)
{
    return comparison(str1,str2)==0;
}

template<typename ST1,typename ST2>
inline bool less_than(ST1 str1,ST2 str2)
{
    return comparison(str1,str2)==-1;
}

template<typename ST1,typename ST2>
inline bool more_than(ST1 str1,ST2 str2)
{
    return comparison(str1,str2)==1;
}

template<typename T,bool (& O)(T,T)>
struct comparator
{
    bool operator()(T s1,T s2)const
    {
        return O(s1,s2);
    }
};

} //namespace string_detail

/**
    @brief Trim newline characters from end of string.
*/
inline std::string * trim_newlines(std::string * str)
{
    std::size_t len=str->length();
    if(len)
    {
        std::string::iterator erase_first=str->end();

        char last=(*str)[len-1];
        if(last=='\n')
        {
            --erase_first;
            if(len > 1 && (*str)[len-2]=='\r') --erase_first;
        }
        else if(last=='\r') --erase_first;
        
        if(erase_first!=str->end())
        {
            str->erase(erase_first,str->end());
            trim_newlines(str);
        }
    }
    
    return str;
}

inline bool scan_newline(const char ** readptr)
{
    if(**readptr == '\r' && *((*readptr)+1)=='\n')
        *readptr += 2;
    else
        if(**readptr == '\n')
            (*readptr)++;
        else
            return false;
    return true;
}

inline size_t hash_value(const_string str)
{
    return boost::hash_range(str.begin(),str.end());
}

inline std::string & operator+=(std::string & dst, const const_string & src)
{
    dst.append(src.begin(),src.end());
    return dst;
}

inline std::ostream & operator<<(std::ostream & dst, const const_string & src)
{
    dst.write(src.begin(),src.length());
    return dst;
}

inline std::istream & operator>>(std::istream & src, const_string & dst)
{
    std::string str;
    str.reserve(32);
    while(src.good())
    {
        char e;
        src.read(&e, sizeof(char));
        if(src.good()) str.append(1, e);
    }
    dst = const_string(str);
    src.clear();
    return src;
}

} //namespace fungu

#endif
