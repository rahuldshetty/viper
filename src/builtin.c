#include <time.h>

#include "builtin.h"

/*
Built-in function for Viper It Bytes.
*/
Value clockNative(int argCount, Value* args){
    return NUMBER_VAL((double)clock()/CLOCKS_PER_SEC);
}

Value lenNative(int argCount, Value* args){
    Value item = args[0];
    return NUMBER_VAL(objectLength(item));
}

void registerBuiltInFunctions(){
    defineNative("clock", clockNative);
    defineNative("len", lenNative);
}


