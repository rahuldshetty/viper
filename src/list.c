#include "list.h"

#include "common.h"
#include "builtin.h"
#include "runtime.h"
#include "table.h"
#include "value.h"

bool append(int argCount, Value self, Value* args){
    if(argCount != 1){
        args[-1] = errorOutput("Expected 1 argument to push method.");
        return false;
    }

    ObjList* list = AS_LIST(self);
    Value item = args[0];
    writeValueArray(&list->array, item);
    args[-1] = self;
    return true;
}

bool remove(int argCount, Value self, Value* args){
    ObjList* list = AS_LIST(self);
    if(list->array.count != 0){
        if(argCount != 1){
            args[-1] = errorOutput("Expected 1 argument to pop method.");
            return false;
        }

        if(!IS_NUMBER(args[0])){
            args[-1] = errorOutput("Expected argument type Number to pop method.");
            return false;
        }

        int index = AS_NUMBER(args[0]);

        // Start Zero-indexed
        if(index < 0 || index >= list->array.count){
            args[-1] = errorOutput("List index out of bound.");
            return false;
        }

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

        args[-1] = popElement;
        return true;
    } else {
        args[-1] = errorOutput("Unable to pop element from empty list.");
        return false;
    }
}

void initListNativeMethods(ObjList* list){
    addNativeObjMethod(&list->nativeMethods, "push", append);
    addNativeObjMethod(&list->nativeMethods, "pop", remove);
}

