/*
    
*/
#ifndef FUNGU_STREAMS_HPP
#define FUNGU_STREAMS_HPP

#include <cstdio>
#include <cassert>
#include <cstring>
#include <vector>
#include <algorithm>

namespace fungu{
namespace stream{

/*
    Abstract base class for an output device
*/
class sink
{
public:
    enum status_code
    {
        WRITABLE = 0,   // Ready
        CLOSED,         // Not open
        FULL,           // Cannot write anymore
        ERROR           // Write failure
    };
    sink(){}
    virtual ~sink(){}
    virtual status_code status()const=0;
    virtual std::size_t write(const char *, std::size_t)=0;
private:
    sink(const sink &);
};

class file_sink:public sink
{
public:
    file_sink(FILE * file)
     :m_file(file)
    {
        
    }
    
    enum open_mode
    {
        TRUNCATE = 0,
        APPEND
    };
    
    file_sink(const char * filename, open_mode mode = TRUNCATE)
    {
        const char * mode_string;
        switch(mode)
        {
            case TRUNCATE: 
                mode_string = "w";
                break;
            case APPEND:
                mode_string = "a";
                break;
            default:
                mode_string = "";
        }
        
        m_file = fopen(filename, mode_string);
    }
    
    ~file_sink()
    {
        if(m_file) fclose(m_file);
    }
    
    void detach()
    {
        m_file = NULL;
    }
    
    status_code status()const
    {
        if(!m_file) return CLOSED;
        if(ferror(m_file)) return ERROR;
        return WRITABLE;
    }
    
    std::size_t write(const char * data, std::size_t datalen)
    {
        if(status() != WRITABLE) return 0;
        return fwrite(data, datalen, 1, m_file);
    }
private:
    FILE * m_file;
};

class memory_sink:public sink
{
public:
    memory_sink(char * start, char * end)
     :m_start(start), m_cursor(m_start), m_end(end)
    {
        assert(start && end);
    }
    
    status_code status()const
    {
        if(m_cursor == m_end) return FULL;
        return WRITABLE;
    }
    
    std::size_t write(const char * data, std::size_t datalen)
    {
        std::size_t remaining = m_end - m_cursor;
        datalen = std::min(datalen, remaining);
        memcpy(m_cursor, data, datalen);
        m_cursor += datalen;
        return datalen;
    }
private:
    char * m_start;
    char * m_cursor;
    char * m_end;
};

class char_vector_sink:public sink
{
public:
    char_vector_sink(std::size_t reserve = 0)
    {
        m_vector.reserve(reserve);
    }
    status_code status()const{return WRITABLE;}
    std::size_t write(const char * data, std::size_t datalen)
    {
        m_vector.resize(m_vector.size() + datalen);
        std::copy(data, data + datalen, m_vector.end() - datalen);
        return datalen;
    }
    const char * data()const{return &(*m_vector.begin());}
    std::size_t size()const{return m_vector.size();}
private:
    std::vector<char> m_vector;
};

class null_sink:public sink
{
public:
    status_code status()const{return WRITABLE;}
    std::size_t write(const char *, std::size_t datalen){return datalen;}
};

}//namespace stream
}//namespace fungu

#endif
