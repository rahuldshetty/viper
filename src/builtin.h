#ifndef viper_builtin_h
#define viper_builtin_h

#include "value.h"

void registerBuiltInFunctions();
Value errorOutput(const char* message);

#endif