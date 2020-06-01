#include "sealighter_krabs.h"
#include <iostream>
#include <fstream>
#include <thread>
#include "sealighter_errors.h"
#include "sealighter_util.h"
#include "sealighter_json.h"
#include "sealighter_predicates.h"
#include "sealighter_handler.h"
#include "sealighter_provider.h"

// -------------------------
// GLOBALS - START
// -------------------------

// Add aliases to make code cleaner
namespace kpc = krabs::predicates::comparers;
namespace kpa = krabs::predicates::adapters;


// Holds a KrabsETW User Session Trace if needed
static user_trace* g_user_session = NULL;

// Holds a KrabsETW Kernel Session Trace if needed
static kernel_trace* g_kernel_session = NULL;

// -------------------------
// GLOBALS - END
// -------------------------
// PRIVATE FUNCTIONS - START
// -------------------------

/*
    Adds a single property comparer filter to a list
*/
template <
    typename ComparerA,
    typename ComparerW
>
void add_filter_to_vector_property_compare_item
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
                predicates::details::property_view_predicate<
                std::string,
                kpa::generic_string<char>,
                ComparerA
                >
            >(new predicates::details::property_view_predicate<
                std::string,
                kpa::generic_string<char>,
                ComparerA
            >(
                name,
                val,
                kpa::generic_string<char>(),
                ComparerA()
                ));
            list.emplace_back(pred);
        }
        else if (type == "STRINGW") {
            std::wstring val = convert_str_wstr(item["value"].get<std::string>());

            auto pred = std::shared_ptr<
                predicates::details::property_view_predicate<
                std::wstring,
                kpa::generic_string<wchar_t>,
                ComparerW
                >
            >(new predicates::details::property_view_predicate<
                std::wstring,
                kpa::generic_string<wchar_t>,
                ComparerW
            >(
                name,
                val,
                kpa::generic_string<wchar_t>(),
                ComparerW()
                ));
            list.emplace_back(pred);
        }
        else {
            // Raise a parse error, type has to be a string
            throw nlohmann::detail::exception(
                nlohmann::detail::parse_error::create(0, 0,
                    "The 'type' of a Property Comparer must be 'STRINGA' or 'STRINGW'"));
        }
    }
    else {
        // Raise a parse error, properites *must* have all these fields
        throw nlohmann::detail::exception(
            nlohmann::detail::parse_error::create(
                0, 0, "Properties must have a 'name', 'type' AND 'value' keys "));
    }
}


/*
    Adds a property comparer filter to a list
*/
template <
    typename ComparerA,
    typename ComparerW
>
void add_filter_to_vector_property_compare
(
    json root,
    std::string element,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root[element].is_null()) {
        printf("        %s: %s\n", element.c_str(), convert_json_string(root[element], false).c_str());
        if (root[element].is_array()) {
            for (json item : root[element]) {
                add_filter_to_vector_property_compare_item<ComparerA, ComparerW>(item, list);
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<sealighter_any_of>(new sealighter_any_of(list)));
            }
        }
        else {
            add_filter_to_vector_property_compare_item<ComparerA, ComparerW>(root[element], pred_vector);
        }
    }
}


/*
    Add a single "property is" filter to a list. 
*/
void add_filter_to_vector_property_is_item
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
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::string>>
                (new sealighter_property_is<std::string>(name, val)));
        }
        else if (type == "STRINGW") {
            auto val = convert_str_wstr(item["value"].get<std::string>());
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::wstring>>
                (new sealighter_property_is<std::wstring>(name, val)));
        }
        else if (type == "INT8") {
            auto val = item["value"].get<std::int8_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::int8_t>>
                (new sealighter_property_is<std::int8_t>(name, val)));
        }
        else if (type == "UINT8") {
            auto val = item["value"].get<std::uint8_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::uint8_t>>
                (new sealighter_property_is<std::uint8_t>(name, val)));
        }
        else if (type == "INT16") {
            auto val = item["value"].get<std::int16_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::int16_t>>
                (new sealighter_property_is<std::int16_t>(name, val)));
        }
        else if (type == "UINT16") {
            auto val = item["value"].get<std::uint16_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::uint16_t>>
                (new sealighter_property_is<std::uint16_t>(name, val)));
        }
        else if (type == "INT32") {
            auto val = item["value"].get<std::int32_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::int32_t>>
                (new sealighter_property_is<std::int32_t>(name, val)));
        }
        else if (type == "UINT32") {
            auto val = item["value"].get<std::uint32_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::uint32_t>>
                (new sealighter_property_is<std::uint32_t>(name, val)));
        }
        else if (type == "INT64") {
            auto val = item["value"].get<std::int64_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::int64_t>>
                (new sealighter_property_is<std::int64_t>(name, val)));
        }
        else if (type == "UINT64") {
            auto val = item["value"].get<std::uint64_t>();
            list.emplace_back(std::shared_ptr<sealighter_property_is<std::uint64_t>>
                (new sealighter_property_is<std::uint64_t>(name, val)));
        }
    }
    else {
        // Raise an parse error, properites *must* have all these fields
        throw nlohmann::detail::exception(
            nlohmann::detail::parse_error::create
            (0, 0, "Properties must have a 'name', 'type' AND 'value' keys "));
    }
}


