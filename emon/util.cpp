#include "util.h"
#include <mutex>

static std::mutex g_print_mutex;

void threaded_println(const char* to_print)
{
    g_print_mutex.lock();
    printf("%s\n", to_print);
    g_print_mutex.unlock();
}

std::string convert_wstr_str
(
    const std::wstring& from
)
{
    std::string to(from.begin(), from.end());
    return to;
}

std::wstring convert_str_wstr
(
    const std::string& from
)
{
    std::wstring to(from.begin(), from.end());
    return to;
}

std::string convert_guid_str
(
    const GUID& in_guid
)
{
    char guid_string[39]; // 2 braces + 32 hex chars + 4 hyphens + null terminator
    snprintf(
        guid_string, sizeof(guid_string),
        "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
        in_guid.Data1, in_guid.Data2, in_guid.Data3,
        in_guid.Data4[0], in_guid.Data4[1], in_guid.Data4[2],
        in_guid.Data4[3], in_guid.Data4[4], in_guid.Data4[5],
        in_guid.Data4[6], in_guid.Data4[7]);
    
    return std::string(guid_string);
}

std::string convert_bytes_systemtimestring
(
    const std::vector<BYTE>& bytes
)
{
    // TODO: Convert SYSTEMTIMEs
    // Could call "SystemTimeToFileTime" first then use that function
    return convert_bytes_hexstring(bytes);
}
std::string convert_bytes_filetimestring
(
    const std::vector<BYTE>& bytes
)
{
    // TODO: Convert FILETIMEs
    return convert_bytes_hexstring(bytes);
}

std::string convert_bytes_sidstring
(
    const std::vector<BYTE>& bytes
)
{
    // TODO: Convert SIDs
    return convert_bytes_hexstring(bytes);
}

std::string convert_bytes_hexstring
(
    const std::vector<BYTE>& bytes
)
{
    std::ostringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (int c : bytes) {
        ss << std::setw(2) << c;
    }
    return ss.str();
}

int convert_bytes_int
(
    const std::vector<BYTE>& bytes
)
{
    int ret = 0;
    if (bytes.size() == 4) {
        ret = (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | (bytes[0]);
    }
    return ret;
}
