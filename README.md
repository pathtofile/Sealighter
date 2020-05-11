# EMon - Sysmon-Like tool for ETW

I created this project to help non-developers dive into researching Event Tracing for Windows (ETW) and Windows PreProcessor Tracing(WPP).

It leverages the feature-rich [Krabs ETW]([htttps://](https://github.com/microsoft/krabsetw)) Library to enable detailed filtering and triage of ETW and WPP Providers and Events.

You can subscribe and filter multiple provers, including Usermode Providers, Kernel Tracing, and WPP Tracing, and output events as JSON to either stdout, a file, or the Windows Event Log (useful for high-volume traces like `FileIO`).

Events can then be parsed in Python, PowerShell, or forwarded to Splunk or ELK for further searching.

Filtering can be done on various aspects of an Event, from it's ID or OpCode, to matching a property value, to doing an arbitarty string search across the entire event (Useful in WPP traces or when you don't know the event structure, but have an idea of it's contents). You can also chain multiple filters together, or negate the filter. You can also filter the maximum events per ID, useful to investigate a new provider without being flooded by similar events.



# Why this exists
ETW is an incredibly useful system for both Red and Blue teams. Red teams may glean insight into the inner workings of Windows components, and Blue teams might get valuble insight into suspicious activity.

But the system is incredibly obtuse - It is difficult to tell what providers produce what data. WPP only compounds that, poviding almost no easy-to-find data about provider and their events.

Projects like [JDU2600's Event List ](https://github.com/jdu2600/Windows10EtwEvents) and [ETWExplorer](https://github.com/zodiacon/EtwExplorer) and give some static insight, but Providers often contain obfuscated event names like `Event(1001)`, so I often instead opt to Dynamiclly run a trace and observe the output.


# Like SilkETW?
Yep, this plays in the same space as FuzzySec's [SilkETW](https://github.com/fireeye/SilkETW). But While Silk is more production-ready, this is designed for researchers like myself, and as such contains a number of features that I couldn't get with Silk, mostly due to the different Libary they used to power the tool. Please see [Here](docs/COMPARISION.md) for more information.

# Intended Audience
Probably someone who understands the basic of ETW, and really wants to dive into discovering what data you can glean from it.

# Getting Started

Please read the following pages:

**[Installation](docs/INSTALLATION.md)** - How to start running EMon, including a simple config, and how to setup Windows Event logging if required.

**[Configuration](docs/CONFIGURATION.md)** - How to configure EMon, including how to specify what Providers to Log, and where to log to.

**[Filtering](docs/FILTERING.md)** - Deep dive into all the types of filtering EMon provides

**[Parsing Data](docs/PARSING_DATA.md)** - How to get and parse data from EMon

**[Scenarios](docs/SCENARIOS.md)** - Walkthrough example scenarios of how I've used EMon in my research.


# Props and further reading
- [Great Blog on ETW and WPP from  Matt Graeber](https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7)
- [JDU2600's Event List ](https://github.com/jdu2600/Windows10EtwEvents)
- [ETWExplorer](https://github.com/zodiacon/EtwExplorer)
- [Krabs ETW, the library that powers EMon](https://github.com/microsoft/krabsetw)
 - [SilkETW](https://github.com/fireeye/SilkETW)