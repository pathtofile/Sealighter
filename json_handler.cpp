#include "etw_handler.h"
#include "util.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

std::string parse_event_to_json
(
    const   EVENT_RECORD&           record,
    const   krabs::trace_context&   trace_context,
            krabs::schema           schema,
    const   bool                    pretty_print
)
{
    krabs::parser parser(schema);
    json json_header = {
        { "event_id", schema.event_id() },
        { "event_name", convert_wstr_str(schema.event_name()) },
        { "thread_id", schema.thread_id() },
        { "timestamp", schema.timestamp().QuadPart },
        { "event_flags", schema.event_flags() },
        { "event_opcode", schema.event_opcode() },
        { "event_version", schema.event_version() },
        { "process_id", schema.process_id()},
        { "provider_name", convert_wstr_str(schema.provider_name()) }
    };
    json json_event = {{"header", json_header}};
    json json_payload;

    for (krabs::property& prop : parser.properties())
    {
        std::wstring prop_name_wstr = prop.name();
        std::string prop_name = convert_wstr_str(prop_name_wstr);

        switch (prop.type())
        {
        case TDH_INTYPE_ANSISTRING:
            json_payload[prop_name] = parser.parse<std::string>(prop_name_wstr);
            break;
        case TDH_INTYPE_UNICODESTRING:
            json_payload[prop_name] = convert_wstr_str(parser.parse<std::wstring>(prop_name_wstr));
            break;
        case TDH_INTYPE_INT8:
            json_payload[prop_name] = parser.parse<std::int8_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT8:
            json_payload[prop_name] = parser.parse<std::uint8_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT16:
            json_payload[prop_name] = parser.parse<std::int16_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT16:
            json_payload[prop_name] = parser.parse<std::uint16_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT32:
            json_payload[prop_name] = parser.parse<std::int32_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT32:
            json_payload[prop_name] = parser.parse<std::uint32_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT64:
            json_payload[prop_name] = parser.parse<std::int64_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT64:
            json_payload[prop_name] = parser.parse<std::uint64_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_FLOAT:
            json_payload[prop_name] = parser.parse<std::float_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_DOUBLE:
            json_payload[prop_name] = parser.parse<std::double_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_BOOLEAN:
            json_payload[prop_name] = parser.parse<boolean>(prop_name_wstr);
            break;
        case TDH_INTYPE_BINARY:
            json_payload[prop_name] = parser.parse<krabs::binary>(prop_name_wstr).bytes();
            break;
        case TDH_INTYPE_GUID:
            json_payload[prop_name] = convert_bytes_guidstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_FILETIME:
            json_payload[prop_name] = convert_bytes_filetimestring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_SID:
            json_payload[prop_name] = convert_bytes_sidstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_POINTER:
        case TDH_INTYPE_SYSTEMTIME:
        case TDH_INTYPE_HEXINT32:
        case TDH_INTYPE_HEXINT64:
        case TDH_INTYPE_MANIFEST_COUNTEDSTRING:
        case TDH_INTYPE_MANIFEST_COUNTEDANSISTRING:
        case TDH_INTYPE_RESERVED24:
        case TDH_INTYPE_MANIFEST_COUNTEDBINARY:
        case TDH_INTYPE_COUNTEDSTRING:
        case TDH_INTYPE_COUNTEDANSISTRING:
        case TDH_INTYPE_REVERSEDCOUNTEDSTRING:
        case TDH_INTYPE_REVERSEDCOUNTEDANSISTRING:
        case TDH_INTYPE_NONNULLTERMINATEDSTRING:
        case TDH_INTYPE_NONNULLTERMINATEDANSISTRING:
        case TDH_INTYPE_UNICODECHAR:
        case TDH_INTYPE_ANSICHAR:
        case TDH_INTYPE_SIZET:
        case TDH_INTYPE_HEXDUMP:
        case TDH_INTYPE_WBEMSID:
        case TDH_INTYPE_NULL:
        default:
            printf("[*] Unhandled TYPE: %d\n", prop.type());
            //RaiseException(1,0,0, NULL);
            json_payload[prop_name] = parser.parse<krabs::binary>(prop_name_wstr).bytes();
            break;
        }
    }
    json_event["payload"] = json_payload;

    if (pretty_print)
    {
        return json_event.dump(4);
    }
    else
    {
        return json_event.dump();
    }
}