/*
    Add the "Property Is" filter to a list,
    if availible. This is a different type of predicate
    to both the basic predicates and the other property comparer ones
*/
void add_filter_to_vector_property_is
(
    json root,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root.is_null()) {
        printf("        Property Is: %s\n", convert_json_string(root, false).c_str());
        if (root.is_array()) {
            for (json item : root) {
                add_filter_to_vector_property_is_item(item, list);
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<sealighter_any_of>(new sealighter_any_of(list)));
            }
        }
        else {
            add_filter_to_vector_property_is_item(root, pred_vector);
        }
    }
};

/*
    Adds a basic filter with two values
*/
template <
    typename TPred,
    typename TJson1=std::uint64_t,
    typename TJson2=std::uint64_t
>
void add_filter_to_vector_basic_pair
(
    json root,
    std::string element,
    std::string item1_name,
    std::string item2_name,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root[element].is_null()) {
        printf("        %s: %s\n", element.c_str(), convert_json_string(root[element], false).c_str());
        if (root[element].is_array()) {
            for (json item : root[element]) {
                if (!item[item1_name].is_null() && !item[item2_name].is_null()) {
                    TJson1 item1 = item[item1_name].get<TJson1>();
                    TJson2 item2 = item[item2_name].get<TJson2>();
                    list.emplace_back(std::shared_ptr<TPred>(new TPred(item1, item2)));
                }
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<sealighter_any_of>(new sealighter_any_of(list)));
            }
        }
        else {
            if (!root[element][item1_name].is_null() && !root[element][item2_name].is_null()) {
                TJson1 item1 = root[element][item1_name].get<TJson1>();
                TJson2 item2 = root[element][item2_name].get<TJson2>();
                pred_vector.emplace_back(std::shared_ptr<TPred>(new TPred(item1, item2)));
            }
        }
    }
}


/*
    Parse JSON to create a KrabsETW filter and add it to a list
    JSON can be a single item, or an array of items that we will 'OR'
*/
template <
    typename TPred,
    typename TJson1=std::uint64_t
>
void add_filter_to_vector_basic
(
    json root,
    std::string element,
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector
)
{
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
    if (!root[element].is_null()) {
        printf("        %s: %s\n", element.c_str(), convert_json_string(root[element], false).c_str());
        // If a list, filter can be any of them
        if (root[element].is_array()) {
            for (json item : root[element]) {
                list.emplace_back(std::shared_ptr<TPred>(new TPred(item.get<TJson1>())));
            }
            if (!list.empty()) {
                pred_vector.emplace_back(std::shared_ptr<sealighter_any_of>(new sealighter_any_of(list)));
            }
        }
        else {
            pred_vector.emplace_back(std::shared_ptr<TPred>
                (new TPred(root[element].get<TJson1>())));
        }
    }
}


