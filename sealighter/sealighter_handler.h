#pragma once
#include "sealighter_krabs.h"

/*
    Parse incoming events into JSON and output
*/
void handle_event
(
    const EVENT_RECORD&     record,
    const trace_context&    trace_context
);

/*
    Parse incoming events into JSON and output
*/
void handle_event_context
(
    const EVENT_RECORD& record,
    const trace_context& trace_context,
    std::shared_ptr<struct sealighter_context_t> event_context
);

/*
    Hold whether we should be outputting the parsed JSON event
*/
enum Output_format
{
    output_stdout,
    output_event_log,
    output_file
};

/*
    Log an event to stdout, file, or Event log
*/
void log_event
(
    std::string    event_string
);


/*
    Create stream to write to output file
*/
int setup_logger_file
(
    std::string filename
);

/*
    Close stream to output file
*/
void teardown_logger_file();

/*
    Stores the global output format
*/
void set_output_format
(
    Output_format format
);
