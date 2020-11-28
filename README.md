# Sealighter - Easy ETW Tracing for Researchers

![CI](https://github.com/pathtofile/Sealighter/workflows/CI/badge.svg?branch=master)

I created this project to help non-developers dive into researching Event Tracing for Windows (ETW) and Windows PreProcessor Tracing (WPP).

# Features
 - Subscribe to multiple ETW and WPP Providers at once
 - Automatically parse events into JSON without needing to know format
 - Robust Event filtering including filter chaining and filter negation
 - Output to Standard out, File, or Windows Event Log (to be ingested by other tools)
 - Get event stack traces
 - Configurable Buffering many events in a time period into one with a count, to reduce the number of events generated


![Screenshot of Sealighter running](docs/screencap.png)

# Overview
Sealighter leverages the feature-rich [Krabs ETW](https://github.com/microsoft/krabsetw) Library to enable detailed filtering and triage of ETW and WPP Providers and Events.

You can subscribe and filter multiple providers, including User mode Providers, Kernel Tracing, and WPP Tracing, and output events as JSON to either stdout, a file, or the Windows Event Log (useful for high-volume traces like `FileIO`). No knowledge of the events the provider may produce, or their format, is necessary, Sealighter automatically captures and parses any events it is asked.

Events can then be parsed from JSON in Python, PowerShell, or forwarded to Splunk or ELK for further searching.

Filtering can be done on various aspects of an Event, from its ID or Opcode, to matching a property value, to doing an arbitrary string search across the entire event (Useful in WPP traces or when you don't know the event structure, but have an idea of its contents). You can also chain multiple filters together, or negate the filter. You can also filter the maximum events per ID, useful to investigate a new provider without being flooded by similar events.

 
# Why this exists
ETW is an incredibly useful system for both Red and Blue teams. Red teams may glean insight into the inner workings of Windows components, and Blue teams might get valuable insight into suspicious activity.

A common research loop would be:
1. Identify interesting ETW Providers using `logman query providers` or Looking for WPP Traces in Binaries
2. Start a Session with the interesting providers enable, and capture events whilst doing something 'interesting'
3. Look over the results, using one or more of:
   - Eyeballing each event/grepping for words you expect to see
   - Run a script in Python or PowerShell to help filter or find interesting captured events
   - Ingesting the data into Splunk or an ELK stack for some advanced UI-driven searching

Doing this with ETW Events can be difficult, without writing code to interact with and parse events from the obtuse ETW API. If you're not a strong programmer (or don't want to deal with the API), your only other options are to use a combination of older inbuilt windows tools to write to disk as binary `etl` files, then dealing with those. WPP traces compounds the issues, providing almost no easy-to-find data about provider and their events.

Projects like [JDU2600's Event List ](https://github.com/jdu2600/Windows10EtwEvents) and [ETWExplorer](https://github.com/zodiacon/EtwExplorer) and give some static insight, but Providers often contain obfuscated event names like `Event(1001)`, meaning the most interesting data only becomes visible by dynamically running a trace and observing the output.


# So like SilkETW?
In a way, this plays in a similar space as FuzzySec's [SilkETW](https://github.com/fireeye/SilkETW). But While Silk is more production-ready for defenders, this is designed for researchers like myself, and as such contains a number of features that I couldn't get with Silk, mostly due to the different Library they used to power the tool. Please see [Here](docs/COMPARISION.md) for more information.

# Intended Audience
Probably someone who understands the basic of ETW, and really wants to dive into discovering what data you can glean from it, without having to write code or manually figure out how to get and parse events.

# Getting Started

Please read the following pages:

**[Installation](docs/INSTALLATION.md)** - How to start running Sealighter, including a simple config, and how to set up Windows Event logging if required.

**[Configuration](docs/CONFIGURATION.md)** - How to configure Sealighter, including how to specify what Providers to Log, and where to log to.

**[Filtering](docs/FILTERING.md)** - Deep dive into all the types of filtering Sealighter provides.

**[Buffering](docs/BUFFERING.md)** - How to use buffering to report many similar events as one

**[Parsing Data](docs/PARSING_DATA.md)** - How to get and parse data from Sealighter.

**[Scenarios](docs/SCENARIOS.md)** - Walkthrough example scenarios of how I've used Sealighter in my research.

**[Limitations](docs/LIMITATIONS.md)** - Things Sealighter doesn't do well or at all.

# Why it's called Sealighter
The name is a contraction of [Seafood Highlighter](https://en.wikipedia.org/wiki/Seafood_extender), which is what we call fake crab meat in Oz. As it's built on Krabs ETW, I thought the name was funny.

# Found problems?
Feel free to raise an issue, although as I state in the [comparison docs](docs/COMPARISION.md) I'm only a single person, and this is a research-ready
tool, not a production-ready.

# Props and further reading
- [Great Blog on ETW and WPP from Matt Graeber](https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7)
- [JDU2600's Event List ](https://github.com/jdu2600/Windows10EtwEvents)
- [ETWExplorer](https://github.com/zodiacon/EtwExplorer)
- [Krabs ETW, the library that powers Sealighter](https://github.com/microsoft/krabsetw)
- [SilkETW](https://github.com/fireeye/SilkETW)
