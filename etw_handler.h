#pragma once

// Disable Warning for KrabsETW
// and it pollutes our build script with a warning
// we won't deal with because its not our code
#pragma warning( push )
#pragma warning( disable : 4244 )
#include "krabs.hpp"
#pragma warning( pop )

using namespace krabs;

void handleEvent
(
    const EVENT_RECORD&     record,
    const trace_context&    trace_context
);

int trace_start
(
    std::string config_path
);


void trace_stop( );
