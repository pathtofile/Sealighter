# TODO - Scenarios

Use this to detail example of how to use various filtering to explore ETW i.

Examples include:

## Basic Process Tracer


## Find correlated Events
Using `none_of->activity_id_is->"{00000000-0000-0000-0000-000000000000}"` to find correlated events.
Needs to be a provider that uses activity IDs, not all do :/
Prefably we would demonstrate events with matching activity IDs across providers


# Track process execution
Using `process_name_contains` with multiple providers at once.
Use Zack Brown's blog as template: https://zacbrown.org/2017/04/11/hidden-treasure-intrusion-detection-with-etw-part-1

# Find data in any field
Using `any_field_contains` to find exec in ShellExecute WPP trace (i.e. a trace with no prior event data)
ShellExecute WPP Provider: "{382b5e24-181e-417f-a8d6-2155f749e724}"
ShellExecute Code:
```powershell
$shellobj = New-Object -ComObject Shell.Application
$shellobj.ShellExecute("cmd.exe", "/C whoami")
```
