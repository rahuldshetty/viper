#include<stdlib.h>

#include "memory.h"

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

#define GC_HEAP_GROW_FACTOR 2

void* reallocate( void* pointer, size_t oldSize, size_t newSize ){
    vm.bytesAllocated += newSize - oldSize;

    if(newSize > oldSize){
#ifdef DEBUG_STRESS_GC
        collectGarbage();
#endif
    }

    if(vm.bytesAllocated > vm.nextGC){
        collectGarbage();
    }

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
#ifdef DEBUG_LOG_GC
  printf("%p free type %d\n", (void*) object, object->type);
#endif


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
            ObjClosure* closure = (ObjClosure*) object;
            FREE_ARRAY(ObjUpvalue*, closure->upvalues, closure->upvalueCount);
            FREE(ObjClosure, object);
            break;
        }

        case OBJ_UPVALUE:{
            FREE(ObjUpvalue, object);
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

    free(vm.grayStack);
}


/* Garbage Collector */

void markObject(Obj* object){
    if(object==NULL) return;
    if (object->isMarked) return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*) object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    object->isMarked = true;

    if(vm.grayCapacity < vm.grayCount + 1){
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = (Obj**)realloc(
            vm.grayStack,
            sizeof(Obj*) * vm.grayCapacity
        );

        // Unable to allocate more memory for garbage collector gray stack
        if(vm.grayStack == NULL) exit(1);
    }

    vm.grayStack[vm.grayCount++] = object;
}

void markValue(Value value){
    if(IS_OBJ(value)) markObject(AS_OBJ(value));
}

void markArray(ValueArray* array){
    for(int i = 0; i < array->count; i++){
        markValue(array->values[i]);
    }
}

void markTable(Table* table){
    for(int i = 0; i<table->capacity; i++){
        Entry* entry = &table->entires[i];
        markObject((Obj*)entry->key);
        markValue(entry->value);
    }
}

void tableRemoveWhite(Table* table){
    for(int i=0 ; i < table->capacity; i++){
        Entry* entry = &table->entires[i];
        if(entry->key != NULL && !entry->key->obj.isMarked){
            tableDelete(table, entry->key);
        }
    }
}

void markRoots(){
    for(Value* slot = vm.stack; slot < vm.stackTop; slot++){
        markValue(*slot);
    }

    for(int i = 0; i < vm.frameCount; i++){
        markObject((Obj*) vm.frames[i].closure);
    }

    for(ObjUpvalue* upvalue = vm.openUpvalues; 
        upvalue != NULL; 
        upvalue = upvalue->next
    ){
        markObject((Obj*) upvalue);
    }

    markTable(&vm.globals);
    markCompilerRoots();
}

void blackendObject(Obj* object){
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void*)object);
    printValue(OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type){
        case OBJ_CLOSURE:{
            ObjClosure* closure = (ObjClosure*) object;
            markObject((Obj*) closure->function);
            for(int i = 0; i < closure->upvalueCount; i++){
                markObject((Obj*) closure->upvalues[i]);
            }
            break;
        }

        case OBJ_FUNCTION:{
            ObjFunction* function = (ObjFunction*) object;
            markObject((Obj*)function->name);
            markArray(&function->chunk.constants);
            break;
        }

        case OBJ_UPVALUE: {
            markValue(((ObjUpvalue*)object)->closed);
            break;
        }

        case OBJ_NATIVE:
        case OBJ_STRING:
            break;
    }
}

void traceReferences(){
    while(vm.grayCount > 0){
        Obj* object = vm.grayStack[--vm.grayCount];
        blackendObject(object);
    }
}

void sweep(){
    Obj* previous = NULL;
    Obj* object = vm.objects;

    while(object != NULL){
        if(object->isMarked){
            object->isMarked = false;
            previous = object;
            object = object->next;
        } else {
            Obj* unreached = object;
            object = object->next;
            if(previous != NULL){
                previous->next = object;
            } else {
                vm.objects = object;
            }

            freeObject(unreached);
            
#ifdef DEBUG_LOG_GC
            printf("%p Swepped ", (void*)unreached);
            printValue(OBJ_VAL(unreached));
            printf("\n");
#endif

        }
    }
}

void collectGarbage(){
#ifdef DEBUG_LOG_GC
    printf("-- GC BEGIN\n");
    size_t before = vm.bytesAllocated;
#endif

    markRoots();
    traceReferences();
    tableRemoveWhite(&vm.strings);
    sweep();

    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- GC END\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
         before - vm.bytesAllocated, before, vm.bytesAllocated,
         vm.nextGC);
#endif
}