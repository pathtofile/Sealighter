# Filtering
 - [Filtering Overview](#Filtering%Overview)
 - [Filter Lists](#Filter%Lists)
 - [Filter Types](#Filter%Types)


# Filtering Overview
Filters provide a way to stem the flood of events that ETW can generate, and only
report on events of interest.

Filters are applied per-provider after checking the `keyword_any` and `keyword_all` fields. The are defined in the config file, e.g.:
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

_____________

# Filter Lists

Filters are added inside the `filter` key, into one of 3 lists. You do not have to specify each list. The lists are:
 - [any_of](#any_of)
 - [all_of](#all_of)
 - [none_of](#none_of)


## any_of
```json
"filters": {
    "any_of": {
        "opcode_is": 1,
        "id_is": 1
    }
}
```
An event will be reported if it matches *any* of the filters in the `any_of` list. In the example above, and event with opcode `1` but id `2` will still be reported.


## all_of
```json
"filters": {
    "all_of": {
        "opcode_is": 1,
        "id_is": 1
    }
}
```
An event will be reported if it matches *all* of the filters in the `all_of` list. In the example above, and event with opcode `1` but id `2` will not be reported, but an event with opcode `1` and id `1` will.

if a field is an array, e.g.:
```json
"filters": {
    "all_of": {
        "opcode_is": [1, 2],
        "id_is": 1
    }
}
```
Then an event only has to match one of the items in the array, as an event can only have one of each field. But it still has to match the rest of the items in the list - An event with id `2` and opcode `1` will match, but not id `2` opcode `2`.


## none_of
```json
"filters": {
    "none_of": {
        "opcode_is": 1,
        "id_is": 1
    }
}
```
An event will be reported only if it *does not* matche any of of the filters in the `none_of` list. In the above example, an event with id `1` and opcode `2` not be reported, but an event with id `2` and opcode `2` will be.

Like `all_of`, if a field is an array then an event only has to 'fail' a match of one of the items in the array, as an event can only have one of each field.

_____________

# Filter Types
There is a lot of different filters you can apply.
For each filter key, you can either set it to a value (either a number or string), or an array, e.g.:
```json
"filters": {
    "none_of": {
        "process_name_contains": "notepad.exe",
        "id_is": [1, 2]
    }
}
```

If an array, it is the same as if the array is a mini `any_of` list, where the event only has to match one of them to match.

The possible filters are:
 - [Header Filters](#Header%20Filters)
   - [id_is](#id_is)
   - [opcode_is](#opcode_is)
   - [process_id_is](#process_id_is)
   - [version_is](#version_is)
   - [process_name_contains](#process_name_contains)
 - [Property Filters](#Property%20Filters)
   - [property_is](#property_is)
   - [property_equals/property_iequals](#property_equals/property_iequals)
   - [property_contains/property_icontains](#property_contains/property_icontains)
   - [property_starts_with/property_istarts_with](#property_starts_with/property_istarts_with)
   - [property_ends_with/property_iends_with](#property_ends_with/property_iends_with)
 - [Discovery Filters](#Discovery%20Filters)
   - [max_events_total](#any_field_contains)
   - [max_events_total](#max_events_total)
   - [max_events_id](#max_events_id)

_____

## Header Filters
These filters filter based upon the Event Header Metadata:


### id_is
```json
"any_of": {
    "id_is": 1
}
```
An event will be reported if its Event ID matches this number.

### opcode_is
```json
"any_of": {
    "opcode_is": 1
}
```
An event will be reported if its Opcode matches this number.

### process_id_is
```json
"any_of": {
    "process_id_is": 1
}
```
An event will be reported if the PID of the process that generated the event matches this number.

### version_is
```json
"any_of": {
    "version_is": 1
}
```
An event will be reported if its Event version matches this number.

### process_name_contains
```json
"any_of": {
    "process_name_contains": "notepad.exe"
}
```
An event will be reported if the Image Name of the Process that generated the event contains this value.

_____


## Property Filters:
These filters apply to the properties within an Event.
Each takes 3 paramaters:
- `name`: The Name of the property to filter
- `value`: The value to filter on
- `type`: The `TDH_INTYPE` that the propery is.

e.g.:
```json
"any_of": {
    "property_is": {
        "name": "ImageName",
        "value": "notepad.exe",
        "type": "STRINGA",
    }
}
```
You are required to specify the `type` to reduce the runtime parsing of each propety and event, doing it once before the ETW Session starts instead of checking every event. You can check a property type by first running another trace and looking in the events `property_type` array. The possible types are:
- STRINGA
- STRINGW
- INT8
- UINT8
- INT16
- UINT16
- INT32
- UINT32
- INT64
- UINT64

These are the types of propert filters:

### property_is
```json
"any_of": {
    "property_is": {
        "name": "ExitCode",
        "value": 0,
        "type": "INT32",
    }
}
```
An event will be reported if the property exists, matches the type and value.


### property_equals/property_iequals
```json
"any_of": {
    "property_equals": {
        "name": "ImageName",
        "value": "notepad.exe",
        "type": "STRINGA",
    }
}
```
An event will be reported if the property exists, matches the type, and is equal to the value. `property_iequals` is the case-insensitive version.

### property_contains/property_icontains
```json
"any_of": {
    "property_contains": {
        "name": "ImageName",
        "value": "notepad.exe",
        "type": "STRINGA",
    }
}
```
An event will be reported if the property exists, matches the type, and contains the value. `property_icontains` is the case-insensitive version.

### property_starts_with/property_istarts_with
```json
"any_of": {
    "property_starts_with": {
        "name": "ImageName",
        "value": "notepad.exe",
        "type": "STRINGA",
    }
}
```
An event will be reported if the property exists, matches the type, and starts with the value. `property_istarts_with` is the case-insensitive version.

### property_ends_with/property_iends_with
```json
"any_of": {
    "property_ends_with": {
        "name": "ImageName",
        "value": "notepad.exe",
        "type": "STRINGA",
    }
}
```
An event will be reported if the property exists, matches the type, and ends with the value. `property_iends_with` is the case-insensitive version.

-------

## Discovery Filters
These are some special filters to help with discovery of interesting events withing a provider

### any_field_contains
```json
"any_of": {
    "any_field_contains": "notepad"
}
```
This searches all properties of all events (including propery names) for this value. It also searches for Wide and ANSI string versions. This is extremely usefull when you don't know what events a provider has, but have some diea of the data it may generate. (See [Secenarios](SCENARIOS.md) for an example of using this).


### max_events_total
```json
"any_of": {
    "max_events_total": 100
}
```
This limits the total number of events to report on. Useful for getting a snapshot of possible events generated by a high-volume provider


### max_events_id
```json
"any_of": {
    "max_events_id": {
        "id_is": 1,
        "max_events": 1
    }
}
```
This limits the total number of events matching a specific ID to report on. Useful for getting a snapshot of possible events generated by a high-volume provider.
