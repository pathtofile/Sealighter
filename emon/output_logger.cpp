#include "output_logger.h"
#include "provider.h"

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
        DWORD status = 0;
        status = EventWriteEMON_REPORT_EVENT(eventString.c_str());
        if (status != ERROR_SUCCESS) {
            printf("Error %ul line %d\n", status, __LINE__);
            return;
        }
    }
}

//void test_logging()
//{
//    DWORD status = 0;
//    status = EventRegisterEMon();
//    if (status != ERROR_SUCCESS) {
//        printf("Error %ul line %d\n", status, __LINE__);
//        return;
//    }
//
//    const char* json_data = "{ \"a\": \"b\" }";
//    status = EventWriteEMON_REPORT_EVENT(json_data);
//    if (status != ERROR_SUCCESS) {
//        printf("Error %ul line %d\n", status, __LINE__);
//        return;
//    }
//
//    status = EventUnregisterEMon();
//    if (status != ERROR_SUCCESS) {
//        printf("Error %ul line %d\n", status, __LINE__);
//        return;
//    }
//}