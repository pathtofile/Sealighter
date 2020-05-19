# Scenarios

# ETW and WPP Overviews and examples
These great blogs provide a great example of the power of ETW and WPP:
- [Data Source Analysis and Dynamic Windows RE using WPP and TraceLogging](https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7)
- [Hidden Treasure: Intrusion Detection with ETW](https://zacbrown.org/2017/04/11/hidden-treasure-intrusion-detection-with-etw-part-1)
- [Tampering with Windows Event Tracing: Background, Offense, and Defense](https://medium.com/palantir/tampering-with-windows-event-tracing-background-offense-and-defense-4be7ac62ac63)


# Example uses using Sealighter
 - [Tracking process execution](#Tracking%20process%20execution)
 - [Find data in any field](#Find%20data%20in%20any%20field)
 - [Find correlated Events](#Find%20correlated%20Events)

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


----------


# Find data in any field
Let's investigate a WPP trace using [Matt Graeber's WPP blog](https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7) as a guide.

Load up his `TLGMetadataParser.psm1` Script in Powershell, and let's see if there's any WPP providers in `Shell32.dll`:
```powershell
Import-Module .\TLGMetadataParser.psm1
$shell32 = Get-TraceLoggingMetadata -Path C:\Windows\System32\shell32.dll
$shell32.Providers | Format-List
```

In the list of providers, you should see this one:
```
ProviderGUID      : 382b5e24-181e-417f-a8d6-2155f749e724
ProviderName      : Microsoft.Windows.ShellExecute
ProviderGroupGUID : 4f50731a-89cf-4782-b3e0-dce8c90476ba
```

This appears to be something to do with `ShellExecute`, a common way malware can launch programs from VBA, or other scripts.
For example, here's how you can use it from PowerShell:
```powershell
$shellobj = New-Object -ComObject Shell.Application
$shellobj.ShellExecute("notepad.exe", "test.txt")
```

Let's Create a Sealighter trace with two providers:
1. A Process Trace, where we'll get all process starts (Event Id 2)
2. A WPP trace for the ShellExecute provider, searching for any event that contains the string "cmd.exe"
The config will look like this:
```json
{
    "session_properties": {
        "session_name": "Sealighter-Trace",
        "output_format": "stdout"
    },
    "user_providers": [
        {
            "name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16,
            "filters": {
                "any_of": {
                    "id_is": 1
                }
            }
        },
        {
            "name": "{382b5e24-181e-417f-a8d6-2155f749e724}",
            "filters": {
                "any_of": {
                    "any_field_contains": "notepad.exe"
                }
            }
        }
    ]
}
```

Start the Sealighter trace, run the PowerShell `.ShellExecute` script, and you should see 2 events similar to the following:
```json
{
    "header": {
        "activity_id": "{636D6229-2DBC-0001-0F21-6E63BC2DD601}",
        "event_flags": 577,
        "event_id": 0,
        "event_name": "ShellExecuteExW",
        "event_opcode": 1,
        "event_version": 0,
        "process_id": 15724,
        "provider_name": "Microsoft.Windows.ShellExecute",
        "task_name": "ShellExecuteExW",
        "thread_id": 16392,
        "timestamp": "2020-05-19 12:08:08Z"
    },
    "properties": {
        "PartA_PrivTags": 0,
        "dwHotKey": 0,
        "fMask": 0,
        "hMonitor": "0000000000000000",
        "hkeyClass": 0,
        "hwnd": "0000000000000000",
        "lpClass": "",
        "lpDirectory": "",
        "lpFile": "notepad.exe",
        "lpIDList": "0000000000000000",
        "lpParameters": "test.txt",
        "lpVerb": "",
        "nShow": 1,
        "site": 0,
        "wilActivity": "08400000"
    }
}
{
    "header": {
        "activity_id": "{00000000-0000-0000-0000-000000000000}",
        "event_flags": 576,
        "event_id": 1,
        "event_name": "",
        "event_opcode": 1,
        "event_version": 3,
        "process_id": 15724,
        "provider_name": "Microsoft-Windows-Kernel-Process",
        "task_name": "ProcessStart",
        "thread_id": 4564,
        "timestamp": "2020-05-19 12:08:08Z"
    },
    "properties": {
        "CreateTime": "2020-05-19 12:08:08Z",
        "Flags": 0,
        "ImageChecksum": 219193,
        "ImageName": "\\Device\\HarddiskVolume4\\Windows\\System32\\notepad.exe",
        "MandatoryLabel": "Mandatory Label\\High Mandatory Level",
        "PackageFullName": "",
        "PackageRelativeAppId": "",
        "ParentProcessID": 15724,
        "ParentProcessSequenceNumber": 7866,
        "ProcessID": 16144,
        "ProcessSequenceNumber": 8055,
        "ProcessTokenElevationType": 2,
        "ProcessTokenIsElevated": 1,
        "SessionID": 1,
        "TimeDateStamp": 587902357
    }
}
```

In my example, the PID of the PowerShell that ran the command was `15724`.
We can see that PowerShell triggered an event

We can see we generated an event we didn't know existed called `ShellExecuteExW`, which containes the program we ran (`lpFile`) and the arguments (`lpParameters`).

We can see the information also matches up with the `Microsoft-Windows-Kernel-Process` trace, confirming that this was us that triggered the event.


----------

## Find correlated Events
Activity IDs are unique GUIDs in the event header for a chain of events, and enable correlation of events across time and even across different providers.

To look for providers that use activity IDs, you could use the following filter, to filter out any events that *don't* have an activity ID:
```json
"filters": {
    "none_of": {
        "activity_id_is": "{00000000-0000-0000-0000-000000000000}"
    }
}
```

Then it would be a case of running various ETW or WPP providers, doing "stuff", and seeing if any events get emmited.
