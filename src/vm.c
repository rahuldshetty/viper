#include <stdarg.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"
#include "vm.h"

VM vm;

void initVM(){
    resetStack();
}

void resetStack(){
    vm.stackTop = vm.stack;
}

void runtimeError(const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

void freeVM(){

}

InterpretResult run(){
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

    // TODO: Optimize inplace stack binary operation
    #define BINARY_OP(valueType, op) \
        do { \
            if(!IS_NUMBER(peek_stack(0)) || !IS_NUMBER(peek_stack(1))) { \
                runtimeError("Operand must be a number."); \
                return INTERPRET_RUNTIME_ERROR; \
            }   \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push( valueType( a op b ) ); \
        } while (false)

    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            printf("          ");
            for(Value* slot = vm.stack; slot < vm.stackTop; slot++){
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");

            disassembleInstruction(vm.chunk,
                                    (int)(vm.ip - vm.chunk->code));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()){
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                printValue(constant);
                printf("\n");
                break;
            }

            case OP_NULL: push(NULL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;
            
            
            // Binary Operators
            case OP_ADD:            BINARY_OP(NUMBER_VAL, +); break;
            case OP_MINUS:          BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:       BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:         BINARY_OP(NUMBER_VAL, /); break;

            // Unary Operators
            case OP_NEGATE:         
                if(!IS_NUMBER(peek_stack(0))){
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                *(vm.stackTop-1) = NUMBER_VAL(-AS_NUMBER(*(vm.stackTop-1))); 
                break;

            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }
        }
    }
    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}

InterpretResult interpret(const char* source){
    Chunk chunk;
    initChunk(&chunk);

    if(!compile(source, &chunk)){
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}

void push(Value value){
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}

Value peek_stack(int distance){
    return vm.stackTop[-1 - distance];
}