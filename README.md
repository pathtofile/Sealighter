# EMon - Sysmon-Like for ETW

For ETW Reseach and arbitrary ETW logging

# Event Log installation

# TODO before v1.0
 * Get property predicates working on heap...
 * Add Max-Event predicate
 * Output to Event Log
 * Document everything!

# To improve post 1.0
 * Parse FILETIMEs
 * Parse SYSTEMTIMEs
 * Parse SIDs
 * Enable Max event counting by hash of fields, or maybe per-provider?
 * Fix use of Macro, and other code cleanups


mc.exe -A -b -c -h ..\Emon -r . event_log.mc
rc.exe /fo event_log.res event_log.rc