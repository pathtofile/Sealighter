
# Setup

Sealighter runs on Windows 7+, x64.

First, grab the latest binary from [Github Releases](https://github.com/pathtofile/Sealighter/releases).\

You might also need the latest Visual C runtime: https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads

## Log to Standard Output or a File

Simply run `sealighter.exe` as Administrator, passing in the path to a JSON config file, e.g.:
```batch
sealighter.exe path\to\config.json
```

For details on what the config file looks like see **[Configuration](CONFIGURATION.md)**, but a simple example to log all process creations and terminations looks like:
```json
{
    "session_properties": {
        "session_name": "My-Process-Trace",
        "output_format": "stdout"
    },
    "user_traces": [
        {
            "trace_name": "proc_trace",
            "provider_name": "Microsoft-Windows-Kernel-Process",
            "keywords_any": 16
        }
    ]
}
```


# Log to Windows Event Log
For high-volume providers, it might be more efficient to log to the Windows Event Log, to ensure events aren't dropped. Prior to being able to write to the Windows Event Log, we need parse some data to the Event Log service.

Download the latest `sealighter_provider.man` manifest from [Github Releases](https://github.com/pathtofile/Sealighter/releases).

Open the manifest in a text edit, and replace `!!SEALIGHTER_LOCATION!!` with full path to sealighter.exe Then run the following from an elevated PowerShell or Command Prompt:
```batch
wevtutil im path/to/sealighter_provider.man
```

Once installed, in the Event Viewer UI under "Applications and Service Logs" you should see an "Sealighter" folder, and "Operational" log. You can also confirm by running this PowerShell:
```powershell
(Get-WinEvent -LogName "Sealighter/Operational").Length
```

If you move Sealighter, run the command again. To uninstall the log, run:
```batch
wevtutil um path/to/sealighter_provider.man
```

You can also change the size of the Log on disk in the Event Viewer UI, if you plan on recording many events.
