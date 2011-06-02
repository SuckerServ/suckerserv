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

namespace datetimelib{

inline time_t now()
{
    return time(NULL);
}

inline const char * print_standard_date(time_t local_timestamp, char * buffer)
{
    tm * fields = localtime(&local_timestamp);
    fields->tm_mon++;
    fields->tm_year += 1900;
    
    sprintf(buffer,"%i-%02i-%02i",fields->tm_year, fields->tm_mon, fields->tm_mday);
    return buffer;
}

inline std::string print_standard_date(time_t local_timestamp)
{
    char buffer[11];
    return print_standard_date(local_timestamp, buffer);
}

inline const char * print_standard_time(time_t local_timestamp,char * buffer)
{
    tm * fields = localtime(&local_timestamp);
    fields->tm_mon++;
    fields->tm_year += 1900;
    
    int tz_off = fields->tm_gmtoff;
    char tz_sign = (tz_off < 0 ? '-' : '+');
    tz_off = abs(tz_off);
    int tz_hours = tz_off/3600;
    int tz_mins = (tz_off-(tz_hours*3600))/60;
    
    sprintf(buffer,"%02i:%02i:%02i%c%02i:%02i",fields->tm_hour, fields->tm_min, fields->tm_sec, tz_sign, tz_hours, tz_mins);
    return buffer;
}

inline std::string print_standard_time(time_t local_timestamp)
{
    char buffer[15];
    return print_standard_time(local_timestamp, buffer);
}

inline std::string print_standard_datetime(time_t ts)
{
    char datebuf[11];
    char timebuf[15];
    char final[30];
    
    sprintf(final,"%sT%s", print_standard_date(ts,datebuf), print_standard_time(ts,timebuf));
    return final;
}

inline std::string duration(int seconds)
{
    int hours = seconds / 3600;
    seconds -= hours * 3600;
    int mins = seconds / 60;
    seconds -= mins * 60;
    char buffer[100];
    sprintf(buffer,"%02i:%02i:%02i",hours, mins, seconds);
    return buffer;
}

inline std::string fduration(int seconds)
{
    int days = seconds / 86400;
    seconds -= days * 86400;
    int hours = seconds / 3600;
    seconds -= hours * 3600;
    int mins = seconds / 60;
    seconds -= mins * 60;
    
    char daysbuf[100];
    sprintf(daysbuf,"%i %s",days, (days == 1 ? "day" : "days"));
    
    char hoursbuf[100];
    sprintf(hoursbuf,"%i %s",hours, (hours == 1 ? "hour" : "hours"));
    
    char minsbuf[100];
    sprintf(minsbuf,"%i %s",mins, (mins == 1 ? "minute" : "minutes"));
    
    char secsbuf[100];
    sprintf(secsbuf,"%i %s",seconds, (seconds == 1 ? "second" : "seconds"));
    
    char final[200];
    
    if(days)
        sprintf(final,"%s %s",daysbuf, (hours ? hoursbuf :""));
    else if(hours)
        sprintf(final,"%s %s",hoursbuf, (mins ? minsbuf : ""));
    else if(mins)
        sprintf(final,"%s %s",minsbuf, (seconds ? secsbuf : ""));
    else
        sprintf(final,"%s",secsbuf);
    
    return final;
}

} //namespace detail

void register_datetime_functions(env & environment)
{
    static function <time_t ()> now_func(datetimelib::now);
    environment.bind_global_object(&now_func, FUNGU_OBJECT_ID("now"));
    
    static function <std::string (time_t)> psd_func((std::string (&)(time_t))datetimelib::print_standard_date);
    environment.bind_global_object(&psd_func, FUNGU_OBJECT_ID("date"));
    
    static function <std::string (time_t)> pst_func((std::string (&)(time_t))datetimelib::print_standard_time);
    environment.bind_global_object(&pst_func, FUNGU_OBJECT_ID("time"));
    
    static function <std::string (time_t)> psdt_func(datetimelib::print_standard_datetime);
    environment.bind_global_object(&psdt_func, FUNGU_OBJECT_ID("datetime"));
    
    static function <std::string (int)> duration_func(datetimelib::duration);
    environment.bind_global_object(&duration_func, FUNGU_OBJECT_ID("duration"));
    
    static function <std::string (int)> fduration_func(datetimelib::fduration);
    environment.bind_global_object(&fduration_func, FUNGU_OBJECT_ID("fduration"));
}

} //namespace corelib
} //namespace script
} //namespace fungu
