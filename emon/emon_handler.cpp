#include "emon_krabs.h"
#include "emon_handler.h"
#include "emon_errors.h"
#include "emon_util.h"
#include "emon_provider.h"

#include <iostream>
#include <fstream>
#include <mutex>


// -------------------------
// GLOBALS - START
// -------------------------

// Output file to write events to
static std::ofstream g_outfile;

// Helper mutex to ensure threaded functions
// print a whole event without interruption
static std::mutex g_threaded_mutex;

// Holds format
static Output_format g_output_format;

// -------------------------
// GLOBALS - END
// -------------------------
// PRIVATE FUNCTIONS - START
// -------------------------


/*
    Print a line to stdout, using a mutex
    to ensure we print each event wholey before
    another can
*/
void threaded_print_ln
(
    std::string event_string
)
{
    g_threaded_mutex.lock();
    printf("%s\n", event_string.c_str());
    g_threaded_mutex.unlock();
}


/*
    Write to Event Log
*/
void write_event_log
(
    krabs::schema   schema,
    std::string event_string
)
{
    DWORD status = ERROR_SUCCESS;

    status = EventWriteEMON_REPORT_EVENT(
        event_string.c_str(),
        (USHORT)schema.event_id(),
        schema.event_name(),
        schema.thread_id(),
        schema.timestamp().QuadPart,
        (USHORT)schema.event_flags(),
        (UCHAR)schema.event_opcode(),
        (UCHAR)schema.event_version(),
        schema.process_id(),
        schema.provider_name()
    );

    //status = EventWriteEMON_REPORT_EVENT(event_string.c_str());

    if (status != ERROR_SUCCESS) {
        printf("Error %ul line %d\n", status, __LINE__);
        return;
    }
}


/*
    Print a line to an output file, using a mutex
    to ensure we print each event wholey before
    another can
*/
void threaded_write_file_ln
(
    std::string event_string
)
{
    g_threaded_mutex.lock();
    g_outfile << event_string << std::endl;
    g_threaded_mutex.unlock();
}


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
            json_payload[prop_name]["type"] = "STRINGA";
            json_payload[prop_name]["data"] = parser.parse<std::string>(prop_name_wstr);
            break;
        case TDH_INTYPE_UNICODESTRING:
            json_payload[prop_name]["type"] = "STRINGW";
            json_payload[prop_name]["data"] = convert_wstr_str(parser.parse<std::wstring>(prop_name_wstr));
            break;
        case TDH_INTYPE_INT8:
            json_payload[prop_name]["type"] = "INT8";
            json_payload[prop_name]["data"] = parser.parse<std::int8_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT8:
            json_payload[prop_name]["type"] = "UINT8";
            json_payload[prop_name]["data"] = parser.parse<std::uint8_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT16:
            json_payload[prop_name]["type"] = "INT16";
            json_payload[prop_name]["data"] = parser.parse<std::int16_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT16:
            json_payload[prop_name]["type"] = "UINT16";
            json_payload[prop_name]["data"] = parser.parse<std::uint16_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT32:
            json_payload[prop_name]["type"] = "INT32";
            json_payload[prop_name]["data"] = parser.parse<std::int32_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT32:
            json_payload[prop_name]["type"] = "UINT32";
            json_payload[prop_name]["data"] = parser.parse<std::uint32_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_INT64:
            json_payload[prop_name]["type"] = "INT64";
            json_payload[prop_name]["data"] = parser.parse<std::int64_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_UINT64:
            json_payload[prop_name]["type"] = "UINT64";
            json_payload[prop_name]["data"] = parser.parse<std::uint64_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_FLOAT:
            json_payload[prop_name]["type"] = "FLOAT";
            json_payload[prop_name]["data"] = parser.parse<std::float_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_DOUBLE:
            json_payload[prop_name]["type"] = "DOUBLE";
            json_payload[prop_name]["data"] = parser.parse<std::double_t>(prop_name_wstr);
            break;
        case TDH_INTYPE_BOOLEAN:
            json_payload[prop_name]["type"] = "BOOLEAN";
            json_payload[prop_name]["data"] =
                convert_bytes_bool(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_BINARY:
            json_payload[prop_name]["type"] = "BINARY";
            json_payload[prop_name]["data"] =
                convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_GUID:
            json_payload[prop_name]["type"] = "GUID";
            json_payload[prop_name]["data"] =
                convert_guid_str(parser.parse<krabs::guid>(prop_name_wstr));
            break;
        case TDH_INTYPE_FILETIME:
            json_payload[prop_name]["type"] = "FILETIME";
            json_payload[prop_name]["data"] = convert_bytes_filetimestring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_SID:
            json_payload[prop_name]["type"] = "SID";
            json_payload[prop_name]["data"] = convert_bytes_sidstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_WBEMSID:
            // *Supposedly* like SID?
            json_payload[prop_name]["type"] = "WBEMSID";
            json_payload[prop_name]["data"] = convert_bytes_hexstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_POINTER:
            json_payload[prop_name]["type"] = "POINTER";
            json_payload[prop_name]["data"] =
                convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_SYSTEMTIME:
            json_payload[prop_name]["type"] = "SYSTEMTIME";
            json_payload[prop_name]["data"] = convert_bytes_systemtimestring(
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
            json_payload[prop_name]["type"] = "OTHER";
            json_payload[prop_name]["data"] =
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

    // Log event if we successfully parsed it
    if (!event_string.empty()) {
        switch (g_output_format)
        {
        case output_stdout:
            threaded_print_ln(event_string);
            break;
        case output_event_log:
            write_event_log(schema, event_string);
            break;
        case output_file:
            threaded_write_file_ln(event_string);
            break;
        }
    }
}

int setup_logger_file
(
    std::string filename
)
{
    g_outfile.open(filename.c_str(), std::ios::out | std::ios::app);
    if (g_outfile.good()) {
        return ERROR_SUCCESS;
    }
    else {
        return EMON_ERROR_OUTPUT_FILE;
    }
}

void teardown_logger_file()
{
    if (g_outfile.is_open()) {
        g_outfile.close();
    }
}

void set_output_format(Output_format format)
{
    g_output_format = format;
}
