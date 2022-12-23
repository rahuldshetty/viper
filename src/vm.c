#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

void initVM(){
    resetStack();
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
}

void resetStack(){
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}

void runtimeError(const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for(int i=vm.frameCount - 1; i >= 0;i--){
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", 
            function->chunk.lines[instruction]
        );

        if(function->name == NULL){
            fprintf(stderr, "script\n");
        } else {
             fprintf(stderr, "%s()\n", function->name->chars);
        }
        
    }

    resetStack();
}

void freeVM(){
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
}

InterpretResult run(){
    CallFrame* frame = &vm.frames[vm.frameCount - 1];
    
    #define READ_BYTE() (*frame->ip++)

    #define READ_CONSTANT() ( frame->function->chunk.constants.values[READ_BYTE()] )

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    #define READ_SHORT() \
        ( frame->ip += 2, (uint16_t)((frame->ip[-2] << 8 ) | frame->ip[-1] ))

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

            disassembleInstruction(&frame->function->chunk,
                                    (int)(frame->ip - frame->function->chunk.code));
        #endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()){
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                // printValue(constant);
                // printf("\n");
                break;
            }

            case OP_NULL: push(NULL_VAL); break;
            case OP_TRUE: push(BOOL_VAL(true)); break;
            case OP_FALSE: push(BOOL_VAL(false)); break;

            // Comparison Operators
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a,b)));
                break;
            }
            case OP_LESS:           BINARY_OP(BOOL_VAL, <); break;
            case OP_GREATER:        BINARY_OP(BOOL_VAL, >); break;
            
            // Binary Operators
            case OP_ADD:{
                if (IS_STRING(peek_stack(0)) && IS_STRING(peek_stack(1))) {
                    concatenate();
                } else if(IS_NUMBER(peek_stack(0)) && IS_NUMBER(peek_stack(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }        
            case OP_MINUS:          BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:       BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:         BINARY_OP(NUMBER_VAL, /); break;

            case OP_NOT:
                *(vm.stackTop-1) = (BOOL_VAL(isFalsey(*(vm.stackTop-1))));
                break;

            // Unary Operators
            case OP_NEGATE:         
                if(!IS_NUMBER(peek_stack(0))){
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                *(vm.stackTop-1) = NUMBER_VAL(-AS_NUMBER(*(vm.stackTop-1))); 
                break;

            case OP_PRINT:{
                printValue(pop());
                printf("\n");
                break;
            }

            case OP_POP: {
                pop();
                break;
            }

            case OP_DEFINE_GLOBAL:{
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek_stack(0));
                pop();
                break;
            }

            case OP_SET_GLOBAL:{
                ObjString* name = READ_STRING();
                // Implicit declaration - keep only below line
                tableSet(&vm.globals, name, peek_stack(0));

                // Explicit declaration - users need to specify `var <identifier>` to declare variable
                // if( tableSet(&vm.globals, name, peek_stack(0)) ){
                //     tableDelete(&vm.globals, name);
                //     runtimeError("Undefined variable '%s'.", name->chars);
                //     return INTERPRET_RUNTIME_ERROR;
                // }
                break;
            }

            case OP_GET_GLOBAL:{
                ObjString* name = READ_STRING();
                Value value;

                if(!tableGet(&vm.globals, name, &value)){
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(value);
                break;
            }

            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek_stack(0);
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if(isFalsey(peek_stack(0))) frame->ip += offset;
                break;
            }

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_CALL:{
                int argCount = READ_BYTE();
                if(!callValue(peek_stack(argCount), argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_RETURN: {
                Value result = pop();
                vm.frameCount--;
                if(vm.frameCount == 0){
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

        }
    }
    #undef READ_STRING
    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
    #undef READ_SHORT
}

InterpretResult interpret(const char* source){
    ObjFunction* function = compile(source);
    if(function==NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    callFn(function, 0);

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

Value peek_stack(int distance){
    return vm.stackTop[-1 - distance];
}

bool isFalsey(Value value){
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

void concatenate(){
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());
    
    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
}

bool callFn(ObjFunction* function, int argCount){
    if(argCount != function->arity){
        runtimeError(
            "Expected %d arguments to <fn %s> but got %d.",
            function->arity,
            function->name->chars,
            argCount
        );
        return false;
    }

    if(vm.frameCount == FRAMES_MAX){
        runtimeError("Stack overflow");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

bool callValue(Value callee, int argCount){
    if(IS_OBJ(callee)){
        switch (OBJ_TYPE(callee)){
            case OBJ_FUNCTION:
                return callFn(AS_FUNCTION(callee), argCount);
            default:
                break;
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}