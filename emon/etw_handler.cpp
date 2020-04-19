#include "etw_handler.h"
#include "util.h"
#include "etw_parser.h"
#include "emon_predicates.h"
#include <iostream>
#include <fstream>
#include <thread>
#include <tuple>
#include <vector>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

static user_trace*             g_user_session = NULL;
static kernel_trace*           g_kernel_session = NULL;
static EVENT_TRACE_PROPERTIES  g_session_properties = { 0 };


// Not sure how else to do cleanly this besides a #define....
#define ADD_PROP_FILTER(pNew_provider, json_filter, prop_str, prop_func)  \
    if (!json_filter[prop_str].is_null() &&                         \
        !json_filter[prop_str]["name"].is_null() &&                 \
        !json_filter[prop_str]["value"].is_null()) {                \
            for (json json_filter_i : json_filter[prop_str]) {      \
                add_predicate_filter(pNew_provider,                 \
                    predicates::prop_func(                          \
                        convert_str_wstr(json_filter_i["name"].get<std::string>()),  \
                        convert_str_wstr(json_filter_i["value"].get<std::string>())));\
            }                                                       \
        }                                                           \

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

template <typename TPred, typename TJson1, typename TJson2>
void add_value_to_vector_2
(
    json root,
    std::string item1_name,
    std::string item2_name,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root.is_null()) {
        if (root.is_array()) {
            for (json item : root) {
                if (!item[item1_name].is_null() && !item[item2_name].is_null()) {
                    TJson1 item1 = item[item1_name].get<TJson1>();
                    TJson2 item2 = item[item2_name].get<TJson2>();
                    list.emplace_back(std::shared_ptr<TPred>(new TPred(item1, item2)));
                }
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<emon_any_of>(new emon_any_of(list)));
            }
        }
        else {
            if (!root[item1_name].is_null() && !root[item2_name].is_null()) {
                TJson1 item1 = root[item1_name].get<TJson1>();
                TJson2 item2 = root[item2_name].get<TJson2>();
                pred_vector.emplace_back(std::shared_ptr<TPred>(new TPred(item1, item2)));
            }
        }
    }
}

template <typename TPred, typename TJson1>
void add_value_to_vector_1
(
    json root,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root.is_null()) {
        if (root.is_array()) {
            for (json item : root) {
                list.emplace_back(std::shared_ptr<TPred>(new TPred(item.get<TJson1>())));
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<emon_any_of>(new emon_any_of(list)));
            }
        }
        else {
            pred_vector.emplace_back(std::shared_ptr<TPred>(new TPred(root.get<TJson1>())));
        }
    }
}

void add_property_is_to_vector_item
(
    json item,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& list
)
{
    if (!item["name"].is_null() && !item["value"].is_null() && !item["type"].is_null()) {
        std::wstring name = convert_str_wstr(item["name"].get<std::string>());
        std::string type = item["type"].get<std::string>();
        if (type == "STRINGA") {
            auto val = item["value"].get<std::string>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::string>>(new emon_property_is<std::string>(name, val)));
        }
        else if (type == "STRINGW") {
            auto val = convert_str_wstr(item["value"].get<std::string>());
            list.emplace_back(std::shared_ptr<emon_property_is<std::wstring>>(new emon_property_is<std::wstring>(name, val)));
        }
        else if (type == "INT8") {
            auto val = item["value"].get<std::int8_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::int8_t>>(new emon_property_is<std::int8_t>(name, val)));
        }
        else if (type == "UINT8") {
            auto val = item["value"].get<std::uint8_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::uint8_t>>(new emon_property_is<std::uint8_t>(name, val)));
        }
        else if (type == "INT16") {
            auto val = item["value"].get<std::int16_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::int16_t>>(new emon_property_is<std::int16_t>(name, val)));
        }
        else if (type == "UINT16") {
            auto val = item["value"].get<std::uint16_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::uint16_t>>(new emon_property_is<std::uint16_t>(name, val)));
        }
        else if (type == "INT32") {
            auto val = item["value"].get<std::int32_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::int32_t>>(new emon_property_is<std::int32_t>(name, val)));
        }
        else if (type == "UINT32") {
            auto val = item["value"].get<std::uint32_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::uint32_t>>(new emon_property_is<std::uint32_t>(name, val)));
        }
        else if (type == "INT64") {
            auto val = item["value"].get<std::int64_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::int64_t>>(new emon_property_is<std::int64_t>(name, val)));
        }
        else if (type == "UINT64") {
            auto val = item["value"].get<std::uint64_t>();
            list.emplace_back(std::shared_ptr<emon_property_is<std::uint64_t>>(new emon_property_is<std::uint64_t>(name, val)));
        }
    }
}

