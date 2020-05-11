# Configuration File

The EMon config file is how you specifiy what events from what providers to log, how to log them, and other ETW session properties.

The file is in JSON. An example config file looks like this:
```json
{
    "session_properties": {
        "session_name": "My-Process-Trace",
        "output_format": "stdout"
    },
    "user_providers": [
        {
            "name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16
        },
        {
            "name": "{382b5e24-181e-417f-a8d6-2155f749e724}",
            "filters": {
                "any_of": {
                    "opcode_is": [1, 2]
                }
            }
        },
    ],
    "kernel_providers": [
        {
            "name": "process",
        }
    ]
}
```

Config Files have 3 Parts:
 - [session_properties](#session_properties)
 - [user_providers](#user_providers)
 - [kernel_providers](#kernel_providers)

_____________

# session_properties
These are where you specify properties of the ETW Session, e.g:
```json
"session_properties": {
    "session_name": "My-Trace",
    "output_format": "stdout",
    "output_filename": "path/to/output.json",
},
```
You can specify the following options:

### session_name
The name of the ETW Session.
Default: EMon

### output_format
Where to output the events to. Can be one of:
 - stdout
 - event_log
 - file

If specifying file, also specify `output_filename`:
```json
"session_properties": {
    "output_format": "stdout",
    "output_filename": "path/to/output.json",
},
```

### output_filename
If outputing to a file, the path to write the output events to.

The following are advanced session properties:
### buffer_size
The Size of the in-memory buffer.
Default: 256

### minimum_buffers
Minumum Buffers to allocate. Default 12

### maximum_buffers
Max Buffers to allocate. Default 48

### flush_timer
Buffer Flush timer in seconds. Default 1

_____________

# user_providers
This is an array of the Usermode or WPP provders you want to subscribe to, e.g.:
```json
    "user_providers": [
        {
            "name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16
        },
        {
            "name": "{382b5e24-181e-417f-a8d6-2155f749e724}",
            "filters": {
                "any_of": {
                    "opcode_is": [1, 2]
                }
            }
        },
    ]
```

User Providers have the following options, all are optional except for `name`:

### name
The name or GUID of the Provider to enable.
For WPP Traces, this *must* be the GUID.

### keywords_any
Only report on Events that has these at least some of these keyword flags. See [Scenarios](SCENARIOS.md) for examples on finding information on a provider's kywords.

Whilst you can also use filters to filter based on keywords (fileters explainer later), the `keywords_any` filtering happens in the Kernel, instead of in userland inside EMon, and is therefore much more efficient to filter.

It is advices to `keywords_any` as much as possible to ensure you don't drop any events.

### keywords_all
Similar to `keywords_any`, but an event must match all of the keywords.

If neither `keywords_any` or `keywords_all` is specified, all events will be passed onto the filters to be reported on.

`keywords_any` and `keywords_all` take precedence of filters.


### Level
Only report if events are at least this logging level.
Like the `keywords_*` options, it is more efficient use this instead of a Filter, and this will take precedence of a Filter

### trace_flags
Any advanced flags

### filters
An array of filters to further filter the events to report on. These can be quite complex, so read the [Filtering](FILTERING.md) section for details.

_____________

# kernel_providers

This is an array of the special sub-providers of the Special `NT Kernel Trace` that you wish to log, e.g.:
```json
"kernel_providers": [
    {
        "name": "process",
        "filters": {
            "any_of": {
                "opcode_is": [1, 2]
            }
        }
    },
    {
        "name": "image_load",
    }
]
```
Kernel Providers have two options:

### name
The kernel provider to log. Must be one of:
- process
- thread
- image_load
- process_counter
- context_switch
- dpc
- interrupt
- system_call
- disk_io
- disk_file_io
- disk_init_io
- thread_dispatch
- memory_page_fault
- memory_hard_fault
- virtual_alloc
- network_tcpip
- registry
- alpc
- split_io
- driver
- profile
- file_io
- file_init_io
- debug_print
- vamap_provider
- object_manager


### filters
Like `user_providers`, this is a list of filters to filter the events to report on. These can be quite complex, so read the [Filtering](FILTERING.md) section for details.
