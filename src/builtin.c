#include <time.h>

#include "builtin.h"

/*
Built-in function for Viper It Bytes.
*/
Value clockNative(int argCount, Value* args){
    return NUMBER_VAL((double)clock()/CLOCKS_PER_SEC);
}

void registerBuiltInFunctions(){
    defineNative("clock", clockNative);
}


