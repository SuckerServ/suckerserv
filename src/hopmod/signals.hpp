#ifndef HOPMOD_SIGNALS_HPP
#define HOPMOD_SIGNALS_HPP

#include <boost/signal.hpp>

// Generic Server Events
extern boost::signal<void (int)> signal_shutdown;

#endif

