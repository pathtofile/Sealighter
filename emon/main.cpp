#include "etw_handler.h"
#include "output_logger.h"
#include "provider.h"
#include <iostream>
#include <Windows.h>

BOOL WINAPI crl_c_handler
(
    DWORD fdwCtrlType
)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
        trace_stop();
        return TRUE;
    }
    return FALSE;
}

DWORD setup()
{
    DWORD status = 0;
    // Setup Event Logging
    status = EventRegisterEMon();
    if (status != ERROR_SUCCESS) {
        printf("Error registering event log: %ul\n", status);
        status = __LINE__;
        return status;
    }

    // Add ctrl+C handler to make sure we stop the trace
    if (!SetConsoleCtrlHandler(crl_c_handler, true)) {
        printf("failed to set ctrl-c handler\n");
        status = __LINE__;
        return status;
    }

    return status;
}

void teardown()
{
    DWORD status;
    status = EventUnregisterEMon();
    if (status != ERROR_SUCCESS) {
        printf("Error unregisting event logger: %ul\n", status);
    }
}

int main
(
    int argc,
    char* argv[]
)
{
    DWORD status = 0;
    if (2 != argc) {
        printf("usage: %s <config_file>\n", argv[0]);
        return 1;
    }

    status = setup();
    if (0 != status) {
        printf("error during setup %d\n", status);
    }
    else {
        status = trace_start(argv[1]);
        if (0 != status) {
            printf("error %d\n", status);
        }
    }

    // Attempt to cleanup
    teardown();

    return status;
}