/*
    Parse JSON to add filters to a vector list
*/
int add_filters_to_vector
(
    std::vector<std::shared_ptr<predicates::details::predicate_base>>& pred_vector,
    json json_list
)
{
    int status = ERROR_SUCCESS;
    try {
        // Add the basic single-value filters
        add_filter_to_vector_basic<predicates::id_is>
            (json_list, "event_id_is", pred_vector);
        add_filter_to_vector_basic<predicates::opcode_is>
            (json_list, "opcode_is", pred_vector);
        add_filter_to_vector_basic<predicates::process_id_is>
            (json_list, "process_id_is", pred_vector);
        add_filter_to_vector_basic<predicates::version_is>
            (json_list, "version_is", pred_vector);

        // Add all the property filters
        add_filter_to_vector_property_is(json_list["property_is"], pred_vector);

        add_filter_to_vector_property_compare<
            kpc::equals<std::equal_to<kpa::generic_string<char>::value_type>>,
            kpc::equals<std::equal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_equals", pred_vector);

        add_filter_to_vector_property_compare<
            kpc::equals<iequal_to<kpa::generic_string<char>::value_type>>,
            kpc::equals<iequal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_iequals", pred_vector);

        add_filter_to_vector_property_compare<
            kpc::contains<std::equal_to<kpa::generic_string<char>::value_type>>,
            kpc::contains<std::equal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_contains", pred_vector);

        add_filter_to_vector_property_compare<
            kpc::contains<iequal_to<kpa::generic_string<char>::value_type>>,
            kpc::contains<iequal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_icontains", pred_vector);

        add_filter_to_vector_property_compare<
            kpc::starts_with<std::equal_to<kpa::generic_string<char>::value_type>>,
            kpc::starts_with<std::equal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_starts_with", pred_vector);

        add_filter_to_vector_property_compare<
            kpc::starts_with<iequal_to<kpa::generic_string<char>::value_type>>,
            kpc::starts_with<iequal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_istarts_with", pred_vector);

        add_filter_to_vector_property_compare<
            kpc::ends_with<std::equal_to<kpa::generic_string<char>::value_type>>,
            kpc::ends_with<std::equal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_ends_with", pred_vector);

        add_filter_to_vector_property_compare<
            kpc::ends_with<iequal_to<kpa::generic_string<char>::value_type>>,
            kpc::ends_with<iequal_to<kpa::generic_string<wchar_t>::value_type>>
        >(json_list, "property_iends_with", pred_vector);

        // Add own own created Predicates
        add_filter_to_vector_basic<sealighter_max_events_total, std::uint64_t>
            (json_list, "max_events_total", pred_vector);
        add_filter_to_vector_basic_pair<sealighter_max_events_id>
            (json_list, "max_events_id", "id_is", "max_events", pred_vector);
        add_filter_to_vector_basic<sealighter_any_field_contains, std::string>
            (json_list, "any_field_contains", pred_vector);
        add_filter_to_vector_basic<sealighter_process_name_contains, std::string>
            (json_list, "process_name_contains", pred_vector);
        add_filter_to_vector_basic<sealighter_activity_id_is, std::string>
            (json_list, "activity_id_is", pred_vector);
    }
    catch (const nlohmann::detail::exception& e) {
        printf("failed to add filters from config to provider\n");
        printf("%s\n", e.what());
        status = SEALIGHTER_ERROR_PARSE_FILTER;
    }    
    return status;
}


/*
    Add Krabs filters to an ETW provider
*/
template <typename T>
int add_filters
(
    details::base_provider<T>* pNew_provider,
    std::shared_ptr<struct sealighter_context_t> sealighter_context,
    json json_provider
)
{
    int status = ERROR_SUCCESS;

    if (json_provider["filters"].is_null() ||
        (json_provider["filters"]["any_of"].is_null() && 
         json_provider["filters"]["all_of"].is_null() &&
         json_provider["filters"]["none_of"].is_null()
        )
       ) {
        // No filters, log everything
        printf("    No event filters\n");
        pNew_provider->add_on_event_callback([sealighter_context](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
            handle_event_context(record, trace_context, sealighter_context);
        });
    }
    else {
        // Build top-level list
        // All 3 options will eventually be ANDed together
        std::vector<std::shared_ptr<predicates::details::predicate_base>> top_list;
        if (!json_provider["filters"]["any_of"].is_null()) {
            printf("    Filtering any of:\n");
            std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
            status = add_filters_to_vector(list, json_provider["filters"]["any_of"]);
            if (ERROR_SUCCESS == status) {
                top_list.emplace_back(std::shared_ptr<sealighter_any_of>(new sealighter_any_of(list)));
            }
        }
        if (ERROR_SUCCESS == status && !json_provider["filters"]["all_of"].is_null()) {
            printf("    Filtering all of:\n");
            std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
            status = add_filters_to_vector(list, json_provider["filters"]["all_of"]);
            if (ERROR_SUCCESS == status) {
                top_list.emplace_back(std::shared_ptr<sealighter_all_of>(new sealighter_all_of(list)));
            }
        }
        if (ERROR_SUCCESS == status && !json_provider["filters"]["none_of"].is_null()) {
            printf("    Filtering none of:\n");
            std::vector<std::shared_ptr<predicates::details::predicate_base>> list;
            status = add_filters_to_vector(list, json_provider["filters"]["none_of"]);
            if (ERROR_SUCCESS == status) {
                top_list.emplace_back(std::shared_ptr<sealighter_none_of>(new sealighter_none_of(list)));
            }
        }

        // Add top level list to a filter
        if (ERROR_SUCCESS == status) {
            sealighter_all_of top_pred = sealighter_all_of(top_list);
            event_filter filter(top_pred);

            filter.add_on_event_callback([sealighter_context](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
                handle_event_context(record, trace_context, sealighter_context);
            });
            pNew_provider->add_filter(filter);
        }
    }

    return status;
}


