#pragma once
#include "sealighter_krabs.h"
#include <vector>
#include <locale>

/**
 * <summary>
 *   Accepts an event if any of the predicates in the vector matches
 * </summary>
 */
struct sealighter_any_of : predicates::details::predicate_base {
    sealighter_any_of(std::vector<std::shared_ptr<predicates::details::predicate_base>> list)
        : list_(list)
    {}

    bool operator()(const EVENT_RECORD& record, const trace_context& trace_context) const {
        for (auto& item : list_) {
            if (item->operator()(record, trace_context)) {
                return true;
            };
        }
        return false;
    }
private:
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list_;
};

/**
 * <summary>
 *   Accepts an event if all of the predicates in the vector matches
 * </summary>
 */
struct sealighter_all_of : predicates::details::predicate_base {
    sealighter_all_of(std::vector<std::shared_ptr<predicates::details::predicate_base>> list)
        : list_(list)
    {}

    bool operator()(const EVENT_RECORD& record, const trace_context& trace_context) const {
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
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list_;
};

/**
 * <summary>
 *   Accepts an event only if none of the predicates in the vector match
 * </summary>
 */
struct sealighter_none_of : predicates::details::predicate_base {
    sealighter_none_of(std::vector<std::shared_ptr<predicates::details::predicate_base>> list)
        : list_(list)
    {}

    bool operator()(const EVENT_RECORD& record, const trace_context& trace_context) const {
        for (auto& item : list_) {
            if (item->operator()(record, trace_context)) {
                return false;
            };
        }
        return true;
    }
private:
    std::vector<std::shared_ptr<predicates::details::predicate_base>> list_;
};

/**
 * <summary>
 *   Accepts an event if its ID matches the expected value,
     But only until we see a maximum number of events.
 * </summary>
 */
struct sealighter_max_events_id: predicates::details::predicate_base {
    sealighter_max_events_id(uint64_t id_expected, uint64_t max_events)
        : id_expected_(USHORT(id_expected))
        , max_events_(max_events)
    {}

    bool operator()(const EVENT_RECORD& record, const trace_context&) const {
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
    uint64_t max_events_;
    mutable uint64_t count_ = 0;
};

/**
 * <summary>
 *   Accepts an event if its ID matches the expected value,
     But only until we see a maximum number of events.
 * </summary>
 */
struct sealighter_max_events_total : predicates::details::predicate_base {
    sealighter_max_events_total(UINT64 max_events)
        : max_events_(max_events)
    {}

