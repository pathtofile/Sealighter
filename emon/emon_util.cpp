#include "emon_krabs.h"
#include <sstream>
#include <fstream>
#include "emon_json.h"
#include "emon_util.h"


std::string convert_json_string
(
    json item,
    bool pretty_print
)
{
    if (pretty_print) {
        return item.dump(4, ' ', false, nlohmann::detail::error_handler_t::replace);
    }
    else {
        return item.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace);
    }
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
#define MAX_NAME 256
    char user_name[MAX_NAME] = "";
    char domain_name[MAX_NAME] = "";
    DWORD user_name_size = MAX_NAME;
    DWORD domain_name_size = MAX_NAME;
    SID_NAME_USE name_use;
    const BYTE* data = (bytes.data());
    std::string output;
    if (LookupAccountSidA(NULL, (PSID)data, user_name, &user_name_size, domain_name, &domain_name_size, &name_use)) {
        output = user_name;
        output += "\\";
        output += domain_name;
    }
    else {
        // Fallback to printing the raw bytes
        output = convert_bytes_hexstring(bytes);
    }
    return output;
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


int convert_bytes_sint32
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

bool convert_bytes_bool
(
    const std::vector<BYTE>& bytes
)
{
    if (convert_bytes_sint32(bytes)) {
        return true;
    }
    return false;
}


bool file_exists
(
    std::string fileName
)
{
    std::ifstream infile(fileName);
    return infile.good();
}
