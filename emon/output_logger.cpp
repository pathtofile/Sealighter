#include "output_logger.h"

void log_event
(
    output_formats  output_format,
    std::string     eventString
)
{
    if (output_format == output_all || output_stdout)
    {
        printf("%s\n", eventString.c_str());
        printf("--------------------\n");
    }

    if (output_format == output_all || output_event_log)
    {

    }
}
