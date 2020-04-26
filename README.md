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
```
wevtutil im provider.man
```

# ???
mc.exe -A -b -c -h ..\Emon -r . event_log.mc
rc.exe /fo event_log.res event_log.rc