/*
    Add Kernel Providers and Create Kernel ETW Session
*/
int add_kernel_traces
(
    json json_config,
    EVENT_TRACE_PROPERTIES  session_properties
)
{
    int status = ERROR_SUCCESS;
    kernel_provider* pNew_provider;
    std::string provider_name;

    // Initialize Session and props
    g_kernel_session = new kernel_trace();
    g_kernel_session->set_trace_properties(&session_properties);

    // Add any Kernel providers
    try {
        for (json json_provider : json_config["kernel_traces"]) {
            if (json_provider["provider_name"].is_null()) {
                printf("Invalid Provider, missing provider name\n");
                status = SEALIGHTER_ERROR_PARSE_KERNEL_PROVIDER;
                break;
            }
            provider_name = json_provider["provider_name"].get<std::string>();

            if (provider_name == "process") {
                pNew_provider = new kernel::process_provider();
            }
            else if (provider_name == "thread") {
                pNew_provider = new kernel::thread_provider();
            }
            else if (provider_name == "image_load") {
                pNew_provider = new kernel::image_load_provider();
            }
            else if (provider_name == "process_counter") {
                pNew_provider = new kernel::process_counter_provider();
            }
            else if (provider_name == "context_switch") {
                pNew_provider = new kernel::context_switch_provider();
            }
            else if (provider_name == "dpc") {
                pNew_provider = new kernel::dpc_provider();
            }
            else if (provider_name == "debug_print") {
                pNew_provider = new kernel::debug_print_provider();
            }
            else if (provider_name == "interrupt") {
                pNew_provider = new kernel::interrupt_provider();
            }
            else if (provider_name == "system_call") {
                pNew_provider = new kernel::system_call_provider();
            }
            else if (provider_name == "disk_io") {
                pNew_provider = new kernel::disk_io_provider();
            }
            else if (provider_name == "disk_file_io") {
                pNew_provider = new kernel::disk_file_io_provider();
            }
            else if (provider_name == "disk_init_io") {
                pNew_provider = new kernel::disk_init_io_provider();
            }
            else if (provider_name == "thread_dispatch") {
                pNew_provider = new kernel::thread_dispatch_provider();
            }
            else if (provider_name == "memory_page_fault") {
                pNew_provider = new kernel::memory_page_fault_provider();
            }
            else if (provider_name == "memory_hard_fault") {
                pNew_provider = new kernel::memory_hard_fault_provider();
            }
            else if (provider_name == "virtual_alloc") {
                pNew_provider = new kernel::virtual_alloc_provider();
            }
            else if (provider_name == "network_tcpip") {
                pNew_provider = new kernel::network_tcpip_provider();
            }
            else if (provider_name == "registry") {
                pNew_provider = new kernel::registry_provider();
            }
            else if (provider_name == "alpc") {
                pNew_provider = new kernel::alpc_provider();
            }
            else if (provider_name == "split_io") {
                pNew_provider = new kernel::split_io_provider();
            }
            else if (provider_name == "driver") {
                pNew_provider = new kernel::driver_provider();
            }
            else if (provider_name == "profile") {
                pNew_provider = new kernel::profile_provider();
            }
            else if (provider_name == "file_io") {
                pNew_provider = new kernel::file_io_provider();
            }
            else if (provider_name == "file_init_io") {
                pNew_provider = new kernel::file_init_io_provider();
            }
            else if (provider_name == "vamap") {
                pNew_provider = new kernel::vamap_provider();
            }
            else if (provider_name == "object_manager") {
                pNew_provider = new kernel::object_manager_provider();
            }
            else {
                printf("Invalid Provider: %s\n", provider_name.c_str());
                status = SEALIGHTER_ERROR_PARSE_KERNEL_PROVIDER;
                break;
            }

            // Create context with trace name
            if (json_provider["trace_name"].is_null()) {
                printf("Invalid Provider, missing trace name\n");
                status = SEALIGHTER_ERROR_PARSE_KERNEL_PROVIDER;
                break;
            }

            std::string trace_name = json_provider["trace_name"].get<std::string>();
            auto sealighter_context =
                std::shared_ptr<struct sealighter_context_t>(new sealighter_context_t(trace_name));
            for (json json_buffers : json_provider["buffers"]) {
                auto event_id = json_buffers["event_id"].get<std::uint32_t>();
                auto max = json_buffers["max_before_buffering"].get<std::uint32_t>();
                auto buffer_list = event_buffer_list_t(event_id, max);
                for (json json_buff_prop: json_buffers["properties_to_match"]) {
                    buffer_list.properties_to_compare.push_back(json_buff_prop.get<std::string>());
                }

                add_buffered_list(trace_name, buffer_list);
            }

            // Add any filters
            printf("Kernel Provider: %s\n", provider_name.c_str());
            status = add_filters(pNew_provider, sealighter_context, json_provider);
            if (ERROR_SUCCESS == status) {
                g_kernel_session->enable(*pNew_provider);
            }
            else {
                printf("Failed to add filters to: %s\n", provider_name.c_str());
                break;
            }

        }
    }
    catch (const nlohmann::detail::exception & e) {
        printf("invalid kernel provider in config file\n");
        printf("%s\n", e.what());
        status = SEALIGHTER_ERROR_PARSE_KERNEL_PROVIDER;
    }
    return status;
}

