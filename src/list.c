#include "list.h"

#include "common.h"
#include "runtime.h"
#include "table.h"
#include "value.h"

Value append(int argCount, Value self, Value* args){
    ObjList* list = AS_LIST(self);
    Value item = args[0];
    writeValueArray(&list->array, item);
    return self;
}

void initListNativeMethods(ObjList* list){
    addNativeObjMethod(&list->nativeMethods, "push", append);
}

