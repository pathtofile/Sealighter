# Parsing Data

## Event Structure
Events are outputted as JSON, for example:
```json

```

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
If the `output_format` was `event_log`, the events are written to the `Emon/Operational` Event Log. You can use PowerShell to get and parse these Events:
```powershell
$events = Get-WinEvent -LogName "Emon/Operational"

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