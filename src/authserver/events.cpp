#include "lua/event.hpp"

lua::event< std::tuple<> >                                         event_init("init");

lua::event< std::tuple<> >                                         event_started("started");
lua::event< std::tuple<int> >                                      event_shutdown("shutdown");
lua::event< std::tuple<> >                                         event_shutdown_scripting("shutdown_scripting");
lua::event< std::tuple<> >                                         event_reloadhopmod("reloadhopmo");

lua::event< std::tuple<const char *> >                             event_varchanged("varchanged");

lua::event< std::tuple<> >                                         event_sleep("sleep");
lua::event< std::tuple<> >                                         event_interval("interval");

lua::event< std::tuple<const char *, const char *, const char *, const char *> > event_adduser("adduser");
lua::event< std::tuple<const char *, const char *> >               event_deleteuser("deleteuser");
lua::event< std::tuple<> >                                         event_clearusers("clearusers");

void register_event_idents(lua::event_environment & env)
{
    lua::event_base * events[] = {
        & event_init,
        & event_started,
        & event_shutdown,
        & event_shutdown_scripting,
        & event_reloadhopmod,
        & event_varchanged,
        & event_sleep,
        & event_interval,
        & event_adduser,
        & event_deleteuser,
        & event_clearusers,
        NULL
    };
    
    env.register_event_idents(events);
}
