#include "emon_krabs.h"
#include "emon_util.h"
#include "emon_logger.h"
#include "provider.h"


void log_event
(
    Output_format   output_format,
    std::string     event_string
)
{
    if (Output_format::output_stdout == output_format) {
        threaded_println(event_string);
    }
    else if (Output_format::output_event_log == output_format) {
        DWORD status = 0;
        status = EventWriteEMON_REPORT_EVENT(event_string.c_str());
        if (status != ERROR_SUCCESS) {
            printf("Error %ul line %d\n", status, __LINE__);
            return;
        }
    }
    else if (Output_format::output_file == output_format) {
        // TODO: Write to file
    }
}
