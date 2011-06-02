/*
    Copyright (C) 2009 Graham Daws
*/
#ifndef HOPMOD_FREE_FUNCTION_SCHEDULER_HPP
#define HOPMOD_FREE_FUNCTION_SCHEDULER_HPP

#include <queue>
#include <boost/function.hpp>
#include <limits>

class free_function_scheduler
{
    struct job;
    struct jobs_queue_entry;
    typedef std::map<int,job> jobs_map;
    typedef std::priority_queue<jobs_queue_entry> jobs_queue;
public:
    free_function_scheduler()
    :m_job_id(0), m_last_update(0)
    {
        
    }
    
    template<typename NullaryFunction>
    int schedule(NullaryFunction func, int countdown, bool repeat = false)
    {
        job new_job;
        new_job.function = func;
        new_job.launch = m_last_update + countdown;
        new_job.interval = countdown;
        new_job.repeat = repeat;
        
        bool cycled = false;
        std::pair<jobs_map::iterator, bool> insert;
        do
        {
            if(m_job_id == std::numeric_limits<int>::max())
            {
                if(cycled) return -1;
                else cycled = true;
            }
            
            insert = m_jobs.insert(jobs_map::value_type(m_job_id++, new_job));
            
            m_jobs_queue.push(insert.first);
            set_next_job();
            
        }while(!insert.second);
        
        return insert.first->first;
    }
    
    free_function_scheduler & update(int totalmillis)
    {
        m_last_update = totalmillis;
       
        while(m_next_job && totalmillis >= m_next_job->launch)
        {
            bool reuse = false;
            const job * this_job = m_next_job;
            jobs_map::iterator this_job_iter = m_next_job_iter;
            
            if(!this_job->cancelled)
            {
                int status = this_job->function();
                if(this_job->repeat && status != -1)
                {
                    this_job_iter->second.launch += this_job->interval;
                    m_jobs_queue.push(m_jobs_queue.top());
                    reuse = true;
                }
            }
            
            if(!reuse) m_jobs.erase(m_next_job_iter);
            
            m_jobs_queue.pop();
            set_next_job();
        }
        
        return *this;
    }
    
    void cancel_all()
    {
        m_jobs.clear();
        m_jobs_queue = jobs_queue();
        m_next_job = NULL;
    }
    
    void cancel(int job_id)
    {
        jobs_map::iterator it = m_jobs.find(job_id);
        if(it == m_jobs.end()) return;
        it->second.cancelled = true;
    }
private:
    
    struct job
    {
        int launch;
        int interval;
        bool repeat;
        bool cancelled;
        boost::function0<int> function;
        
        job():launch(0), interval(0), repeat(false), cancelled(false){}
        
        bool operator<(const job & y)const
        {
            return launch > y.launch;
        }
    };
    
    struct jobs_queue_entry
    {
        jobs_map::iterator job_iter;
        jobs_queue_entry(jobs_map::iterator iter):job_iter(iter){}
        bool operator<(const jobs_queue_entry & y)const
        {
            return job_iter->second.launch > y.job_iter->second.launch;
        }
    };
    
    void set_next_job()
    {
        if(m_jobs_queue.empty())
        {
            m_next_job = NULL;
            return;
        }
        
        m_next_job = &m_jobs_queue.top().job_iter->second;
        m_next_job_iter = const_cast<jobs_queue_entry &>(m_jobs_queue.top()).job_iter;
    }
    
    int m_job_id;
    int m_last_update;
    jobs_map m_jobs;
    jobs_queue m_jobs_queue;
    const job * m_next_job;
    jobs_map::iterator m_next_job_iter;
};

#endif
