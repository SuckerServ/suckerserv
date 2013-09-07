#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "hopmod.hpp"
#include "free_function_scheduler.hpp"

#include <fungu/script.hpp>
#include <fungu/script/lua/lua_function.hpp>
using namespace fungu;

static free_function_scheduler free_scheduled;

static int sched_free_lua_function(lua_State * L, bool);
static inline int sched_free_lua_sleep(lua_State *);
static inline int sched_free_lua_interval(lua_State *);
static script::any sched_free_cs_function(bool, script::env_object::call_arguments &, script::env_frame *);
static void cancel_free_scheduled(int);
static void cancel_timer(int);

void init_scheduler()
{
    static script::function<script::raw_function_type> free_sleep(boost::bind(sched_free_cs_function, false, _1, _2));
    get_script_env().bind_global_object(&free_sleep, FUNGU_OBJECT_ID("sleep"));
    
    static script::function<script::raw_function_type> free_interval(boost::bind(sched_free_cs_function, true, _1, _2));
    get_script_env().bind_global_object(&free_interval, FUNGU_OBJECT_ID("interval"));
    
    static script::function<void (int)> cancel_timer_func(cancel_timer);
    get_script_env().bind_global_object(&cancel_timer_func, FUNGU_OBJECT_ID("cancel_timer"));
    
    register_lua_function(sched_free_lua_sleep, "sleep");
    register_lua_function(sched_free_lua_interval, "interval");
    
    signal_shutdown.connect(cancel_free_scheduled, boost::signals2::at_front);
}

void cancel_free_scheduled(int)
{
    free_scheduled.cancel_all();
}

int call_lua_function(script::env_object::shared_ptr func, script::env * e)
{
    try
    {
        script::env_frame frame(e);
        std::vector<script::any> args;
        script::callargs empty(args);
        return script::lexical_cast<int>(func->call(empty,&frame));
    }
    catch(script::error err)
    {
        script::source_context * ctx = (e->get_source_context() ? e->get_source_context()->clone() : NULL);
        report_script_error(new script::error_trace(err,const_string(), ctx));
        return -1;
    }
    catch(script::error_trace * errinfo)
    {
        report_script_error(errinfo);
        return -1;
    }
}

int sched_free_lua_function(lua_State * L, bool repeat)
{
    int countdown = luaL_checkint(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);
    lua_pushvalue(L, 2);
    
    script::env_object::shared_ptr luaFunctionObject = new script::lua::lua_function(L);
    luaFunctionObject->set_adopted();
    
    int id = free_scheduled.schedule(boost::bind(call_lua_function, luaFunctionObject, &get_script_env()), countdown, repeat);
    lua_pushinteger(L, id);
    return 1;
}

int sched_free_lua_sleep(lua_State * L)
{
    return sched_free_lua_function(L, false);
}

int sched_free_lua_interval(lua_State * L)
{
    return sched_free_lua_function(L, true);
}

int call_cs_function(script::code_block code, script::env * e)
{
    script::env_frame frame(e);
    
    try
    {
        return script::lexical_cast<int>(code.eval_each_expression(&frame));
    }
    catch(script::error err)
    {
        report_script_error(new script::error_trace(err,const_string(),code.get_source_context()->clone()));
        return -1;
    }
    catch(script::error_trace * errinfo)
    {
        report_script_error(errinfo);
        return -1;
    }
}

script::any sched_free_cs_function(bool repeat, script::env_object::call_arguments & args, script::env_frame * frame)
{
    script::callargs_serializer cs(args, frame);
    
    int countdown = cs.deserialize(args.front(),type_tag<int>());
    args.pop_front();
    
    script::code_block code = cs.deserialize(args.front(),type_tag<script::code_block>());
    args.pop_front();
    
    int id = free_scheduled.schedule(boost::bind(call_cs_function, code, frame->get_env()), countdown, repeat);
    return id;
}

void sched_callback(int (* fun)(void *),void * closure)
{
    //FIXME memory leak for closure when job is cancelled
    free_scheduled.schedule(boost::bind(fun, closure), 0);
}

void sched_callback(int (* fun)(void *), void * closure, int delay)
{
    free_scheduled.schedule(boost::bind(fun, closure), delay);
}

void update_scheduler(int timenow)
{
    free_scheduled.update(timenow);
}

static void cancel_timer(int job_id)
{
    free_scheduled.cancel(job_id);
}
