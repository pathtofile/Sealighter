// EMon.cpp
#include "etw_handler.h"
#include <iostream>

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

int main
(
    int argc,
    char* argv[]
)
{
    int ret = 0;
    if (argc != 2)
    {
        printf("Usage: %s <config_file>\n", argv[0]);
        return 1;
    }
    if (!SetConsoleCtrlHandler(crl_c_handler, TRUE))
    {
        printf("Failed to set ctrl-C handler\n");
        return 2;
    }

    ret = trace_start(argv[1]);

    return ret;
}
