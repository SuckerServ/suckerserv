#ifndef HOPMOD_SCOPED_SETTING_HPP
#define HOPMOD_SCOPED_SETTING_HPP

template<typename T>
class scoped_setting
{
public:
    scoped_setting(T & var,const T & new_value)
     :m_old_value(var),m_var(var)
    {
        m_var = new_value;
    }
    ~scoped_setting(){m_var = m_old_value;}
private:
    T m_old_value;
    T & m_var;
};

#endif
