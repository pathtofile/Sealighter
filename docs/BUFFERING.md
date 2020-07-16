# Buffering
Buffering enables the reporting of many similar events in a time period as one with a count.

Events are buffered together by collapsing events with a matching set of properties.

For example, in a Process Trace, you could buffer all process starts of `cmd.exe` together,
only reporting one event per minute. That event with contain a `buffered_count` field with the
number of `cmd.exe`s seen that minute.


There are two things to configure to enable buffering:
 - [Buffering Timeout](#Buffering%20Timeout)
 - [Per-Provider Buffers](#Per-Provider%20Buffers)

## Buffering Timeout
By setting the `buffering_timout_seconds` option in the `session_properties` config
header sets the time between 'flushes' of events, and therefore the time period to
report similar events as one:
```json
    "session_properties": {
        "buffering_timout_seconds":  10
    },
    // ...
```
If not set, the default is 30 seconds.


## Per-Provider Buffers
To enable buffering for a provider, add the `buffer` key:
```json
    "user_traces": [
        {
            "trace_name": "ProcTrace01",
            "provider_name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16,
            "buffers": [
                {
                    "event_id": 1,
                    "max_before_buffering": 1,
                    "fields": [
                        "ImageName"
                    ]
                }
            ]
        }
```

Buffers are configured per event ID, they have the following fields:
 - [event_id](#event_id)
 - [max_before_buffering](#max_before_buffering)
 - [properties_to_match](#properties_to_match)


### event_id
The Event ID to buffer


### max_before_buffering
The number of events to report as-is before starting to buffer.
This enables you to specify "No more than X matching events per Y seconds".
If set to `0` then all matching events will be buffered together.


### properties_to_match
This is an array of the names of the event properties to buffer events on.
Events that have the same values in all of these fields are considered 'the same'
and will be buffered together.
