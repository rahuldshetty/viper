#ifndef viper_runtime_h
#define viper_runtime_h

#include "common.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

ObjNative* addNativeMethod(Table* method, const char* name, NativeFn func);
ObjNative* addNativeObjMethod(Table* method, const char* name, NativeObjFn func);

bool IsNativeMethodSupported(Value);

#endif