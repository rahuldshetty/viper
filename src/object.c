#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

Obj* allocateObject(size_t size, ObjType type);

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType);

bool isObjType(Value value, ObjType type){
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString* allocateString(char* chars, int length, uint32_t hash){
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    tableSet(&vm.strings, string, NULL_VAL);
    return string;
}

// Hash Function: FNV-1a
uint32_t hashString(const char* key, int length){
    uint32_t hash = 2166136261u;
    for(int i=0;i<length;i++){
        hash ^= (uint32_t) key[i];
        hash *= 16777619; 
    }
    return hash;
}

ObjString* copyString(const char* chars, int length){
    uint32_t hash = hashString(chars, length);

    // Cache string object and re-use
    ObjString* interned = tableFindString(
        &vm.strings, chars, length, hash
    );
    if(interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

void printFunction(ObjFunction* function){
    if(function->name == NULL){
        printf("<script>");
        return;
    }

    printf("<fn %s>", function->name->chars);
}

Obj* allocateObject(size_t size, ObjType type){
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;

    return object;
}

ObjFunction* newFunction(){
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity=0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;   
}

void printObject(Value value){
    switch(OBJ_TYPE(value)){
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        
        case OBJ_FUNCTION:{
            printFunction(AS_FUNCTION(value));
            break;
        }

        case OBJ_NATIVE:{
            printf("<built-in fn>");
            break;
        }

    }
}

ObjString* takeString(char* chars, int length){
    uint32_t hash = hashString(chars, length);

    // Cache string object and re-use
    ObjString* interned = tableFindString(
        &vm.strings, chars, length, hash
    );
    if(interned != NULL){
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjNative* newNative(NativeFn function){
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}