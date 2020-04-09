#pragma once
#include "etw_handler.h"

enum class Output_format
{
    output_stdout,
    output_event_log,
    output_all
};

struct cmp_wstr
{
    bool operator()(const wchar_t* a, const wchar_t* b) const
    {
        return std::wcscmp(a, b) < 0;
    }
};

void threaded_println(const char* to_print);

std::string convert_wstr_str
(
    const std::wstring& from
);

std::wstring convert_str_wstr(
    const std::string& from
);

std::string convert_bytes_filetimestring
(
    const std::vector<BYTE>& bytes
);

std::string convert_guid_str
(
    const GUID& in_guid
);

std::string convert_bytes_sidstring
(
    const std::vector<BYTE>& bytes
);


std::string convert_bytes_hexstring
(
    const std::vector<BYTE>& bytes
);

int convert_bytes_int
(
    const std::vector<BYTE>& bytes
);