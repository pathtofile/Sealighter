// Error list
#pragma once

// No Config file supplied
#define SEALIGHTER_ERROR_NOCONFIG 1

// Failed to register event log
#define SEALIGHTER_ERROR_EVENTLOG_REGISTER 2

// Failed to register ctrl+c handler
#define SEALIGHTER_ERROR_CTRL_C_REGISTER 3

// Failed to parse session properties
#define SEALIGHTER_ERROR_PARSE_CONFIG_PROPS 4

// Failed to parse Kernel Provider in config
#define SEALIGHTER_ERROR_PARSE_KERNEL_PROVIDER 6

// No user or kernel providers in config
#define SEALIGHTER_ERROR_PARSE_NO_PROVIDERS 7

// Bad output format in config
#define SEALIGHTER_ERROR_OUTPUT_FORMAT 8

// Failed to parse filters in config
#define SEALIGHTER_ERROR_PARSE_FILTER 9

// Failed to parse User Provider in config
#define SEALIGHTER_ERROR_PARSE_USER_PROVIDER 10

// Failed to define any ETW Session
#define SEALIGHTER_ERROR_NO_SESSION_CREATED 11

// Config file doesn't exit
#define SEALIGHTER_ERROR_MISSING_CONFIG 12

// Counld't open output stream to output file
#define SEALIGHTER_ERROR_OUTPUT_FILE 13

// Failed to resolve the specified provider
#define SEALIGHTER_ERROR_NO_PROVIDER 14
