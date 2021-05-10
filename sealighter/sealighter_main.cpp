#include <iostream>
#include <fstream>
#include <string>
#include "sealighter_handler.h"
#include "sealighter_errors.h"
#include "sealighter_util.h"
#include "sealighter_controller.h"
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
        log_messageA("usage: %s <config_file>\n", argv[0]);
        return SEALIGHTER_ERROR_NOCONFIG;
    }

    std::string config_path = argv[1];

    if (!file_exists(config_path)) {
        log_messageA("Error: Config file doesn't exist\n");
        return SEALIGHTER_ERROR_MISSING_CONFIG;
    }
    std::ifstream  config_stream(config_path);
    std::string config_string((std::istreambuf_iterator<char>(config_stream)),
        (std::istreambuf_iterator<char>()));
    config_stream.close();

    status = run_sealighter(config_string);

    return status;
}
