
#include "runtime.h"

ObjNative* addNativeMethod(Table* method, const char* name, NativeFn func){
    ObjString* mname = copyString(name, strlen(name));
    ObjNative* natFn = newNative(func);
    tableSet(method, mname, OBJ_VAL(natFn));
    return natFn;
}

ObjNative* addNativeObjMethod(Table* method, const char* name, NativeObjFn func){
    ObjString* mname = copyString(name, strlen(name));
    ObjNative* natFn = newObjNative(func);
    tableSet(method, mname, OBJ_VAL(natFn));
    return natFn;
}