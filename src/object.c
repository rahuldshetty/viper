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
    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NULL_VAL);
    pop();
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
    object->isMarked = false;

    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

    return object;
}

ObjFunction* newFunction(){
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity=0;
    function->name = NULL;
    function->upvalueCount = 0;
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

        case OBJ_CLOSURE:{
            printFunction(AS_CLOSURE(value)->function);
            break;
        }

        case OBJ_UPVALUE:{
            printf("upvalue");
            break;      
        }

        case OBJ_CLASS:{
            printf("<class '%s'>", AS_CLASS(value)->name->chars);
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
ObjClosure* newClosure(ObjFunction* function){
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    
    for(int i = 0; i < function->upvalueCount; i++){
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;

    closure->upvalueCount = function->upvalueCount;
    closure->upvalues = upvalues;

    return closure;
}

ObjUpvalue* newObjUpvalue(Value* slot){
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->next = NULL;
    upvalue->closed = NULL_VAL;
    return upvalue;
}

ObjClass* newClass(ObjString* name){
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    return klass;
}