    bool operator()(const EVENT_RECORD&, const trace_context&) const {
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
struct sealighter_property_is : predicates::details::predicate_base {
    sealighter_property_is(const std::wstring& property, const T& expected)
        : property_(property)
        , expected_(expected)
    {}

    bool operator()(const EVENT_RECORD& record, const trace_context& trace_context) const
    {
        schema schema(record, trace_context.schema_locator);
        parser parser(schema);

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
 *   Accepts if 'to_find' is it the event at all, either in a property name,
     Or in a STRINGA or STRINGW field
 * </summary>
 */
struct sealighter_any_field_contains : predicates::details::predicate_base {
    //sealighter_any_field_contains(std::string to_find)
    //    : to_findW_(convert_str_wstr(boost::algorithm::to_lower_copy(to_find)))
    //    , to_findA_(boost::algorithm::to_lower_copy(to_find))
    //{}
    sealighter_any_field_contains(std::string to_find)
    {}

    bool operator()(const EVENT_RECORD& record, const trace_context& trace_context) const {
        UNREFERENCED_PARAMETER(record);
        UNREFERENCED_PARAMETER(trace_context);
        return false;
        //schema schema(record, trace_context.schema_locator);
        //parser parser(schema);

        //for (property& prop : parser.properties())
        //{
        //    // First check the property name
        //    if (boost::algorithm::to_lower_copy(prop.name()).find(to_findW_) != std::string::npos) {
        //        return true;
        //    };

        //    switch (prop.type())
        //    {
        //        case TDH_INTYPE_ANSISTRING:
        //            if (boost::algorithm::to_lower_copy(parser.parse<std::string>(prop.name())).find(to_findA_) != std::string::npos) {
        //                return true;
        //            };
        //            break;
        //        case TDH_INTYPE_UNICODESTRING:
        //            if (boost::algorithm::to_lower_copy(parser.parse<std::wstring>(prop.name())).find(to_findW_) != std::string::npos) {
        //                return true;
        //            };
        //            break;
        //        // These *might* contains somethinf of use
        //        // If we search the raw bytes
        //        case TDH_INTYPE_MANIFEST_COUNTEDSTRING:
        //        case TDH_INTYPE_MANIFEST_COUNTEDANSISTRING:
        //        case TDH_INTYPE_RESERVED24:
        //        case TDH_INTYPE_MANIFEST_COUNTEDBINARY:
        //        case TDH_INTYPE_COUNTEDSTRING:
        //        case TDH_INTYPE_COUNTEDANSISTRING:
        //        case TDH_INTYPE_HEXINT32:
        //        case TDH_INTYPE_HEXINT64:
        //        case TDH_INTYPE_REVERSEDCOUNTEDSTRING:
        //        case TDH_INTYPE_REVERSEDCOUNTEDANSISTRING:
        //        case TDH_INTYPE_NONNULLTERMINATEDSTRING:
        //        case TDH_INTYPE_NONNULLTERMINATEDANSISTRING:
        //        case TDH_INTYPE_UNICODECHAR:
        //        case TDH_INTYPE_ANSICHAR:
        //        case TDH_INTYPE_BINARY:
        //        case TDH_INTYPE_HEXDUMP:
        //        case TDH_INTYPE_NULL:
        //            // Use BOOST to see if we can find it in the raw bytes
        //            if (check_byes(parser.parse<binary>(prop.name()).bytes())) {
        //                return true;
        //            }
        //            break;
        //        // These are just numbers or arbitrary bytes, they won't contain text
        //        case TDH_INTYPE_INT8:
        //        case TDH_INTYPE_UINT8:
        //        case TDH_INTYPE_INT16:
        //        case TDH_INTYPE_UINT16:
        //        case TDH_INTYPE_INT32:
        //        case TDH_INTYPE_UINT32:
        //        case TDH_INTYPE_INT64:
        //        case TDH_INTYPE_UINT64:
        //        case TDH_INTYPE_FLOAT:
        //        case TDH_INTYPE_DOUBLE:
        //        case TDH_INTYPE_BOOLEAN:
        //        case TDH_INTYPE_GUID:
        //        case TDH_INTYPE_FILETIME:
        //        case TDH_INTYPE_SID:
        //        case TDH_INTYPE_WBEMSID:
        //        case TDH_INTYPE_POINTER:
        //        case TDH_INTYPE_SYSTEMTIME:
        //        case TDH_INTYPE_SIZET:
        //        default:
        //            continue;
        //    }
        //}
        //return false;
    }
private:
    bool check_byes(std::vector<BYTE> bytes) const {
        //auto to_find_bytesA = std::vector<BYTE>(to_findA_.begin(), to_findA_.end());
        //auto to_find_bytesW = std::vector<BYTE>(to_findW_.begin(), to_findW_.end());
        //auto r1 = boost::make_iterator_range(bytes.begin(), bytes.end());
        //auto r2A = boost::make_iterator_range(to_find_bytesA.begin(), to_find_bytesA.end());
        //auto r2W = boost::make_iterator_range(to_find_bytesW.begin(), to_find_bytesW.end());
        //if (boost::contains(r1, r2A)) {
        //    return true;
        //}
        //else {
        //    return boost::contains(r1, r2W);
        //}
        UNREFERENCED_PARAMETER(bytes);
        return false;
    }

    std::wstring to_findW_;
    std::string to_findA_;
};
