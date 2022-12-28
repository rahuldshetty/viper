#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "builtin.h"
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

    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;

    vm.grayCapacity = 0;
    vm.grayCount = 0;
    vm.grayStack = NULL;

    initTable(&vm.globals);
    initTable(&vm.strings);

    registerBuiltInFunctions();
}

void resetStack(){
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
}

void runtimeError(const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for(int i=vm.frameCount - 1; i >= 0;i--){
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
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

// Foreign Function Interface
void defineNative(const char* name, NativeFn function){
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}


void freeVM(){
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
}

InterpretResult run(){
    CallFrame* frame = &vm.frames[vm.frameCount - 1];
    
    #define READ_BYTE() (*frame->ip++)

    #define READ_CONSTANT() ( frame->closure->function->chunk.constants.values[READ_BYTE()] )

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

            disassembleInstruction(&frame->closure->function->chunk,
                                    (int)(frame->ip - frame->closure->function->chunk.code));
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

            case OP_GET_UPVALUE:{
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }

            case OP_SET_UPVALUE:{
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek_stack(0);
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
                closeUpvalues(frame->slots);
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

            case OP_CLOSURE:{
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(function);
                push(OBJ_VAL(closure));

                for(int i = 0;i <closure->upvalueCount; i++){
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();

                    if(isLocal){
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }

                }
                
                break;
            }

            case OP_CLOSE_UPVALUE:{
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            }

            case OP_CLASS:{
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            }

            case OP_GET_PROPERTY:{
                if(!IS_INSTANCE(peek_stack(0))){
                    runtimeError("Only instances have property.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek_stack(0));
                ObjString* name = READ_STRING();

                Value value;
                if(tableGet(&instance->fields, name, &value)){
                    pop();
                    push(value);
                    break;
                }
                
                if(!bindMethod(instance->kclass, name)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SET_PROPERTY:{
                if(!IS_INSTANCE(peek_stack(1))){
                    runtimeError("Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance* instance = AS_INSTANCE(peek_stack(1));
                tableSet(&instance->fields, READ_STRING(), peek_stack(0));
                Value value = pop();
                pop();
                push(value);
                break;             
            }

            case OP_METHOD:{
                defineMethod(READ_STRING());
                break;
            }

            case OP_INVOKE:{
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                if(!invoke(method, argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
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
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    callFn(closure, 0);

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
    ObjString* b = AS_STRING(peek_stack(0));
    ObjString* a = AS_STRING(peek_stack(1));
    
    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

bool callFn(ObjClosure* closure, int argCount){
    if(argCount != closure->function->arity){
        runtimeError(
            "Expected %d arguments to <fn %s> but got %d.",
            closure->function->arity,
            closure->function->name->chars,
            argCount
        );
        return false;
    }

    if(vm.frameCount == FRAMES_MAX){
        runtimeError("Stack overflow");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

bool callValue(Value callee, int argCount){
    if(IS_OBJ(callee)){
        switch (OBJ_TYPE(callee)){
            case OBJ_FUNCTION:{
                ObjClosure* closure = newClosure(AS_FUNCTION(callee));
                return callFn(closure, argCount);
            }
            case OBJ_NATIVE:{
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            case OBJ_CLOSURE:{
                return callFn(AS_CLOSURE(callee), argCount);
            }

            case OBJ_CLASS:{
                ObjClass* kclass = AS_CLASS(callee);
                vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(kclass));

                // Constructor call handling
                Value initializer;
                if(tableGet(&kclass->methods, kclass->name, &initializer)){
                    return callFn(AS_CLOSURE(initializer), argCount);
                } else if(argCount != 0){
                    runtimeError("Expected 0 arguments but got %d.", argCount);
                    return false;
                }

                return true;
            }
            
            case OBJ_BOUND_METHOD:{
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm.stackTop[-argCount - 1] = bound->receiver;
                return callFn(bound->method, argCount);
            }

            default:
                break;
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

ObjUpvalue* captureUpvalue(Value* local){
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while(upvalue != NULL && upvalue->location > local){
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if(upvalue != NULL && upvalue->location == local){
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newObjUpvalue(local);
    createdUpvalue->next = upvalue;

    if(prevUpvalue == NULL){
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue; 
}

void closeUpvalues(Value* last){
    while(vm.openUpvalues != NULL && vm.openUpvalues->location >= last){
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

void defineMethod(ObjString* name){
    Value method = peek_stack(0); // closure
    ObjClass* klass = AS_CLASS(peek_stack(1)); 
    tableSet(&klass->methods, name, method);
    pop(); // pop closure
}


bool bindMethod(ObjClass* klass, ObjString* name){
    Value method;
    if(!tableGet(&klass->methods, name, &method)){
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(peek_stack(0), AS_CLOSURE(method));

    pop();
    push(OBJ_VAL(bound));
    return true;
}

bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount){
    Value method;
    if(!tableGet(&klass->methods, name, &method)){
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }
    return callFn(AS_CLOSURE(method), argCount);
}

bool invoke(ObjString* name, int argCount){
    Value receiver = peek_stack(argCount);

    if(!IS_INSTANCE(receiver)){
        runtimeError("Only instances have methods.");
        return false;
    }

    ObjInstance* instance = AS_INSTANCE(receiver);
    return invokeFromClass(instance->kclass, name, argCount);
}