#ifndef HOPMOD_EVENTS_HPP
#define HOPMOD_EVENTS_HPP

#include "lua/event.hpp"

extern lua::event< std::tuple<> >                                         event_init;
extern lua::event< std::tuple<> >                                         event_started;
extern lua::event< std::tuple<int> >                                      event_shutdown;
extern lua::event< std::tuple<> >                                         event_shutdown_scripting;
extern lua::event< std::tuple<> >                                         event_reloadhopmod;
extern lua::event< std::tuple<const char *> >                             event_varchanged;
extern lua::event< std::tuple<> >                                         event_sleep;
extern lua::event< std::tuple<> >                                         event_interval;
extern lua::event< std::tuple<const char *, const char *, const char *, const char *> > event_adduser;
extern lua::event< std::tuple<const char *, const char *> >               event_deleteuser;
extern lua::event< std::tuple<> >                                         event_clearusers;


void register_event_idents(lua::event_environment &);

#endif