void add_property_is_to_vector
(
    json root,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root.is_null()) {
        if (root.is_array()) {
            for (json item : root) {
                add_property_is_to_vector_item(item, list);
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<emon_any_of>(new emon_any_of(list)));
            }
        }
        else {
            add_property_is_to_vector_item(root, pred_vector);
        }
    }
}

template <typename ComparerA, typename ComparerW>
void add_property_to_vector_item
(
    json item,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& list
)
{
    if (!item["name"].is_null() && !item["value"].is_null() && !item["type"].is_null()) {
        std::wstring name = convert_str_wstr(item["name"].get<std::string>());
        std::string type = item["type"].get<std::string>();
        if (type == "STRINGA") {
            std::string val = item["value"].get<std::string>();
            auto pred = std::shared_ptr<
                krabs::predicates::details::property_view_predicate<
                    std::string,
                    krabs::predicates::adapters::generic_string<char>,
                    ComparerA
                >
            >(new krabs::predicates::details::property_view_predicate<
                    std::string,
                    krabs::predicates::adapters::generic_string<char>,
                    ComparerA
            >(
                name,
                val,
                krabs::predicates::adapters::generic_string<char>(),
                ComparerA()
            ));
            list.emplace_back(pred);
        }
        else if (type == "STRINGW") {
            std::wstring val = convert_str_wstr(item["value"].get<std::string>());

            auto pred = std::shared_ptr<
                krabs::predicates::details::property_view_predicate<
                std::wstring,
                krabs::predicates::adapters::generic_string<wchar_t>,
                ComparerW
                >
            >(new krabs::predicates::details::property_view_predicate<
                std::wstring,
                krabs::predicates::adapters::generic_string<wchar_t>,
                ComparerW
            >(
                name,
                val,
                krabs::predicates::adapters::generic_string<wchar_t>(),
                ComparerW()
            ));
            list.emplace_back(pred);
        }
    }
}

template <typename ComparerA, typename ComparerW>
void add_property_to_vector
(
    json root,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root.is_null()) {
        if (root.is_array()) {
            for (json item : root) {
                add_property_to_vector_item<ComparerA, ComparerW>(item, list);
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<emon_any_of>(new emon_any_of(list)));
            }
        }
        else {
            add_property_to_vector_item<ComparerA, ComparerW>(root, pred_vector);
        }
    }
}

