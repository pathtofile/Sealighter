#pragma once
#include "sealighter_krabs.h"
#include "sealighter_json.h"


struct event_buffer_t {
    event_buffer_t()
    {}

    json json_event;
};


struct event_buffer_list_t {
    event_buffer_list_t
    (
        std::uint32_t id,
        std::uint32_t max
    )
        : event_id(id)
        , max_before_buffering(max)
        , event_count(0)
    {}

    const std::uint32_t event_id;
    const std::uint32_t max_before_buffering;

    std::uint32_t event_count;
    std::vector<std::string> properties_to_compare;
    std::vector<json> json_event_buffered;
};

struct sealighter_context_t {
    sealighter_context_t
    (
        std::string name,
        bool dump_event
    )
        : trace_name(name)
        , dump_raw_event(dump_event)
    {}

    const std::string trace_name;
    const bool dump_raw_event;
};

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

void add_buffered_list
(
    std::string trace_name,
    event_buffer_list_t buffered_list
);

void set_buffer_lists_timeout
(
    uint32_t timeout
);

void start_bufferring();

void stop_bufferring();
