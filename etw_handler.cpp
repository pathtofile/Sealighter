#include "etw_handler.h"
#include "json_handler.h"
#include "util.h"
#include <iostream>
#include <fstream>

using json = nlohmann::json;

static user_trace*      pSession = NULL;
EVENT_TRACE_PROPERTIES  session_properties = { 0 };

void handleEvent
(
    const EVENT_RECORD&     record,
    const trace_context&    trace_context
)
{
    schema schema(record, trace_context.schema_locator);
    //std::wcout << L"Event " << schema.event_id();
    //std::wcout << L"(" << schema.event_name() << L") received." << std::endl;
    //if (schema.event_id() == 1 || schema.event_id() == 2 || schema.event_id() == 3 || schema.event_id() == 4) {
    std::string eventString = parse_event_to_json(record, trace_context, schema, true);
    printf("%s\n", eventString.c_str());
    printf("--------------------\n");
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
        sessionName = L"EMon";
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
        }

        // Initialize Session and props
        pSession = new user_trace(sessionName);
        pSession->set_trace_properties(&session_properties);

        // Parse the Providers
        for (json json_provider : json_config["providers"])
        {
            if (json_provider["name"].is_null())
            {
                printf("Invalid Provider\n");
                return 1;
            }
            provider<>* pNew_provider = new provider<>(convert_str_wstr(json_provider["name"].get<std::string>()));

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
                    if (!json_filter["id_is"].is_null())
                        add_predicate_filter(pNew_provider,
                            predicates::id_is(json_filter["id_is"].get<std::uint64_t>()));
                    
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