void add_filters_to_vector(std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector, json json_list)
{
    // Add any of the header filters
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    
    // Add the basic single-value filters
    add_value_to_vector_1<predicates::id_is, std::uint64_t>(json_list["id_is"], pred_vector);
    add_value_to_vector_1<predicates::opcode_is, std::uint64_t>(json_list["opcode_is"], pred_vector);
    add_value_to_vector_1<predicates::process_id_is, std::uint64_t>(json_list["process_id_is"], pred_vector);
    add_value_to_vector_1<predicates::version_is, std::uint64_t>(json_list["version_is"], pred_vector);

    // Add all the property filters
    add_property_is_to_vector(json_list["property_is"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::equals<std::equal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::equals<std::equal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_equals"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::equals<iequal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::equals<iequal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_iequals"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::contains<std::equal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::contains<std::equal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_contains"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::contains<iequal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::contains<iequal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_icontains"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::starts_with<std::equal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::starts_with<std::equal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_starts_with"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::starts_with<iequal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::starts_with<iequal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_istarts_with"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::ends_with<std::equal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::ends_with<std::equal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_starts_with"], pred_vector);

    add_property_to_vector<
        krabs::predicates::comparers::ends_with<iequal_to<krabs::predicates::adapters::generic_string<char>::value_type>>,
        krabs::predicates::comparers::ends_with<iequal_to<krabs::predicates::adapters::generic_string<wchar_t>::value_type>>
    >(json_list["property_istarts_with"], pred_vector);

    // Add own own created Predicates
    add_value_to_vector_1<emon_max_events_total, std::uint64_t>(json_list["max_events_total"], pred_vector);
    add_value_to_vector_2<emon_max_events_id, std::uint64_t, std::uint64_t>(json_list["max_events_id"], "id_is", "max_events", pred_vector);
    add_value_to_vector_1<emon_any_field_contains, std::string>(json_list["any_field_contains"], pred_vector);
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
        // Build top-level list
        // All 3 options will eventually be ANDed together
        std::vector<std::shared_ptr<predicates::details::predicate_base>> top_list;
        if (!json_provider["filters"]["any_of"].is_null()) {
            std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
            add_filters_to_vector(list, json_provider["filters"]["any_of"]);
            top_list.emplace_back(std::shared_ptr<emon_any_of>(new emon_any_of(list)));
        }
        if (!json_provider["filters"]["all_of"].is_null()) {
            std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
            add_filters_to_vector(list, json_provider["filters"]["all_of"]);
            top_list.emplace_back(std::shared_ptr<emon_all_of>(new emon_all_of(list)));
        }
        if (!json_provider["filters"]["none_of"].is_null()) {
            std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
            add_filters_to_vector(list, json_provider["filters"]["none_of"]);
            top_list.emplace_back(std::shared_ptr<emon_none_of>(new emon_none_of(list)));
        }

        // Add top level list to a filter
        emon_all_of top_pred = emon_all_of(top_list);
        krabs::event_filter filter(top_pred);
        filter.add_on_event_callback((c_provider_callback)&handleEvent);
        pNew_provider->add_filter(filter);
    }
}


DWORD add_user_providers(json json_config, std::wstring session_name)
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

DWORD add_kernel_providers(json json_config)
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


DWORD setup_trace
(
    std::string       config_path
)
{
    DWORD status = 0;
    try {
        std::ifstream   config_stream(config_path);
        json            json_config;
        std::wstring    session_name = L"EMon-Trace";

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

            if (!json_props["output_format"].is_null()) {
                std::string format = json_props["output_format"].get<std::string>();
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
        }

        // Add user providers
        if (!json_config["user_providers"].is_null()) {
            status = add_user_providers(json_config, session_name);
        }

        if (0 == status && !json_config["kernel_providers"].is_null()) {
            status = add_kernel_providers(json_config);
        }
    }
    catch (const nlohmann::detail::exception&) {
        printf("invalid config file\n");
        return __LINE__;
    }
    return status;
}


template <typename T>
void run_trace(krabs::trace<T>* trace)
{
    if (NULL != trace) {
        // Ensure we always stop the trace afterwards
        try {
            trace->start();
        }
        catch (...) {
            trace->stop();
            throw;
        }
    }
}

DWORD trace_start
(
    std::string config_path
)
{
    DWORD status = 0;
    status = setup_trace(config_path);
    if (0 != status) {
        return status;
    }

    if (NULL == g_user_session && NULL == g_kernel_session) {
        printf("No Providers defined\n");
        return __LINE__;
    }
    // Don't run multithreaded if we don't have to
    else if (NULL != g_user_session && NULL == g_kernel_session) {
        printf("Starting User Trace...\n");
        run_trace(g_user_session);
    }
    else if (NULL == g_user_session && NULL != g_kernel_session) {
        printf("Starting Kernel Trace...\n");
        run_trace(g_kernel_session);
    }
    else {
        // Have to multi-thread it
        printf("Starting User and Kernel Traces...\n");
        std::thread user_thread = std::thread(run_trace<krabs::details::ut>, g_user_session);
        std::thread kernel_thread = std::thread(run_trace<krabs::details::kt>, g_kernel_session);
        user_thread.join();
        kernel_thread.join();
    }
    return status;
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
