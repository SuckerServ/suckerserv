#ifdef BOOST_BUILD_PCH_ENABLED
#include "pch.hpp"
#endif

#include "hopmod.hpp"
#include "lib/handle_resolver.hpp"

boost::signals2::signal<void (int)> signal_shutdown;

void disconnect_all_slots()
{
    signal_shutdown.disconnect_all_slots();
}

