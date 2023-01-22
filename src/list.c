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

Value remove(int argCount, Value self, Value* args){
    ObjList* list = AS_LIST(self);
    if(list->array.count != 0){
        int index = AS_NUMBER(args[0]);
        // support negative index for pop
        if(index < 0){
            index = list->array.count + index;
        }
        Value popElement = list->array.values[index];
        
        // Shift elements above popElement index to one step back
        for(int i = index; i < list->array.count - 1; i++){
            list->array.values[i] = list->array.values[i+1];
        }

        // reduce list size
        list->array.count--;

        return popElement;
    } else {
        // TODO: Handle empty list error
        return NULL_VAL;
    }
}

void initListNativeMethods(ObjList* list){
    addNativeObjMethod(&list->nativeMethods, "push", append);
    addNativeObjMethod(&list->nativeMethods, "pop", remove);
}

