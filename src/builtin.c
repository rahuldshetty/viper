#include <time.h>

#include "builtin.h"
#include "file.h"
#include "object.h"
#include "value.h"
#include "vm.h"

/*
Built-in function for Viper It Bytes.
*/

Value errorOutput(const char* message){
    return OBJ_VAL(copyString(message, strlen(message)));
}

bool clockNative(int argCount, Value* args){
    args[-1] = NUMBER_VAL((double)clock()/CLOCKS_PER_SEC);
    return true;
}

bool lenNative(int argCount, Value* args){
    Value item = args[0];
    if(argCount != 1){
        args[-1] = errorOutput("Expected 1 argument to len method.");
        return false;
    }
    if(!IS_STRING(item) && !IS_LIST(item) && !IS_MAP(item) ){
        args[-1] = errorOutput("Invalid datatype for len method. Expected type: String, List, Map.");
        return false;
    }
    args[-1] = NUMBER_VAL(objectLength(item));
    return true;
}

bool strNative(int argCount, Value* args){
    Value item = args[0];
    if(argCount != 1){
        args[-1] = errorOutput("Expected 1 argument to str method.");
        return false;
    }
    args[-1] = OBJ_VAL(strValue(item));
    return true;
}

bool fileNative(int argCount, Value* args){
    if(argCount == 0){
        args[-1] = errorOutput("Expected atleast 1 argument to file method.");
        return false;
    }

    Value path = args[0];

    if(!IS_STRING(path)){
        args[-1] = errorOutput("Invalid datatype for path argument. Expected type: String");
        return false;
    }

    Value mode = OBJ_VAL(copyString("r", 1));
    if(argCount > 1){
        if(IS_STRING(args[1]))
        {
            mode = args[1];
        } else {
            args[-1] = errorOutput("Invalid datatype for mode argument. Expected type: String.");
            return false;
        }
    }

    // if(!is_valid_mode(AS_FILE(mode)->mode->chars)){
    //     args[-1] = errorOutput("Invalid mode for file handling.");
    //     return false;
    // }

    ObjFile* file = file_open(
        AS_STRING(path),
        AS_STRING(mode)
    );

    if(!file->isOpen){
        args[-1] = errorOutput("Invalid file path.");
        return false;
    }

    args[-1] = OBJ_VAL(
        file
    );
    return true;
}

void registerBuiltInFunctions(){
    defineNative("clock", clockNative);
    defineNative("len", lenNative);
    defineNative("str", strNative);
    defineNative("file", fileNative);
}