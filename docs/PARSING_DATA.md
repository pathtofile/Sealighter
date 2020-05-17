# Parsing Data

## Event Structure
Events are outputted as JSON, for example:
```json
{
    "header": {
        "activity_id": "{00000000-0000-0000-0000-000000000000}",
        "event_flags": 576,
        "event_id": 1,
        "event_name": "",
        "event_opcode": 1,
        "event_version": 3,
        "process_id": 17964,
        "provider_name": "Microsoft-Windows-Kernel-Process",
        "task_name": "ProcessStart",
        "thread_id": 25932,
        "timestamp": "2020-05-17 11:54:24Z"
    },
    "payload": {
        "CreateTime": "2020-05-17 11:54:24Z",
        "Flags": 0,
        "ImageChecksum": 219193,
        "ImageName": "\\Device\\HarddiskVolume4\\Windows\\System32\\notepad.exe",
        "MandatoryLabel": "Mandatory Label\\Medium Mandatory Level",
        "PackageFullName": "",
        "PackageRelativeAppId": "",
        "ParentProcessID": 17964,
        "ParentProcessSequenceNumber": 15206,
        "ProcessID": 25752,
        "ProcessSequenceNumber": 33013,
        "ProcessTokenElevationType": 3,
        "ProcessTokenIsElevated": 0,
        "SessionID": 4,
        "TimeDateStamp": 587902357
    },
    "payload_types": {
        "CreateTime": "FILETIME",
        "Flags": "UINT32",
        "ImageChecksum": "UINT32",
        "ImageName": "STRINGW",
        "MandatoryLabel": "SID",
        "PackageFullName": "STRINGW",
        "PackageRelativeAppId": "STRINGW",
        "ParentProcessID": "UINT32",
        "ParentProcessSequenceNumber": "UINT64",
        "ProcessID": "UINT32",
        "ProcessSequenceNumber": "UINT64",
        "ProcessTokenElevationType": "UINT32",
        "ProcessTokenIsElevated": "UINT32",
        "SessionID": "UINT32",
        "TimeDateStamp": "UINT32"
    }
}
```

There are 3 sections to the JSON:
 - [header](#header)
 - [payload](#payload)
 - [payload_types](#payload_types)

## header
words

## payload
words

## payload_types
words


## Parsing Output Files
If the `output_format` was `file`, the events written to the file on a single line, 1 event per line. You can parse this file with Python:
```python
import json
events = list()

with open("output.json", "r") as f:
    for line in f:
        events.append(json.loads(line))
```

## Parsing Event Logs
If the `output_format` was `event_log`, the events are written to the `Sealighter/Operational` Event Log. You can use PowerShell to get and parse these Events:
```powershell
$events = Get-WinEvent -LogName "Sealighter/Operational"
```

The event `.Properties` array has the JSON blob at index 0, and each header at 

```powershell
foreach ($event in $events) {
    $event_name = $event.Properties[2].Value
    $provider_name = $event.Properties[-1].Value
    Write-Host "$provider_name - $event_name"
}
```


## With Splunk
Start Splunk
```bash
docker run --rm --name splunk -d -p 8000:8000 -e 'SPLUNK_START_ARGS=--accept-license' -e 'SPLUNK_PASSWORD=hyperbutts' splunk/splunk:latest
```

 - Into Python from file
 - Into Powershell from Event Log
 - Into Splunk from file or event log