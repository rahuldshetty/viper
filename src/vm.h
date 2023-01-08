#ifndef viper_vm_h
#define viper_vm_h

#include <stdlib.h>

#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX ( FRAMES_MAX * UINT8_COUNT )


typedef struct{
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX]; // TODO: dynamically grow Stack
    Value* stackTop;
    Table globals;  // global variables
    Table strings;
    Table constants; // global constants
    struct ObjUpvalue* openUpvalues;
    Obj* objects;

    size_t bytesAllocated; // current allocated bytes in heap
    size_t nextGC; // threshold to trigger GC

    /* Garbage Collector */
    int grayCount;
    int grayCapacity;
    Obj** grayStack;
} VM;

VM vm;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
void resetStack();

InterpretResult run();
InterpretResult interpret(const char* source);

void push(Value value); 
Value pop();
Value peek_stack(int distance);
bool isFalsey(Value value);
void concatenate();
bool callFn(ObjClosure* closure, int argCount);
bool callValue(Value callee, int argCount);

void defineNative(const char* name, NativeFn function);

ObjUpvalue* captureUpvalue(Value* local);
void closeUpvalues(Value* last);
void defineMethod(ObjString* name);
bool bindMethod(ObjClass* klass, ObjString* name);
bool invoke(ObjString* name, int argCount);
bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount);
bool handleIndexOperator(Value, Value,Value);
bool handleIndexSetOperator(Value object, Value index, Value result);
int objectLength(Value object);

#endif