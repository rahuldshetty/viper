#ifndef viper_common_h
#define viper_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

// No. of local variables that can persist in a scope
#define UINT8_COUNT (UINT8_MAX + 1)

// Remove below lines to print stack trace
#undef DEBUG_PRINT_CODE
#undef DEBUG_TRACE_EXECUTION

#endif