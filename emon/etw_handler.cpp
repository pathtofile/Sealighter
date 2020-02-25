#include "etw_handler.h"
#include "util.h"
#include "output_logger.h"
#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

static user_trace*              pSession = NULL;
static EVENT_TRACE_PROPERTIES   session_properties = { 0 };
static output_formats           output_format = output_stdout;

// Not sure how else to do this besides a #define....
#define ADD_PROP_FILTER(pNew_provider, json_filter, prop_str, prop_func)            \
                    if (!json_filter[prop_str].is_null() &&                         \
                        !json_filter[prop_str]["name"].is_null() &&                 \
                        !json_filter[prop_str]["value"].is_null())                  \
                        add_predicate_filter(pNew_provider,                         \
                            predicates::prop_func(                                  \
                                json_filter[prop_str]["name"].get<std::wstring>(),  \
                                json_filter[prop_str]["value"].get<std::wstring>()))

std::string parse_event_to_json
(
    const   EVENT_RECORD & record,
    const   krabs::trace_context & trace_context,
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
    json json_event = { {"header", json_header} };
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

void handleEvent
(
    const EVENT_RECORD&     record,
    const trace_context&    trace_context
)
{
    schema schema(record, trace_context.schema_locator);
    std::string eventString = parse_event_to_json(record, trace_context, schema, true);
    log_event(output_format, eventString);
}

void add_predicate_filter
(
    provider<>* pNew_provider,
    filter_predicate predicate
)
{
    event_filter filter(predicate);
    filter.add_on_event_callback((c_provider_callback)&handleEvent);
    pNew_provider->add_filter(filter);
}

int setup_trace
(
    std::string       config_path
)
{
    try
    {
        std::wstring    sessionName;
        std::ifstream   config_stream(config_path);
        json            json_config;
        
        // Read in config file
        config_stream >> json_config;
        config_stream.close();

        // Set defaults
        sessionName = L"EMon-Trace";
        session_properties.BufferSize = 256;
        session_properties.MinimumBuffers = 12;
        session_properties.MaximumBuffers = 48;
        session_properties.FlushTimer = 1;
        session_properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;

        json json_props = json_config["session_properties"];
        if (!json_props.is_null())
        {
            if (!json_props["session_name"].is_null())
                sessionName = convert_str_wstr(json_props["session_name"].get<std::string>());

            if (!json_props["buffer_size"].is_null())
                session_properties.BufferSize = json_props["buffer_size"].get<std::uint32_t>();
            
            if (!json_props["minimum_buffers"].is_null())
                session_properties.MinimumBuffers = json_props["minimum_buffers"].get<std::uint32_t>();
            
            if (!json_props["maximum_buffers"].is_null())
                session_properties.MaximumBuffers = json_props["maximum_buffers"].get<std::uint32_t>();
            
            if (!json_props["flush_timer"].is_null())
                session_properties.FlushTimer = json_props["flush_timer"].get<std::uint32_t>();

            if (!json_props["output_format"].is_null())
            {
                std::string format = json_props["output_format"].get<std::string>();
                if (format == "all")
                {
                    output_format = output_all;
                }
                else if (format == "stdout")
                {
                    output_format = output_stdout;
                }
                else if (format == "event_log")
                {
                    output_format = output_event_log;
                }
                else
                {
                    printf("Invalid output_format\n");
                    return 1;
                }
            }
        }

        // Initialize Session and props
        pSession = new user_trace(sessionName);
        pSession->set_trace_properties(&session_properties);

        // Parse the Providers
        for (json json_provider : json_config["providers"])
        {
            provider<>* pNew_provider;
            if (json_provider["name"].is_null())
            {
                printf("Invalid Provider\n");
                return 1;
            }
            pNew_provider = new provider<>(convert_str_wstr(json_provider["name"].get<std::string>()));

            if (!json_provider["keywords_all"].is_null())
                pNew_provider->all(json_provider["keywords_all"].get<std::uint64_t>());

            if (!json_provider["keywords_any"].is_null())
                pNew_provider->any(json_provider["keywords_any"].get<std::uint64_t>());

            if (!json_provider["level"].is_null())
                pNew_provider->level(json_provider["level"].get<std::uint64_t>());

            if (!json_provider["trace_flags"].is_null())
                pNew_provider->trace_flags(json_provider["trace_flags"].get<std::uint64_t>());

            // Go through any filters
            if (json_provider["filters"].is_null())
            {
                // No filters, log everything
                pNew_provider->add_on_event_callback((c_provider_callback)&handleEvent);
            }
            else
            {
                for (json json_filter: json_provider["filters"])
                {
                    // Add any of the header filters
                    if (!json_filter["id_is"].is_null())
                        add_predicate_filter(pNew_provider,
                            predicates::id_is(json_filter["id_is"].get<std::uint64_t>()));

                    if (!json_filter["opcode_is"].is_null())
                        add_predicate_filter(pNew_provider,
                            predicates::opcode_is(json_filter["opcode_is"].get<std::uint64_t>()));

                    if (!json_filter["process_id_is"].is_null())
                        add_predicate_filter(pNew_provider,
                            predicates::process_id_is(json_filter["process_id_is"].get<std::uint64_t>()));

                    if (!json_filter["version_is"].is_null())
                        add_predicate_filter(pNew_provider,
                            predicates::version_is(json_filter["version_is"].get<std::uint64_t>()));

                    // Add any of the property filters
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_is", property_is);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_contains", property_contains);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_ends_with", property_ends_with);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_equals", property_equals);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_starts_with", property_starts_with);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_icontains", property_icontains);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_iends_with", property_iends_with);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_iequals", property_iequals);
                    ADD_PROP_FILTER(pNew_provider, json_filter, "property_istarts_with", property_istarts_with);

                    // TODO: Work out how to add OR, AND, or NOTs...
                    //event_filter filter(predicates::id_is(1));
                    //auto filter1 = krabs::predicates::not_filter(krabs::predicates::id_is(1));
                    //filter.add_on_event_callback((c_provider_callback)&handleEvent);
                    //provider.add_filter(filter);
                    // filter.add_on_event_callback(...
                    //provider.add_filter(filter);
                }
            }

            pSession->enable(*pNew_provider);
        }
    }
    catch (const nlohmann::detail::exception&)
    {
        printf("invalid config file\n");
        return 1;
    }
    return 0;
}

int trace_start
(
    std::string config_path
)
{
    int ret = 0;
    ret = setup_trace(config_path);
    if (ret != 0)
    {
        return ret;
    }

    // Ensure we always stop the trace afterwards
    try
    {
        printf("Starting Trace...\n");
        pSession->start();
    }
    catch (...)
    {
        pSession->stop();
        throw;
    }
    return ret;
}

void trace_stop()
{
    if (NULL != pSession)
    {
        pSession->stop();
    }
}
