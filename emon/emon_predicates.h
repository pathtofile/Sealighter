#pragma once
#include "krabs.hpp"
#include <vector>

/**
 * <summary>
 *   Accepts an event if any of the predicates in the vector matches
 * </summary>
 */
struct emon_any_of : krabs::predicates::details::predicate_base {
    emon_any_of(std::vector<std::shared_ptr<krabs::predicates::details::predicate_base>> list)
        : list_(list)
    {}

    bool operator()(const EVENT_RECORD& record, const krabs::trace_context& trace_context) const {
        for (auto& item : list_) {
            if (item->operator()(record, trace_context)) {
                return true;
            };
        }
        return false;
    }
private:
    std::vector<std::shared_ptr<krabs::predicates::details::predicate_base>> list_;
};

/**
 * <summary>
 *   Accepts an event if all of the predicates in the vector matches
 * </summary>
 */
struct emon_all_of : krabs::predicates::details::predicate_base {
    emon_all_of(std::vector<std::shared_ptr<krabs::predicates::details::predicate_base>> list)
        : list_(list)
    {}

    bool operator()(const EVENT_RECORD& record, const krabs::trace_context& trace_context) const {
        if (list_.empty()) {
            return false;
        }
        for (auto& item : list_) {
            if (!item->operator()(record, trace_context)) {
                return false;
            };
        }
        return true;
    }
private:
    std::vector<std::shared_ptr<krabs::predicates::details::predicate_base>> list_;
};

/**
 * <summary>
 *   Accepts an event only if none of the predicates in the vector match
 * </summary>
 */
struct emon_none_of : krabs::predicates::details::predicate_base {
    emon_none_of(std::vector<std::shared_ptr<krabs::predicates::details::predicate_base>> list)
        : list_(list)
    {}

    bool operator()(const EVENT_RECORD& record, const krabs::trace_context& trace_context) const {
        for (auto& item : list_) {
            if (item->operator()(record, trace_context)) {
                return false;
            };
        }
        return true;
    }
private:
    std::vector<std::shared_ptr<krabs::predicates::details::predicate_base>> list_;
};

/**
 * <summary>
 *   Accepts an event if its ID matches the expected value,
     But only until we see a maximum number of events.
 * </summary>
 */
struct emon_id_is_max : krabs::predicates::details::predicate_base {
    emon_id_is_max(size_t id_expected, UINT64 max_events)
        : id_expected_(USHORT(id_expected))
        , max_events_(max_events)
    {}

    bool operator()(const EVENT_RECORD& record, const krabs::trace_context&) const {
        // Match correct id first
        if (record.EventHeader.EventDescriptor.Id == id_expected_) {
            if (count_ < max_events_) {
                // Increment  count
                count_++;
                return true;
            }
        }
        return false;
    }
private:
    USHORT id_expected_;
    UINT64 max_events_;
    mutable UINT64 count_ = 0;
};

/**
 * <summary>
 *   Accepts an event if its ID matches the expected value,
     But only until we see a maximum number of events.
 * </summary>
 */
struct emon_max_events : krabs::predicates::details::predicate_base {
    emon_max_events(UINT64 max_events)
        : max_events_(max_events)
    {}

    bool operator()(const EVENT_RECORD&, const krabs::trace_context&) const {
        if (count_ < max_events_) {
            // Increment  count
            count_++;
            return true;
        }
        return false;
    }
private:
    UINT64 max_events_;
    mutable UINT64 count_ = 0;
};


/**
* <summary>
*   Returns true if the event property matches the expected value.
* </summary>
*/
template <typename T>
struct emon_property_is : krabs::predicates::details::predicate_base {
    emon_property_is(const std::wstring& property, const T& expected)
        : property_(property)
        , expected_(expected)
    {}

    bool operator()(const EVENT_RECORD& record, const krabs::trace_context& trace_context) const
    {
        krabs::schema schema(record, trace_context.schema_locator);
        krabs::parser parser(schema);

        try {
            return (expected_ == parser.parse<T>(property_));
        }
        catch (...) {
            return false;
        }
    }

private:
    const std::wstring property_;
    const T expected_;
};

/**
 * <summary>
 *   Gets a collection_view of a property using the specified adapter
 *   and executes the specified predicate against the view.
 *   This is used to provide type-specialization for properties
 *   that can be represented by the collection_view.
 * </summary>
 */
template <typename T, typename Adapter, typename Predicate>
struct emon_property_view_predicate : krabs::predicates::details::predicate_base
{
    emon_property_view_predicate(
        const std::wstring& property,
        const T& expected,
        Adapter adapter,
        Predicate predicate)
        : property_(property)
        , expected_(expected)
        , adapter_(adapter)
        , predicate_(predicate)
    { }

    bool operator()(const EVENT_RECORD& record, const krabs::trace_context& trace_context) const
    {
        krabs::schema schema(record, trace_context.schema_locator);
        krabs::parser parser(schema);

        try {
            auto view = parser.view_of(property_, adapter_);
            return predicate_(view.begin(), view.end(), expected_.begin(), expected_.end());
        }
        catch (...) {
            return false;
        }
    }

private:
    const std::wstring property_;
    const T expected_;
    Adapter adapter_;
    Predicate predicate_;
};

/**
 * Accepts events if property case insensitive contains expected value
 */
template <
    typename Adapter = krabs::predicates::adapters::generic_string<wchar_t>,
    typename T,
    typename Comparer = contains<iequal_to<typename Adapter::value_type>>>
    emon_property_view_predicate<T, Adapter, Comparer>* emon_property_icontains(
        const std::wstring& prop,
        const T& expected)
{
    return { prop, expected, Adapter(), Comparer() };
}
