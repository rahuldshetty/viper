#ifndef viper_vm_h
#define viper_vm_h

#include "object.h"
#include "table.h"
#include "value.h"


#define FRAMES_MAX 64
#define STACK_MAX ( FRAMES_MAX * UINT8_COUNT )


typedef struct{
    ObjFunction* function;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX]; // TODO: dynamically grow Stack
    Value* stackTop;
    Table globals;
    Table strings;
    Obj* objects;
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

#endif