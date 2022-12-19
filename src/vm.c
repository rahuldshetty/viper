#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "vm.h"

VM vm;

void initVM(){
    resetStack();
}

void resetStack(){
    vm.stackTop = vm.stack;
}

void freeVM(){

}

InterpretResult run(){
    #define READ_BYTE() (*vm.ip++)
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
    #define BINARY_OP(op) \
        do { \
            double b = pop(); \
            double a = pop(); \
            push(a op b); \
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
            
            // Binary Operators
            case OP_ADD:            BINARY_OP(+); break;
            case OP_MINUS:          BINARY_OP(-); break;
            case OP_MULTIPLY:       BINARY_OP(*); break;
            case OP_DIVIDE:         BINARY_OP(/); break;

            // Unary Operators
            case OP_NEGATE:         push(-pop()); break;

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

InterpretResult interpret(Chunk* chunk){
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}

void push(Value value){
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}