/*
    Add User providers and create User ETW Session
*/
int add_user_traces
(
    json json_config,
    EVENT_TRACE_PROPERTIES  session_properties,
    std::wstring session_name
)
{
    int status = ERROR_SUCCESS;
    // Initialize Session and props
    g_user_session = new user_trace(session_name);
    g_user_session->set_trace_properties(&session_properties);
    try {
        // Parse the Usermode Providers
        for (json json_provider : json_config["user_traces"]) {
            GUID provider_guid;
            provider<>* pNew_provider;
            std::wstring provider_name;
            if (json_provider["provider_name"].is_null()) {
                printf("Invalid Provider\n");
                status = SEALIGHTER_ERROR_PARSE_USER_PROVIDER;
                break;
            }

            // If provider_name is a GUID, use that
            // Otherwise pass it off to Krabs to try to resolve
            provider_name = convert_str_wstr(json_provider["provider_name"].get<std::string>());
            provider_guid = convert_wstr_guid(provider_name);
            if (provider_guid != GUID_NULL) {
                pNew_provider = new provider<>(provider_guid);
            }
            else {
                pNew_provider = new provider<>(provider_name);
            }

            wprintf(L"User Provider: %s\n", provider_name.c_str());

            // If no keywords_all or keywords_any is set
            // then set a default 'match anything'
            if (json_provider["keywords_all"].is_null() && json_provider["keywords_any"].is_null()) {
                printf("    Keywords Any: 0xffffffffffff\n");
                pNew_provider->any(0xffffffffffff);
            }
            else {
                if (!json_provider["keywords_all"].is_null()) {
                    uint64_t data = json_provider["keywords_all"].get<std::uint64_t>();
                    printf("    Keywords All: 0x%llx\n", data);
                    pNew_provider->all(data);
                }

                if (!json_provider["keywords_any"].is_null()) {
                    uint64_t data = json_provider["keywords_any"].get<std::uint64_t>();
                    printf("    Keywords Any: 0x%llx\n", data);
                    pNew_provider->any(data);
                }
            }
            if (!json_provider["level"].is_null()) {
                uint64_t data = json_provider["level"].get<std::uint64_t>();
                printf("    Level: 0x%llx\n", data);
                pNew_provider->level(data);
            }

            if (!json_provider["trace_flags"].is_null()) {
                uint64_t data = json_provider["trace_flags"].get<std::uint64_t>();
                printf("    Trace Flags: 0x%llx\n", data);
                pNew_provider->trace_flags(data);
            }

            // Check if we want a stacktrace
            // This is just a helper option, you could also set this in the trace_flags
            if (!json_provider["report_stacktrace"].is_null() && json_provider["report_stacktrace"].get<bool>()) {
                // Add the stacktrace trace flag
                pNew_provider->trace_flags(pNew_provider->trace_flags() | EVENT_ENABLE_PROPERTY_STACK_TRACE);
            }

            // Create context with trace name
            if (json_provider["trace_name"].is_null()) {
                printf("Invalid Provider, missing trace name\n");
                status = SEALIGHTER_ERROR_PARSE_KERNEL_PROVIDER;
                break;
            }

            std::string trace_name = json_provider["trace_name"].get<std::string>();
            auto sealighter_context =
                std::shared_ptr<struct sealighter_context_t>(new sealighter_context_t(trace_name));
            for (json json_buffers : json_provider["buffers"]) {
                auto event_id = json_buffers["event_id"].get<std::uint32_t>();
                auto max = json_buffers["max_before_buffering"].get<std::uint32_t>();
                auto buffer_list = event_buffer_list_t(event_id, max);
                for (json json_buff_prop : json_buffers["properties_to_match"]) {
                    buffer_list.properties_to_compare.push_back(json_buff_prop.get<std::string>());
                }

                add_buffered_list(trace_name, buffer_list);
            }

            // Add any filters
            status = add_filters(pNew_provider, sealighter_context, json_provider);
            if (ERROR_SUCCESS == status) {
                g_user_session->enable(*pNew_provider);
            }
            else {
                wprintf(L"Failed to add filters to: %s\n", provider_name.c_str());
                break;
            }
        }
    }
    catch (const nlohmann::detail::exception & e) {
        printf("invalid providers in config file\n");
        printf("%s\n", e.what());
        status = SEALIGHTER_ERROR_PARSE_USER_PROVIDER;
    }
    return status;
}


