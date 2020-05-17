// Utility helper functions
#pragma once
#include "sealighter_json.h"


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
    Helper to convert LARGE_INTEGER timestamp to string
    MSDN States this is:
        Time at which the information in this structure was updated,
        in 100-nanosecond intervals since midnight, January 1, 1601.

    This is the same as a FILETIME
*/
std::string convert_timestamp_string
(
    const LARGE_INTEGER timestamp
);


/*
    Helper to convert FILETIME to string
*/
std::string convert_filetime_string
(
    const FILETIME ftime
);


/*
    Helper to convert SYSTEMTIME to string
*/
std::string convert_systemtime_string
(
    const SYSTEMTIME stime
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