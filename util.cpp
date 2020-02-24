#include "util.h"

std::string convert_wstr_str
(
    std::wstring from
)
{
    std::string to(from.begin(), from.end());
    return to;
}

std::wstring convert_str_wstr
(
    std::string from
)
{
    std::wstring to(from.begin(), from.end());
    return to;
}

std::string convert_bytes_filetimestring
(
    const std::vector<BYTE>& bytes
)
{
    return "TODO:FILETIME";
}

std::string convert_bytes_guidstring
(
    const std::vector<BYTE>& bytes
)
{
    return "TODO:GUID";
}

std::string convert_bytes_sidstring
(
    const std::vector<BYTE>& bytes
)
{
    return "TODO:SID";
}