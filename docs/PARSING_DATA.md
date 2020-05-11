# Parse Data

## With Python
```python
import json
events = list()

with open("out.json", "r") as f:
    for line in f:
        events.append(json.loads(line))
```



## With Splunk
Start Splunk
```bash
docker run --rm --name splunk -d -p 8000:8000 -e 'SPLUNK_START_ARGS=--accept-license' -e 'SPLUNK_PASSWORD=hyperbutts' splunk/splunk:latest
```