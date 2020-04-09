#include "etw_handler.h"
#include "util.h"
#include "output_logger.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <tuple>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

static user_trace*             g_user_session = NULL;
static kernel_trace*           g_kernel_session = NULL;
static EVENT_TRACE_PROPERTIES  g_session_properties = { 0 };
static Output_format           g_output_format = Output_format::output_stdout;

static int g_max_events = 0;
static std::map<const wchar_t*, std::map<USHORT,int>, cmp_wstr> g_max_events_map;

// Not sure how else to do cleanly this besides a #define....
#define ADD_PROP_FILTER(pNew_provider, json_filter, prop_str, prop_func)            \
                    if (!json_filter[prop_str].is_null() &&                         \
                        !json_filter[prop_str]["name"].is_null() &&                 \
                        !json_filter[prop_str]["value"].is_null())                  \
                        add_predicate_filter(pNew_provider,                         \
                            predicates::prop_func(                                  \
                                convert_str_wstr(json_filter[prop_str]["name"].get<std::string>()),  \
                                convert_str_wstr(json_filter[prop_str]["value"].get<std::string>())))


template <typename T>
void add_predicate_filter
(
    details::base_provider<T>* pNew_provider,
    filter_predicate predicate
)
{
    event_filter filter(predicate);
    filter.add_on_event_callback((c_provider_callback)&handleEvent);
    pNew_provider->add_filter(filter);
}

template <typename T>
void add_filters
(
    details::base_provider<T>* pNew_provider,
    json json_provider
)
{
    if (json_provider["filters"].is_null()) {
        // No filters, log everything
        pNew_provider->add_on_event_callback((c_provider_callback)&handleEvent);
    }
    else {
        for (json json_filter : json_provider["filters"]) {
            // Add any of the header filters
            if (!json_filter["id_is"].is_null()) {
                add_predicate_filter(pNew_provider,
                    predicates::id_is(json_filter["id_is"].get<std::uint64_t>()));
            }

            if (!json_filter["id_is_not"].is_null()) {
                add_predicate_filter(pNew_provider, predicates::details::not_filter<predicates::id_is>(predicates::id_is(json_filter["id_is"].get<std::uint64_t>())));
            }

            if (!json_filter["opcode_is"].is_null()) {
                add_predicate_filter(pNew_provider,
                    predicates::opcode_is(json_filter["opcode_is"].get<std::uint64_t>()));
            }

            if (!json_filter["process_id_is"].is_null()) {
                add_predicate_filter(pNew_provider,
                    predicates::process_id_is(json_filter["process_id_is"].get<std::uint64_t>()));
            }

            if (!json_filter["version_is"].is_null()) {
                add_predicate_filter(pNew_provider,
                    predicates::version_is(json_filter["version_is"].get<std::uint64_t>()));
            }

            // Add any of the property filters
            // TODO: Macros is how I usually stop code repitition, but is there a better c++ way?
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
        }
    }
}

std::string parse_event_to_json
(
    const   EVENT_RECORD& record,
    const   krabs::trace_context& trace_context,
    krabs::schema           schema,
    const   bool                    pretty_print
)
{
    // If limiting the number of unique events, check that first
    if (0 != g_max_events) {
        const wchar_t* provider_name = schema.provider_name();
        int check_id = 0;
        // kernel traces use opcode, user traces use the event_id
        if (std::wcscmp(provider_name, L"MSNT_SystemTrace") == 0) {
            check_id = schema.event_opcode();
        }
        else {
            check_id = schema.event_id();
        }

        auto provider_i = g_max_events_map.find(provider_name);
        if (provider_i == g_max_events_map.end()) {
            // Add to dictionary
            std::map<USHORT, int> provider_map;
            provider_map[check_id] = 1;
            g_max_events_map[provider_name] = provider_map;
        }
        else {
            auto event_i = provider_i->second.find(check_id);
            if (event_i == provider_i->second.end()) {
                // Add to dictionary
                provider_i->second[check_id] = 1;
            }
            else if (event_i->second < g_max_events) {
                // Increment but report
                event_i->second++;
            }
            else {
                // Over reporting limit, ignore
                return "";
            }
        }
    }

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
            if (convert_bytes_int(parser.parse<krabs::binary>(prop_name_wstr).bytes())) {
                json_payload[prop_name] = true;
            }
            else {
                json_payload[prop_name] = false;
            }
            break;
        case TDH_INTYPE_BINARY:
            json_payload[prop_name] = convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_GUID:
            json_payload[prop_name] = convert_guid_str(parser.parse<krabs::guid>(prop_name_wstr));
            break;
        case TDH_INTYPE_FILETIME:
            json_payload[prop_name] = convert_bytes_filetimestring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_SID:
            json_payload[prop_name] = convert_bytes_sidstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_WBEMSID:
            // *Supposedly* like SID?
            json_payload[prop_name] = convert_bytes_hexstring(
                parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        case TDH_INTYPE_POINTER:
            json_payload[prop_name] = convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
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
        case TDH_INTYPE_NULL:
        default:
            printf("[*] Unhandled TYPE: %d\n", prop.type());
            //RaiseException(1,0,0, NULL);
            json_payload[prop_name] = convert_bytes_hexstring(parser.parse<krabs::binary>(prop_name_wstr).bytes());
            break;
        }
    }
    json_event["payload"] = json_payload;

    if (pretty_print) {
        return json_event.dump(4);
    }
    else {
        return json_event.dump();
    }
}

