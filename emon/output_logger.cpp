#include "output_logger.h"

void log_event
(
    Output_format  output_format,
    std::string     eventString
)
{
    if (Output_format::output_all  == output_format || Output_format::output_stdout == output_format) {
        threaded_println(eventString.c_str());
    }

    if (Output_format::output_all == output_format || Output_format::output_event_log == output_format) {
        // TODO: Log to event log
    }
}
