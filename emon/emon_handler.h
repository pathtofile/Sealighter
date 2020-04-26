#pragma once
#include "emon_krabs.h"

/*
    Parse incoming events into JSON and output
*/
void handle_event
(
    const EVENT_RECORD&     record,
    const trace_context&    trace_context
);
