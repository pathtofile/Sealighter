#pragma once
#include "emon_util.h"


/*
    Log an event to stdout, file, or Event log
*/
void log_event
(
    Output_format  output_format,
    std::string    event_string
);
