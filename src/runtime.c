
#include "runtime.h"

ObjClass* addGlobalClass(const char *name, ObjClass *super){
    ObjString* className = copyString(name, strlen(name));
    ObjClass* class = newClass(className);
    tableSet(&vm.constants, OBJ_VAL(className), OBJ_VAL(class));
    return class;
}

ObjNative* addNativeMethod(void *klass, const char* name, NativeFn func){
    ObjString* mname = copyString(name, strlen(name));
    ObjNative* natFn = newNative(func);
    
}