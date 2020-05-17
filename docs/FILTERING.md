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

# Standard

# "all_of", "any_of", or "none_of" list:
## Basic
 - id_is
 - opcode_is
 - process_id_is
 - version_is

## My own
 - max_events_total
 - max_events_id
 - any_field_contains
 - process_name_contains

## Property checks:
 - proprty_is
 - proprty_equals, proprty_iequals
 - proprty_contains, proprty_icontains
 - proprty_starts_with, proprty_istarts_with
 - proprty_ends_with, proprty_iends_with
