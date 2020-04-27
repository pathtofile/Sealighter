# EMon - Sysmon-Like for ETW

For ETW Reseach and arbitrary ETW logging

# Event Log installation

# TODO before v1.0
 * Document everything!
   * Including how to install manifest

# To improve post 1.0
 * Parse FILETIMEs
 * Parse SYSTEMTIMEs
 * Enable Max event counting by hash of fields, or maybe per-provider?

# Install manifest
Replace `!!EMON_LOCATION!!` with full path to emon.exe
Then:
```
wevtutil im emon_provider.man
```

```python
import json
events = list()

with open("out.json", "r") as f:
    for line in f:
        events.append(json.loads(line))
```


# Splunk
Start Splunk
```bash
docker run --rm --name splunk -d -p 8000:8000 -e 'SPLUNK_START_ARGS=--accept-license' -e 'SPLUNK_PASSWORD=hyperbutts' splunk/splunk:latest
```