void handleEvent
(
    const EVENT_RECORD& record,
    const trace_context& trace_context
)
{
    schema schema(record, trace_context.schema_locator);
    std::string eventString = parse_event_to_json(record, trace_context, schema, true);
    if (!eventString.empty())
        log_event(g_output_format, eventString);
}

int add_user_providers(json json_config, std::wstring session_name)
{
    // Initialize Session and props
    g_user_session = new user_trace(session_name);
    g_user_session->set_trace_properties(&g_session_properties);
    try {
        // Parse the Usermode Providers
        for (json json_provider : json_config["user_providers"]) {
            provider<>* pNew_provider;
            if (json_provider["name"].is_null()) {
                printf("Invalid Provider\n");
                return __LINE__;
            }
            pNew_provider = new provider<>(convert_str_wstr(json_provider["name"].get<std::string>()));

            if (!json_provider["keywords_all"].is_null()) {
                pNew_provider->all(json_provider["keywords_all"].get<std::uint64_t>());
            }

            if (!json_provider["keywords_any"].is_null()) {
                pNew_provider->any(json_provider["keywords_any"].get<std::uint64_t>());
            }

            if (!json_provider["level"].is_null()) {
                pNew_provider->level(json_provider["level"].get<std::uint64_t>());
            }

            if (!json_provider["trace_flags"].is_null()) {
                pNew_provider->trace_flags(json_provider["trace_flags"].get<std::uint64_t>());
            }

            // Add any filters
            add_filters(pNew_provider, json_provider);

            g_user_session->enable(*pNew_provider);
        }
    }
    catch (const nlohmann::detail::exception&) {
        printf("invalid providers in config file\n");
        return __LINE__;
    }
    return 0;
}

int add_kernel_providers(json json_config)
{
    // Initialize Session and props
    g_kernel_session = new kernel_trace();
    g_kernel_session->set_trace_properties(&g_session_properties);
    try {
        for (json json_provider : json_config["kernel_providers"]) {
            details::base_provider<kernel_provider> xxx;

            kernel_provider* pNew_provider;
            if (json_provider["name"].is_null()) {
                printf("Invalid Provider\n");
                return __LINE__;
            }
            std::string provider_name = json_provider["name"].get<std::string>();
            if (provider_name == "process") {
                pNew_provider = new krabs::kernel::process_provider();
            }
            else if (provider_name == "process") {
                pNew_provider = new krabs::kernel::process_provider();
            }
            else if (provider_name == "thread") {
                pNew_provider = new krabs::kernel::thread_provider();
            }
            else if (provider_name == "image_load") {
                pNew_provider = new krabs::kernel::image_load_provider();
            }
            else if (provider_name == "process_counter") {
                pNew_provider = new krabs::kernel::process_counter_provider();
            }
            else if (provider_name == "context_switch") {
                pNew_provider = new krabs::kernel::context_switch_provider();
            }
            else if (provider_name == "dpc") {
                pNew_provider = new krabs::kernel::dpc_provider();
            }
            else if (provider_name == "interrupt") {
                pNew_provider = new krabs::kernel::interrupt_provider();
            }
            else if (provider_name == "system_call") {
                pNew_provider = new krabs::kernel::system_call_provider();
            }
            else if (provider_name == "disk_io") {
                pNew_provider = new krabs::kernel::disk_io_provider();
            }
            else if (provider_name == "disk_file_io") {
                pNew_provider = new krabs::kernel::disk_file_io_provider();
            }
            else if (provider_name == "disk_init_io") {
                pNew_provider = new krabs::kernel::disk_init_io_provider();
            }
            else if (provider_name == "thread_dispatch") {
                pNew_provider = new krabs::kernel::thread_dispatch_provider();
            }
            else if (provider_name == "memory_page_fault") {
                pNew_provider = new krabs::kernel::memory_page_fault_provider();
            }
            else if (provider_name == "memory_hard_fault") {
                pNew_provider = new krabs::kernel::memory_hard_fault_provider();
            }
            else if (provider_name == "virtual_alloc") {
                pNew_provider = new krabs::kernel::virtual_alloc_provider();
            }
            else if (provider_name == "network_tcpip") {
                pNew_provider = new krabs::kernel::network_tcpip_provider();
            }
            else if (provider_name == "registry") {
                pNew_provider = new krabs::kernel::registry_provider();
            }
            else if (provider_name == "alpc") {
                pNew_provider = new krabs::kernel::alpc_provider();
            }
            else if (provider_name == "split_io") {
                pNew_provider = new krabs::kernel::split_io_provider();
            }
            else if (provider_name == "driver") {
                pNew_provider = new krabs::kernel::driver_provider();
            }
            else if (provider_name == "profile") {
                pNew_provider = new krabs::kernel::profile_provider();
            }
            else if (provider_name == "file_io") {
                pNew_provider = new krabs::kernel::file_io_provider();
            }
            else if (provider_name == "file_init_io") {
                pNew_provider = new krabs::kernel::file_init_io_provider();
            }
            else {
                printf("Invalid Provider: %s\n", provider_name.c_str());
                return __LINE__;
            }

            // Add any filters
            add_filters(pNew_provider, json_provider);
            g_kernel_session->enable(*pNew_provider);
        }
    }
    catch (const nlohmann::detail::exception&) {
        printf("invalid providers in config file\n");
        return __LINE__;
    }
    return 0;
}


