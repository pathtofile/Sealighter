# EMon - Sysmon-Like for ETW

For ETW Reseach and arbitrary ETW logging

# Event Log installation

# TODO before v1.0
 * Output to Event Log
   * Includes schema updates for event headers
   * Add install event log schema option
 * Document everything!

# To improve post 1.0
 * Parse FILETIMEs
 * Parse SYSTEMTIMEs
 * Parse SIDs
 * Enable Max event counting by hash of fields, or maybe per-provider?


mc.exe -A -b -c -h ..\Emon -r . event_log.mc
rc.exe /fo event_log.res event_log.rc