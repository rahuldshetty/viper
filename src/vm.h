#ifndef viper_vm_h
#define viper_vm_h

#define STACK_MAX 256

#include "chunk.h"
#include "value.h"

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value stack[STACK_MAX]; // TODO: dynamically grow Stack
    Value* stackTop;
} VM;

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