# Scenarios

# ETW and WPP Overviews and examples
These great blogs provide a great example of the power of ETW and WPP:
- [Data Source Analysis and Dynamic Windows RE using WPP and TraceLogging]()https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7
- [Hidden Treasure: Intrusion Detection with ETW](https://zacbrown.org/2017/04/11/hidden-treasure-intrusion-detection-with-etw-part-1)
- [Tampering with Windows Event Tracing: Background, Offense, and Defense](https://medium.com/palantir/tampering-with-windows-event-tracing-background-offense-and-defense-4be7ac62ac63)


# Example uses using Sealighter
 - [Tracking process execution](#Tracking%20process%20execution)
 - [Find correlated Events](#Find%20correlated%20Events)
 - [Find data in any field](#Find%20data%20in%20any%20field)

# Tracking process execution
Lets trace a program using [Zac Brown's ideas](https://zacbrown.org/2017/04/11/hidden-treasure-intrusion-detection-with-etw-part-1). Create the Following Config:
```json
{
    "session_properties": {
        "session_name": "Sealighter-Trace",
        "output_format": "file",
        "output_filename": "output.json"
    },
    "user_providers": [
        {
            "name": "Microsoft-Windows-DNS-Client",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        },
        {
            "name": "Microsoft-Windows-WMI",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        },
        {
            "name": "Microsoft-Windows-PowerShell",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        }
    ],
    "kernel_providers": [
        {
            "name": "image_load",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        },
        {
            "name": "network_tcpip",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        }
    ]
}
```

We can now start Sealighter, Open PowerShell, run mimikatz in it, then stop the trace.

Then we can use Python to parse the events:
```python
import json
events = list()

with open("output.json", "r") as f:
    for line in f:
        events.append(json.loads(line))

for event_string in events:
    event = json.loads(event_string)
    if event["provider_name"] == "MSNT_SystemTrace" and event["task_name"] == "Image":
        # Image Load:
        image_filename = event["properties"]["FileName"]
        print(f"[A] Image Loaded: {image_filename})")

    elif event["provider_name"] == "Microsoft-Windows-PowerShell" and event["event_id"] == 4104:
        # PowerShell Script
        script = event["properties"]["ScriptBlockText"]
        print(f"[B] Script: {script})")
```


## Find correlated Events
Using `none_of->activity_id_is->"{00000000-0000-0000-0000-000000000000}"` to find correlated events.
Needs to be a provider that uses activity IDs, not all do :/
Prefably we would demonstrate events with matching activity IDs across providers


# Find data in any field
Using `any_field_contains` to find exec in ShellExecute WPP trace (i.e. a trace with no prior event data)
ShellExecute WPP Provider: "{382b5e24-181e-417f-a8d6-2155f749e724}"
ShellExecute Code:
```powershell
$shellobj = New-Object -ComObject Shell.Application
$shellobj.ShellExecute("cmd.exe", "/C whoami")
```