/*
    Parse the Config file, setup
    ETW Session and Krabs filters
*/
int parse_config
(
    std::string       config_path
)
{
    int     status = ERROR_SUCCESS;
    json    json_config;
    EVENT_TRACE_PROPERTIES  session_properties = { 0 };
    std::wstring    session_name = L"Sealighter-Trace";

    if (!file_exists(config_path)) {
        printf("Error: Config file doesn't exist\n");
        return SEALIGHTER_ERROR_MISSING_CONFIG;
    }

    try {
        std::ifstream   config_stream(config_path);

        // Read in config file
        config_stream >> json_config;
        config_stream.close();

        // Set defaults
        session_properties.BufferSize = 256;
        session_properties.MinimumBuffers = 12;
        session_properties.MaximumBuffers = 48;
        session_properties.FlushTimer = 1;
        session_properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_INDEPENDENT_SESSION_MODE;

        // Parse the config json for any custom properties
        json json_props = json_config["session_properties"];
        if (!json_props.is_null())
        {
            if (!json_props["session_name"].is_null()) {
                session_name = convert_str_wstr(json_props["session_name"].get<std::string>());
                wprintf(L"Session Name: %s\n", session_name.c_str());
            }

            if (!json_props["buffer_size"].is_null()) {
                session_properties.BufferSize = json_props["buffer_size"].get<std::uint32_t>();
            }

            if (!json_props["minimum_buffers"].is_null()) {
                session_properties.MinimumBuffers =
                    json_props["minimum_buffers"].get<std::uint32_t>();
            }

            if (!json_props["maximum_buffers"].is_null()) {
                session_properties.MaximumBuffers =
                    json_props["maximum_buffers"].get<std::uint32_t>();
            }

            if (!json_props["flush_timer"].is_null()) {
                session_properties.FlushTimer =
                    json_props["flush_timer"].get<std::uint32_t>();
            }

            if (!json_props["output_format"].is_null()) {
                std::string format = json_props["output_format"].get<std::string>();
                if ("stdout" == format) {
                    set_output_format(Output_format::output_stdout);
                }
                else if ("event_log" == format) {
                    set_output_format(Output_format::output_event_log);
                }
                else if ("file" == format) {
                    if (json_props["output_filename"].is_null()) {
                        printf("When output_format == 'file', also set 'output_filename'\n");
                        status = SEALIGHTER_ERROR_OUTPUT_FILE;
                    }
                    else {
                        //g_output_format = Output_format::output_file;
                        set_output_format(Output_format::output_file);
                        status = setup_logger_file(json_props["output_filename"].get<std::string>());
                    }
                }
                else {
                    printf("Invalid output_format\n");
                    status = SEALIGHTER_ERROR_OUTPUT_FORMAT;
                }
                printf("Output: %s\n", format.c_str());
            }

            if (!json_props["buffering_timout_seconds"].is_null()) {
                auto timeout = json_props["buffering_timout_seconds"].get<std::uint32_t>();
                set_buffer_lists_timeout(timeout);
            }
        }
    }
    catch (const nlohmann::detail::exception& e) {
        printf("invalid session properties in config file\n");
        printf("%s\n", e.what());
        status = SEALIGHTER_ERROR_PARSE_CONFIG_PROPS;
    }

    if (ERROR_SUCCESS == status) {
        if (json_config["user_traces"].is_null() && json_config["kernel_traces"].is_null()) {
            printf("No User or Kernel providers in config file\n");
            status = SEALIGHTER_ERROR_PARSE_NO_PROVIDERS;
        }
        else {
            if (!json_config["user_traces"].is_null()) {
                status = add_user_traces(json_config, session_properties, session_name);
            }

            // Add kernel providers if needed
            if (ERROR_SUCCESS == status && !json_config["kernel_traces"].is_null()) {
                status = add_kernel_traces(json_config, session_properties);
            }
        }
    }

    return status;
}


