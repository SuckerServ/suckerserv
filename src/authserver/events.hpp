#ifndef HOPMOD_EVENTS_HPP
#define HOPMOD_EVENTS_HPP

#include "lua/event.hpp"

extern lua::event< boost::tuple<> >                                         event_init;
extern lua::event< boost::tuple<> >                                         event_started;
extern lua::event< boost::tuple<int> >                                      event_shutdown;
extern lua::event< boost::tuple<> >                                         event_shutdown_scripting;
extern lua::event< boost::tuple<> >                                         event_reloadhopmod;
extern lua::event< boost::tuple<const char *> >                             event_varchanged;
extern lua::event< boost::tuple<> >                                         event_sleep;
extern lua::event< boost::tuple<> >                                         event_interval;
extern lua::event< boost::tuple<const char *, const char *, const char *, const char *> > event_adduser;
extern lua::event< boost::tuple<const char *, const char *> >               event_deleteuser;
extern lua::event< boost::tuple<> >                                         event_clearusers;


void register_event_idents(lua::event_environment &);

#endif

