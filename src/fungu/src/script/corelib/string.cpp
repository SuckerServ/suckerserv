/*   
 *   The Fungu Scripting Engine
 *   
 *   Copyright (c) 2008-2009 Graham Daws.
 *
 *   Distributed under a BSD style license (see accompanying file LICENSE.txt)
 */

namespace fungu{
namespace script{
namespace corelib{
    
namespace stringlib{

inline int strcmp(const char * s1,const char * s2){return ::strcmp(s1,s2) == 0;}

inline any format(env_object::call_arguments & args,env_frame *)
{
    std::string result;
   
    const_string _format = args.safe_casted_front<const_string>();
    args.pop_front();
    
    std::size_t sumlength = _format.length();
    
    const_string subs[9];
    const_string * subptr = subs;
    while(!args.empty() && subptr < subs + 9)
    {
        *(subptr++) = args.casted_front<const_string>();
        args.pop_front();
        sumlength += (subptr - 1)->length();
    }
    
    int subsc = subptr - subs;
    
    result.reserve(sumlength);
    
    for(const_string::const_iterator it = _format.begin();
        it != _format.end(); it++)
    {
        if(*it == '%')
        {
            if(++it == _format.end()) 
                throw error(OPERATION_ERROR, boost::make_tuple(std::string("missing index number")));
            
            char digitc = *it;
            int n = digitc - '1';
            
            if(digitc < '1' || digitc > '9' || n > subsc - 1)
                throw error(OPERATION_ERROR, boost::make_tuple(std::string("invalid index number")));
            
            result.append(subs[n].copy());
        }
        else result.append(1,*it);
    }
    
    return result;
}

inline any concat(env_object::call_arguments & args,env_frame *)
{
    std::string result;
    while(!args.empty())
    {
        result += args.casted_front<const_string>().copy();
        result += ' ';
        args.pop_front();
    }
    return result;
}

inline any concatword(env_object::call_arguments & args,env_frame *)
{
    std::string result;
    while(!args.empty())
    {
        result += args.casted_front<const_string>().copy();
        args.pop_front();
    }
    return result;
}

inline int index_of(const char * search,const char * body)
{
    const char * pos = strstr(body,search);
    if(!pos) return -1;
    else return pos - body;
}

inline std::string substr(const std::string & src, int pos, int count)
{
    return src.substr(pos,count);
}

inline std::vector<std::string> split(const char * body,const char * delim)
{
    std::vector<std::string> result;
    
    const char * body_end = body + strlen(body);
    const char * start = body + strspn(body,delim);
    
    while(start < body_end)
    {
        const char * end = start + strcspn(start, delim);
        result.push_back(std::string(start,end));
        start = end + strspn(end, delim);
    }
    
    return result;
}

inline int strstr(const char * s1, const char * s2)
{
    const char * s3 = ::strstr(s1, s2);
    return (s3 ? s3 - s1 : -1);
}

std::string strreplace(const char * original, const char * oldsub, const char * newsub)
{
    std::string result(original);
    size_t oldsublen = strlen(oldsub);
    size_t newsublen = strlen(newsub);
    std::string::size_type pos = 0;
    
    do
    {
        pos = result.find(oldsub, pos);
        if(pos != std::string::npos) 
            result.replace(pos, oldsublen, newsub, newsublen);
    }while(pos != std::string::npos);

    return result;
}

} //namespace detail

void register_string_functions(env & environment)
{
    static function<int (const char *,const char *)> strcmp_func(stringlib::strcmp);
    environment.bind_global_object(&strcmp_func, FUNGU_OBJECT_ID("strcmp"));
    environment.bind_global_object(&strcmp_func, FUNGU_OBJECT_ID("streq"));
    
    static function<raw_function_type> format_func(stringlib::format);
    environment.bind_global_object(&format_func, FUNGU_OBJECT_ID("format"));
    
    static function<raw_function_type> concat_func(stringlib::concat);
    environment.bind_global_object(&concat_func, FUNGU_OBJECT_ID("concat"));
    
    static function<raw_function_type> concatword_func(stringlib::concatword);
    environment.bind_global_object(&concatword_func, FUNGU_OBJECT_ID("concatword"));
    
    static function<int (const char *,const char *)> index_of_func(stringlib::index_of);
    environment.bind_global_object(&index_of_func, FUNGU_OBJECT_ID("indexof"));
    
    static function<std::string (const std::string &,int,int)> substr_func(stringlib::substr);
    environment.bind_global_object(&substr_func, FUNGU_OBJECT_ID("substr"));
    
    static function<std::vector<std::string> (const char *,const char *)> split_func(stringlib::split);
    environment.bind_global_object(&split_func, FUNGU_OBJECT_ID("split"));
    
    static function<int (const char *,const char *)> strstr_func(stringlib::strstr);
    environment.bind_global_object(&strstr_func, FUNGU_OBJECT_ID("strstr"));
    
    static function<size_t (const char *)> strlen_func(strlen);
    environment.bind_global_object(&strlen_func, FUNGU_OBJECT_ID("strlen"));
    
    static function<std::string (const char *,const char *,const char *)> strreplace_func(stringlib::strreplace);
    environment.bind_global_object(&strreplace_func, FUNGU_OBJECT_ID("strreplace"));
}

} //namespace corelib
} //namespace script
} //namespace fungu

