// SimpleETW.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "krabs.hpp"

krabs::user_trace SIMPLEETW_TRACE(L"SimpleETW");

// typedef void(*c_provider_callback)(const EVENT_RECORD &, const krabs::trace_context &);
void handleEvent
(
    const EVENT_RECORD& record,
    const krabs::trace_context& trace_context
)
{
    krabs::schema schema(record, trace_context.schema_locator);
    //krabs::schema schema(record, trace_context.schema_locator);

    //std::wcout << L"Event " << schema.event_id();
    //std::wcout << L"(" << schema.event_name() << L") received." << std::endl;

    if (schema.event_id() == 1) {
        // The event we're interested in has a field that contains a bunch of
        // info about what it's doing. We can snap in a parser to help us get
        // the property information out.
        krabs::parser parser(schema);

        // We have to explicitly name the type that we're parsing in a template
        // argument.
        // We could alternatively use try_parse if we didn't want an exception to
        // be thrown in the case of failure.
        std::wstring imageName = parser.parse<std::wstring>(L"ImageName");
        std::wcout << L"\\ImageName: " << imageName << std::endl;
    }
}

BOOL WINAPI CtrlCHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
    case CTRL_C_EVENT:
        SIMPLEETW_TRACE.stop();
        return TRUE;
    }
    return FALSE;
}

int main()
{

    if (!SetConsoleCtrlHandler(CtrlCHandler, TRUE))
    {
        printf("Failed to set ctrl-C handler\n");
        return 1;
    }

    krabs::provider<> provider(L"Microsoft-Windows-Kernel-Process");

    provider.any(0x10);

    krabs::event_filter filter(krabs::predicates::id_is(7937));
    // filter.add_on_event_callback(...
    //provider.add_filter(filter);
    provider.add_on_event_callback((krabs::c_provider_callback)&handleEvent);

    SIMPLEETW_TRACE.enable(provider);
    SIMPLEETW_TRACE.start();
}
