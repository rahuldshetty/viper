#include <time.h>

#include "builtin.h"
#include "object.h"
#include "value.h"
#include "vm.h"

/*
Built-in function for Viper It Bytes.
*/
Value clockNative(int argCount, Value* args){
    return NUMBER_VAL((double)clock()/CLOCKS_PER_SEC);
}

Value lenNative(int argCount, Value* args){
    Value item = args[0];
    Value result =  NUMBER_VAL(objectLength(item));
    return result;
}

Value strNative(int argCount, Value* args){
    Value item = args[0];
    return OBJ_VAL(strValue(item));
}

void registerBuiltInFunctions(){
    defineNative("clock", clockNative);
    defineNative("len", lenNative);
    defineNative("str", strNative);
}


