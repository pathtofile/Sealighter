// Utility helper functions
#pragma once
#include "emon_json.h"


/*  
    Hold whether we should be outputting the parsed JSON event
*/
enum class Output_format
{
    output_stdout,
    output_event_log,
    output_file
};
static Output_format g_output_format = Output_format::output_stdout;


/*
    Simple function to print a line to stdout, ensuring the output isn't
    interupted part way through by another print
*/
void threaded_println
(
    std::string to_print
);


/*
    Helper to convert json to string
*/
std::string convert_json_string
(
    json item,
    bool pretty_print
);


/*
    Helper to convert widestring to string
*/
std::string convert_wstr_str
(
    const std::wstring& from
);


/*
    Helper to convert string to widestring
*/
std::wstring convert_str_wstr(
    const std::string& from
);


/*
    Helper to convert byte array to string,
    treating the bytes as a FILETIME struct
*/
std::string convert_bytes_filetimestring
(
    const std::vector<BYTE>& bytes
);


/*
    Helper to convert byte array to string,
    treating the bytes as a SYSTEMTIME struct
*/
std::string convert_bytes_systemtimestring
(
    const std::vector<BYTE>& bytes
);


/*
    Helper to convert byte array to string,
    treating the bytes as a GUID
*/
std::string convert_guid_str
(
    const GUID& in_guid
);


/*
    Helper to convert byte array to string,
    treating the bytes as a SID
*/
std::string convert_bytes_sidstring
(
    const std::vector<BYTE>& bytes
);


/*
    Helper to convert byte array to a
    hexidecimal string representation
    (will *not* have a leading "0x")
*/
std::string convert_bytes_hexstring
(
    const std::vector<BYTE>& bytes
);


/*
    Helper to convert byte array to an int
*/
int convert_bytes_sint32
(
    const std::vector<BYTE>& bytes
);


/*
    Helper to convert byte array to a boolean
    Simply converts to an int, then checks does
     "if(value)..."
*/
bool convert_bytes_bool
(
    const std::vector<BYTE>& bytes
);

/*
    Checks if a file exits on disk
*/
bool file_exists
(
    std::string fileName
);