#include <iostream>
#include "emon_controller.h"
#include "emon_errors.h"

/*
    Main entrypoint
*/
int main
(
    int argc,
    char* argv[]
)
{
    int status = 0;
    if (2 != argc) {
        printf("usage: %s <config_file>\n", argv[0]);
        return EMON_ERROR_NOCONFIG;
    }

    status = run_emon(argv[1]);

    return status;
}
