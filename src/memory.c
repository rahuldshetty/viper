#include<stdlib.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

void* reallocate( void* pointer, size_t oldSize, size_t newSize ){
    if(newSize == 0){
        free(pointer);
        return NULL;
    }
    void* result = realloc(pointer, newSize);
    // Out-of-memory for re-allocation
    if(result==NULL){
        exit(1);
    }
    return result;
}

void freeObject(Obj* object){
    switch (object->type){
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, object);
            break;
        }

        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*) object;
            freeChunk(&function->chunk);
            FREE(ObjFunction, object); 
            break;
        }

        case OBJ_NATIVE: {
            FREE(ObjNative, object);
            break;
        }

        case OBJ_CLOSURE:{
            FREE(ObjClosure, object);
            break;
        }

    }
}

void freeObjects(){
    Obj* object = vm.objects;
    while(object != NULL){
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}