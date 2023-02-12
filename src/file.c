
#include "builtin.h"
#include "file.h"
#include "object.h"
#include "runtime.h"

bool is_valid_mode(const char* mode){

}

bool file_close(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to close method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    if(file->isOpen){
        fclose(file->file);
        file->file = NULL;
        file->isOpen = false;
    }
    return true;
}

ObjFile* file_open(ObjString* path, ObjString* mode){
    ObjFile* file = newFile(path, mode);
    file->file = fopen(file->path->chars, mode->chars);
    if(file->file != NULL)
        file->isOpen = true; 
    return file;
}

void initFileNativeMethods(ObjFile* file){
    addNativeObjMethod(&file->nativeMethods, "close", file_close);
}