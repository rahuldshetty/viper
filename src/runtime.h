#ifndef viper_runtime_h
#define viper_runtime_h

#include "object.h"
#include "vm.h"

ObjClass* addGlobalClass(const char *name, ObjClass *super);
ObjNative* addNativeMethod(void *klass, const char* name, NativeFn func);

#endif