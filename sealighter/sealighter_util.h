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
    Helper to convert string to a lowercase version of that string
*/
std::string convert_str_str_lowercase(
    const std::string& from
);

/*
    Helper to convert wstring to a lowercase version of that string
*/
std::wstring convert_wstr_wstr_lowercase(
    const std::wstring& from
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
    Helper to convert string to widestring lowercase
*/
std::wstring convert_str_wstr_lowercase(
    const std::string& from
);

/*
    Helper to convert string to lowercase byte vector
*/
std::vector<BYTE> convert_str_bytes_lowercase(
    const std::string& from
);

/*
    Helper to convert string to lowercase widechar byte vector
*/
std::vector<BYTE> convert_str_wbytes_lowercase(
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
    const LARGE_INTEGER from
);


/*
    Helper to convert FILETIME to string
*/
std::string convert_filetime_string
(
    const FILETIME from
);


/*
    Helper to convert SYSTEMTIME to string
*/
std::string convert_systemtime_string
(
    const SYSTEMTIME from
);


/*
    Helper to convert byte array to string,
    treating the bytes as a GUID
*/
std::string convert_guid_str
(
    const GUID& from
);

/*
    Helper to convert widestring to GUID.
    If this fails the GUID will be NULL_GUID
*/
GUID convert_wstr_guid
(
    std::wstring from
);

/*
    Helper to convert string to GUID.
    If this fails the GUID will be NULL_GUID
*/
GUID convert_str_guid
(
    std::string from
);


/*
    Helper to convert byte array to string,
    treating the bytes as a SID
*/
std::string convert_bytes_sidstring
(
    const std::vector<BYTE>& from
);


/*
    Helper to convert byte array to a
    hexidecimal string representation
    (will *not* have a leading "0x")
*/
std::string convert_bytes_hexstring
(
    const std::vector<BYTE>& from
);


/*
    Helper to convert byte array to an int
*/
int convert_bytes_sint32
(
    const std::vector<BYTE>& from
);


/*
    Helper to convert byte array to a boolean
    Simply converts to an int, then checks does
     "if(value)..."
*/
bool convert_bytes_bool
(
    const std::vector<BYTE>& from
);

/*
    Checks if a file exits on disk
*/
bool file_exists
(
    std::string fileName
);