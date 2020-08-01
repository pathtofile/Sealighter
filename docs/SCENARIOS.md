# Scenarios

# ETW and WPP Overviews and examples
These great blogs provide a great example of the power of ETW, TraceLogging, and WPP:
- [Data Source Analysis and Dynamic Windows RE using WPP and TraceLogging](https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7)
- [Hidden Treasure: Intrusion Detection with ETW](https://zacbrown.org/2017/04/11/hidden-treasure-intrusion-detection-with-etw-part-1)
- [Tampering with Windows Event Tracing: Background, Offense, and Defense](https://medium.com/palantir/tampering-with-windows-event-tracing-background-offense-and-defense-4be7ac62ac63)


# Example uses using Sealighter
 - [Tracking process execution](#Tracking%20process%20execution)
 - [Find data in any field](#Find%20data%20in%20any%20field)
 - [Find correlated Events](#Find%20correlated%20Events)
 - [Use Stack Traces](#Use%20Stack%20Traces)
 - [Use Buffering](#Use%20Buffering)
 - [WPP Tracing](#WPP%20Tracing)

# Tracking process execution
Lets trace a program using [Zac Brown's ideas](https://zacbrown.org/2017/04/11/hidden-treasure-intrusion-detection-with-etw-part-1). Create the Following Config:
```json
{
    "session_properties": {
        "session_name": "Sealighter-Trace",
        "output_format": "file",
        "output_filename": "output.json"
    },
    "user_traces": [
        {
            "trace_name": "powershell_dns",
            "provider_name": "Microsoft-Windows-DNS-Client",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        },
        {
            "trace_name": "powershell_wmi",
            "provider_name": "Microsoft-Windows-WMI",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        },
        {
            "trace_name": "powershell_winpshell",
            "provider_name": "Microsoft-Windows-PowerShell",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        }
    ],
    "kernel_traces": [
        {
            "trace_name": "powershell_kern_image_load",
            "provider_name": "image_load",
            "filters": {
                "any_of": {
                    "process_name_contains": "powershell.exe"
                }
            }
        },
        {
            "trace_name": "powershell_kern_tcpip",
            "provider_name": "network_tcpip",
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
Let's investigate a TraceLogging trace using [Matt Graeber's blog](https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7) as a guide.

Load up his `TLGMetadataParser.psm1` Script in PowerShell, and let's see if there's any TraceLogging providers in `Shell32.dll`:
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
2. A TraceLogging trace for the ShellExecute provider, searching for any event that contains the string "cmd.exe"
The config will look like this:
```json
{
    "session_properties": {
        "session_name": "Sealighter-Trace",
        "output_format": "stdout"
    },
    "user_traces": [
        {
            "trace_name": "proc_trace",
            "provider_name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16,
            "filters": {
                "any_of": {
                    "event_id_is": 1
                }
            }
        },
        {
            "trace_name": "shell32_trace",
            "provider_name": "{382b5e24-181e-417f-a8d6-2155f749e724}",
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
        "timestamp": "2020-05-19 12:08:08Z",
        "trace_name": "shell32_trace"
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
        "timestamp": "2020-05-19 12:08:08Z",
        "trace_name": "proc_trace"
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

We can see a generated event that we didn't know existed called `ShellExecuteExW`, which contains the program we ran (`lpFile`) and the arguments (`lpParameters`).

We can also see the information also matches up with the `Microsoft-Windows-Kernel-Process` trace, confirming that this was us that triggered the event.


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

Then it would be a case of running various ETW, TraceLogging, or WPP providers, doing "stuff", and seeing if any events get emitted.


### Use Stack Traces
To demonstrate the use of Stack Traces, let's make and trace our own C program that also does `ShellExecute`:
First we compile the following simple x86 C program and name it `shellcaller.exe`:
```c++
#include <Windows.h>
#include <shellapi.h>
#include <stdio.h>

__declspec(noinline)
static void call_com()
{
    // Print the base address to make RE faster
    // You could also find this out by degging the process
    printf("Base address: 0x%p\n", (void*)GetModuleHandleA(NULL));
    ShellExecuteA(NULL, "open", "notepad.exe", NULL, NULL, SW_SHOWNORMAL);
}

int main()
{
    call_com();
    return 0;
}
```

Then, we'll run Sealighter with the same sort of config to trace the Shell32 Events,
but also use `report_stacktrace` to get stack traces:
```json
{
    "session_properties": {
        "session_name": "Sealighter-Trace",
        "output_format": "stdout"
    },
    "user_traces": [
        {
            "trace_name": "shell32_trace",
            "provider_name": "{382b5e24-181e-417f-a8d6-2155f749e724}",
            "report_stacktrace": true,
            "filters": {
                "any_of": {
                    "any_field_contains": "shellcaller.exe"
                }
            }
        }
    ]
}
```

Start Sealighter, and once the trace has started run `shellcaller.exe` - It should launch
a `notepad.exe` process, and you should get an output similar to below (but with different numbers):
```
Base address: 0x00B80000
```

Sealighter should also output an event similar to below (but again with different numbers):
```json
{
    "header": {
        "activity_id": "{6D9D4EDB-33CC-0002-7D4A-A16DCC33D601}",
        "event_flags": 545,
        "event_id": 0,
        "event_name": "ShellExecuteExW",
        "event_opcode": 1,
        "event_version": 0,
        "process_id": 12016,
        "provider_name": "Microsoft.Windows.ShellExecute",
        "task_name": "ShellExecuteExW",
        "thread_id": 5964,
        "timestamp": "2020-05-28 10:20:07Z",
        "trace_name": "ShellExecute-Stacktrace"
    },
    "properties": {
        "PartA_PrivTags": 0,
        "dwHotKey": 0,
        "fMask": 5376,
        "hMonitor": "00000000",
        "hkeyClass": 0,
        "hwnd": "00000000",
        "lpClass": "",
        "lpDirectory": "",
        "lpFile": "notepad.exe",
        "lpIDList": "00000000",
        "lpParameters": "",
        "lpVerb": "open",
        "nShow": 1,
        "site": 7340143,
        "wilActivity": "4C170000"
    },
    "property_types": {
        "PartA_PrivTags": "UINT64",
        "dwHotKey": "UINT32",
        "fMask": "UINT32",
        "hMonitor": "OTHER",
        "hkeyClass": "UINT32",
        "hwnd": "OTHER",
        "lpClass": "STRINGW",
        "lpDirectory": "STRINGW",
        "lpFile": "STRINGW",
        "lpIDList": "OTHER",
        "lpParameters": "STRINGW",
        "lpVerb": "STRINGW",
        "nShow": "INT32",
        "site": "UINT32",
        "wilActivity": "OTHER"
    },
    "stack_trace": [
        "0x7FFA18BAB944",
        "0x7FFA18692E9F",
        "0x7FFA1868902A",
        "0x773817C3",
        "0x773811B9",
        "0x7FFA186838C9",
        "0x7FFA186832BD",
        "0x7FFA18BE266E",
        "0x7FFA18BD12F1",
        "0x7FFA18B84543",
        "0x7FFA18B844EE",
        "0x7740169C",
        "0x773F451B",
        "0x773F43D9",
        "0x76DE9F06",
        "0x76DAEA61",
        "0x76DAE21B",
        "0x76DADFF2",
        "0x76EE05F4",
        "0x76EE0591",
        "0xB8106E",
        "0x7653F989",
        "0x773F7084",
        "0x773F7054"
    ]
}
```
So we see that `shellcaller.exe` did indeed call `ShellExecute` to launch `notepad.exe`
We now also get a stack trace.

In this example, The base in-memory address of `shellcaller.exe` is `0x00B80000`.
We can also see one of the addresses in the stack is `0xB8106E`, 0x106E bytes into the image.

Now, if We open up a tool like [Ghidra](https://ghidra-sre.org), import `shellcaller.exe` into it,
and go `0x106E` bytes into the image, we should see our call to `SHELL32.DLL::ShellExecuteA`.



## Use Buffering
Buffering enables the reporting of many similar events in a time period as one with a count.

For example, Let's create a trace to log process starts, but buffer all process that have the same ImageFileName together, reporting in groups of every 10 seconds. We will use the following config:
```json
{
    "session_properties": {
        "session_name": "Sealighter-Trace",
        "output_format": "stdout",
        "buffering_timout_seconds":  10
    },
    "user_traces": [
        {
            "trace_name": "ProcTrace01",
            "provider_name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16,
            "filters": {
                "any_of": {
                    "event_id_is": 1
                }
            },
            "buffers": [
                {
                    "event_id": 1,
                    "max_before_buffering": 0,
                    "properties_to_match": [
                        "ImageName"
                    ]
                }
            ]
        }
    ]
}
```
We have created a `Microsoft-Windows-Kernel-Process` to only look at Event ID 1, i.e. `ProcessStart` events.
We have set the `buffering_timout_seconds` option to 10 seconds.
We have set 1 buffer for Event 1, to match in the Property `ImageName`.

Running this trace, we will get reports once every 10 seconds. Any matching ImageNames will be
rolled into a single event with a `buffered_count` field.

Process with different ImageNames will be their own events.


## WPP Tracing
WPP Tracing is special, as the format of the events are not contained in the trace at runtime.
The format is usually compiled into the pdb file, or in a seperate TMF file.

Curretly Sealighter cannot auto-parse WPP events, however we can get the raw event data, and parse them
after the fact with Python.

As an example, lets trace the OLE32 COM provider. Using the microsoft documentation [here](https://support.microsoft.com/en-us/help/926098/how-to-enable-com-and-com-diagnostic-tracing), first enable OLE32 tracing by runnig the following command in an elevated prompt:
```
reg add HKEY_LOCAL_MACHINE\Software\Microsoft\OLE\Tracing /v ExecutablesToTrace /t REG_MULTI_SZ /d * /f
```
Now we can run a sealighter trace.

We need to log not only the WPP provider GUID, but also the message GUID, as WPP messages get delivered
to their own provider. We know these GUIDS by reverse engineering ole32.dll. Keywords and levels in WPP traces are CHARs, so to get all events we will set both to 0xff, i.e. `255`.
We will use `dump_raw_event` to dump the raw hex-encoded bytes, and also limit it to just 10 messages,
to prevent the trace from flooding us with too much information. The end config shoud look something like this:
```json
{
    "session_properties": {
        "session_name": "Sealighter-Trace",
        "output_format": "file",
        "output_filename": "ole.json"
    },
    "user_traces": [
        {
            "trace_name": "ole32",
            "provider_name": "{BDA92AE8-9F11-4D49-BA1D-A4C2ABCA692E}",
            "keywords_any": 255,
            "level": 255
        },
        {
            "trace_name": "ole32-message",
            "provider_name": "{0F480EA8-F109-39A3-8A27-36DC7E84A294}",
            "dump_raw_event": true,
            "filters": {
                "any_of": {
                    "max_events_total": 10
                }
            },
        }
    ]
}
```
Note the GUIDs might be different with different versions of the dll.
Start the trace, wait 30 seconds, and you should have 10 events in `ole.json`.
We know from reverse engineering that the messages we logged are just a ANSI string, so
we can parse the file with this simple python script:
```python
import json
import binascii

with open("ole.json", "r") as f:
    for line in f:
        # Get the event in JSON format
        event = json.loads(line)
        process_id = event["header"]["process_id"]
        # Extract and convert the raw bytes
        raw_hex = event["raw"]
        raw_bytes = binascii.unhexlify(raw_hex)
        raw_string = raw_bytes.decode("utf16")
        # Print the string
        print(f"[{process_id}] - {raw_string}")
```
Run the script, and you should see output like the following:
```
[1516] - (onecore\com\combase\common\internal\comtrace.cxx):(InitializeTracing):(159) Starting OLE32 tracing for: C:\WINDOWS\system32\conhost.exe
[1316] - (onecore\com\combase\rpcss\objex\manager.cxx):(_ServerAllocateOIDs):(2968) process:000001634C6BC9C0 PID:618 C:\WINDOWS\System32\svchost.exe OXID:AAC34D291DA574F0
[1316] - (onecore\com\combase\rpcss\objex\manager.cxx):(ServerAllocateOIDsInternal):(2866) process:000001634C6BC9C0 PID:618 C:\WINDOWS\System32\svchost.exe Server OXID:AAC34D291DA574F0
[1316] - (onecore\com\combase\rpcss\objex\idtable.hxx):(CIdKey::CIdKey):(42) this:000000247377E8F0 ID:AAC34D291DA574F0
```

Once finished, we can stop OLE tracing by delting the reg key we created:
```
reg delete HKEY_LOCAL_MACHINE\Software\Microsoft\OLE\Tracing /v ExecutablesToTrace /f 
```

In the future Sealighter might enable you to define the event structure at runtime so events look like other ETW events, but not right now.
For more information on WPP tracing, again see [Matt Graeber's blog](https://posts.specterops.io/data-source-analysis-and-dynamic-windows-re-using-wpp-and-tracelogging-e465f8b653f7) as a guide.
