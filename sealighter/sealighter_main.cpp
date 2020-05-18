#include <iostream>
#include "sealighter_controller.h"
#include "sealighter_errors.h"

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
        return SEALIGHTER_ERROR_NOCONFIG;
    }

    status = run_sealighter(argv[1]);

    return status;
}
