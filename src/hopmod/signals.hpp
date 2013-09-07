#ifndef HOPMOD_SIGNALS_HPP
#define HOPMOD_SIGNALS_HPP

#include <boost/signals2/signal.hpp>

// Generic Server Events
extern boost::signals2::signal<void (int)> signal_shutdown;

#endif

