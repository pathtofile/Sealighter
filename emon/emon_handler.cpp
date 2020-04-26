#include "emon_krabs.h"
#include "emon_handler.h"
#include "emon_logger.h"
#include "emon_util.h"


/*
    Convert an ETW Event to JSON
*/
std::string parse_event_to_json
(
    krabs::schema   schema,
    const   bool    pretty_print
)
{
    krabs::parser parser(schema);
    json json_payload;
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
    json json_event = { {"header", json_header} };

    for (krabs::property& prop : parser.properties())
    {
        std::wstring prop_name_wstr = prop.name();
        std::string prop_name = convert_wstr_str(prop_name_wstr);

        switch (prop.type())
        {
        case TDH_INTYPE_ANSISTRING:
            json_payload[prop_name + " <STRINGA>"] = parser.parse<std::string>(prop_name_wstr);
            break;
        case TDH_INTYPE_UNICODESTRING:
            json_payload[prop_name + " <STRINGW>"] =
                convert_wstr_str(parser.parse<std::wstring>(prop_name_wstr));
            break;
        case TDH_INTYPE_INT8:
            json_payload[prop_name + " <INT8>"] = parser.parse<std::int8_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT8:
            json_payload[prop_name + " <UINT8>"] = parser.parse<std::uint8_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT16:
            json_payload[prop_name + " <INT16>"] = parser.parse<std::int16_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT16:
            json_payload[prop_name + " <UINT16>"] = parser.parse<std::uint16_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT32:
            json_payload[prop_name + " <INT32>"] = parser.parse<std::int32_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT32:
            json_payload[prop_name + " <UINT32>"] = parser.parse<std::uint32_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT64:
            json_payload[prop_name + " <INT64>"] = parser.parse<std::int64_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT64:
            json_payload[prop_name + " <UINT64>"] = parser.parse<std::uint64_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_FLOAT:
            json_payload[prop_name + " <FLOAT>"] = parser.parse<std::float_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_DOUBLE:
            json_payload[prop_name + " <DOUBLE>"] = parser.parse<std::double_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_BOOLEAN:
            json_payload[prop_name + " <BOOLEAN>"] =
                convert_bytes_bool(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_BINARY:
            json_payload[prop_name + " <BINARY>"] =
                convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_GUID:
            json_payload[prop_name + " <GUID>"] =
                convert_guid_str(parser.parse<krabs::guid>(prop_name_wstr));
            break;
        case TDH_INTYPE_FILETIME:
            json_payload[prop_name + " <FILETIME>"] = convert_bytes_filetimestring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_SID:
            json_payload[prop_name + " <SID>"] = convert_bytes_sidstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_WBEMSID:
            // *Supposedly* like SID?
            json_payload[prop_name + " <WBEMSID>"] = convert_bytes_hexstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_POINTER:
            json_payload[prop_name + " <POINTER>"] =
                convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_SYSTEMTIME:
            json_payload[prop_name + " <SYSTEMTIME>"] = convert_bytes_systemtimestring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
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
        case TDH_INTYPE_NULL:
        default:
            printf("[*] Unhandled TYPE: %d\n", prop.type());
            //RaiseException(1,0,0, NULL);
            json_payload[prop_name + " <OTHER>"] =
                convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        }
    }
    json_event["payload"] = json_payload;

    return convert_json_string(json_event, pretty_print);
}


void handle_event
(
    const EVENT_RECORD& record,
    const trace_context& trace_context
)
{
    schema schema(record, trace_context.schema_locator);
    std::string event_string;
    // If writing to a file, don't pretty print
    // This makes it 1 line per event
    if (Output_format::output_file == g_output_format) {
        event_string = parse_event_to_json(schema, false);
    }
    else {
        event_string = parse_event_to_json(schema, true);
    }
    if (!event_string.empty())
        log_event(g_output_format, event_string);
}
