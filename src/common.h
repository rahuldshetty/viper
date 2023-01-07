#ifndef viper_common_h
#define viper_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "utils.h"

#define NAN_BOXING
#define DEBUG_PRINT_CODE
#define DEBUG_TRACE_EXECUTION

#define DEBUG_STRESS_GC 
#define DEBUG_LOG_GC

// No. of local variables that can persist in a scope
#define UINT8_COUNT (UINT8_MAX + 1)

// Remove below lines to print stack trace
#undef DEBUG_PRINT_CODE
#undef DEBUG_TRACE_EXECUTION

#undef DEBUG_STRESS_GC
#undef DEBUG_LOG_GC

// Uncomment to remove support for NaN Boxing
// #undef NAN_BOXING

#endif