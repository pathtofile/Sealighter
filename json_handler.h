#pragma once
#include "etw_handler.h"
#include "nlohmann/json.hpp"


std::string parse_event_to_json
(
    const   EVENT_RECORD&           record,
    const   krabs::trace_context&   trace_context,
            krabs::schema           schema,
    const   bool                    pretty_print
);