/*
    Run a trace, and ensure we stop if something goes wrong
*/
template <typename T>
void run_trace(trace<T>* trace)
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


/*
    Stop any running trace
*/
void stop_traces()
{
    if (NULL != g_user_session) {
        g_user_session->stop();
    }
    if (NULL != g_kernel_session) {
        g_kernel_session->stop();
    }
    // Stop bufferring thread if needed
    stop_bufferring();

    teardown_logger_file();

    // Unregister Event Logger
    (void)EventUnregisterSealighter();

}


/*
    Handler for Ctrl+C cancel events.
    Makes sure we stop our ETW Session when shutting down
*/
BOOL WINAPI crl_c_handler
(
    DWORD fdwCtrlType
)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
        stop_traces();
        return TRUE;
    }
    return FALSE;
}


// -------------------------
// PRIVATE FUNCTIONS - END
// -------------------------
// PUBLIC FUNCTIONS - START
// -------------------------
int run_sealighter
(
    std::string config_path
)
{
    int status = ERROR_SUCCESS;

    // Setup Event Logging
    status = EventRegisterSealighter();
    if (ERROR_SUCCESS != status) {
        printf("Error registering event log: %ul\n", status);
        return SEALIGHTER_ERROR_EVENTLOG_REGISTER;
    }

    // Add ctrl+C handler to make sure we stop the trace
    if (!SetConsoleCtrlHandler(crl_c_handler, true)) {
        printf("failed to set ctrl-c handler\n");
        return SEALIGHTER_ERROR_CTRL_C_REGISTER;
    }

    // Parse config file
    status = parse_config(config_path);
    if (ERROR_SUCCESS != status) {
        return status;
    }
    if (NULL == g_user_session && NULL == g_kernel_session) {
        printf("Failed to define any ETW Session\n");
        return SEALIGHTER_ERROR_NO_SESSION_CREATED;
    }
    else {
        // Setup Buffering thread if needed
        start_bufferring();

        // Start Trace we've configured
        // Don't run multithreaded if we don't have to
        if (NULL != g_user_session && NULL == g_kernel_session) {
            printf("Starting User Trace...\n");
            printf("-----------------------------------------\n");
            run_trace(g_user_session);
        }
        else if (NULL == g_user_session && NULL != g_kernel_session) {
            printf("Starting Kernel Trace...\n");
            printf("-----------------------------------------\n");
            run_trace(g_kernel_session);
        }
        else {
            // Have to multi-thread it
            printf("Starting User and Kernel Traces...\n");
            printf("-----------------------------------------\n");
            std::thread user_thread = std::thread(run_trace<details::ut>, g_user_session);
            std::thread kernel_thread = std::thread(run_trace<details::kt>, g_kernel_session);

            // Call join, blocking until both have shut down
            user_thread.join();
            kernel_thread.join();
        }
    }

    // Make sure trace is stopped
    stop_traces();

    return status;
}