int setup_trace
(
    std::string       config_path
)
{
    int ret = 0;
    try {
        std::ifstream   config_stream(config_path);
        json            json_config;
        std::wstring    session_name;

        // Read in config file
        config_stream >> json_config;
        config_stream.close();

        // Set defaults
        g_session_properties.BufferSize = 256;
        g_session_properties.MinimumBuffers = 12;
        g_session_properties.MaximumBuffers = 48;
        g_session_properties.FlushTimer = 1;
        g_session_properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;

        json json_props = json_config["session_properties"];
        if (!json_props.is_null())
        {
            if (!json_props["session_name"].is_null()) {
                session_name = convert_str_wstr(json_props["session_name"].get<std::string>());
            }

            if (!json_props["buffer_size"].is_null()) {
                g_session_properties.BufferSize = json_props["buffer_size"].get<std::uint32_t>();
            }

            if (!json_props["minimum_buffers"].is_null()) {
                g_session_properties.MinimumBuffers = json_props["minimum_buffers"].get<std::uint32_t>();
            }

            if (!json_props["maximum_buffers"].is_null()) {
                g_session_properties.MaximumBuffers = json_props["maximum_buffers"].get<std::uint32_t>();
            }

            if (!json_props["flush_timer"].is_null()) {
                g_session_properties.FlushTimer = json_props["flush_timer"].get<std::uint32_t>();
            }

            if (!json_props["g_output_format"].is_null()) {
                std::string format = json_props["g_output_format"].get<std::string>();
                if (format == "all") {
                    g_output_format = Output_format::output_all;
                }
                else if (format == "stdout") {
                    g_output_format = Output_format::output_stdout;
                }
                else if (format == "event_log") {
                    g_output_format = Output_format::output_event_log;
                }
                else {
                    printf("Invalid g_output_format\n");
                    return __LINE__;
                }
            }

            // Check for special "event_count_max" property
            if (!json_props["event_count_max"].is_null()) {
                g_max_events = json_props["event_count_max"].get<std::uint32_t>();
            }
        }

        // Add user providers
        if (!json_config["user_providers"].is_null()) {
            ret = add_user_providers(json_config, session_name);
        }

        if (0 == ret && !json_config["kernel_providers"].is_null()) {
            ret = add_kernel_providers(json_config);
        }
    }
    catch (const nlohmann::detail::exception&) {
        printf("invalid config file\n");
        return __LINE__;
    }
    return ret;
}

void run_user_trace()
{
    if (NULL != g_user_session) {
        // Ensure we always stop the trace afterwards
        try {
            threaded_println("Starting User Trace...");
            g_user_session->start();
        }
        catch (...) {
            g_user_session->stop();
            throw;
        }
    }
}

void run_kernel_trace()
{
    if (NULL != g_kernel_session) {
        // Ensure we always stop the trace afterwards
        try {
            threaded_println("Starting Kernel Trace...");
            g_kernel_session->start();
        }
        catch (...) {
            g_kernel_session->stop();
            throw;
        }
    }
}

int trace_start
(
    std::string config_path
)
{
    int ret = 0;
    ret = setup_trace(config_path);
    if (0 != ret) {
        return ret;
    }

    if (NULL == g_user_session && NULL == g_kernel_session) {
        printf("No Providers defined\n");
        return __LINE__;
    }
    // Run single-theaded if possible
    else if (NULL != g_user_session && NULL == g_kernel_session) {
        run_user_trace();
    }
    else if (NULL == g_user_session && NULL != g_kernel_session) {
        run_kernel_trace();
    }
    else {
        // Have to multi-thread it
        std::thread user_thread = std::thread(run_user_trace);
        std::thread kernel_thread = std::thread(run_kernel_trace);
        user_thread.join();
        kernel_thread.join();
    }
    return ret;
}

void trace_stop()
{
    if (NULL != g_user_session) {
        g_user_session->stop();
    }
    if (NULL != g_kernel_session) {
        g_kernel_session->stop();
    }
}
