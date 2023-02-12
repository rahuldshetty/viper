#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "builtin.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "map.h"
#include "memory.h"
#include "object.h"
#include "runtime.h"
#include "value.h"
#include "vm.h"

void initVM(){
    if(vm.inited){
        return;
    }

    vm.stack = NULL;
    vm.stackCapacity = 0;

    resetStack();
    vm.objects = NULL;

    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;

    vm.grayCapacity = 0;
    vm.grayCount = 0;
    vm.grayStack = NULL;

    initTable(&vm.globals);
    initTable(&vm.strings);
    initTable(&vm.constants);

    vm.inited = true;
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
    ObjString* string = copyString(name, (int)strlen(name));
    ObjNative* native = newNative(function);
    push(OBJ_VAL(string));
    push(OBJ_VAL(native));
    tableSet(&vm.globals, string, OBJ_VAL(native));
    pop();
    pop();
}


void freeVM(){
    if(!vm.inited){
        return;
    }

    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeTable(&vm.constants);

    freeObjects();
}

InterpretResult run(){
    CallFrame* frame = &vm.frames[vm.frameCount - 1];
    register uint8_t* ip = frame->ip;
    
    #define READ_BYTE() (*ip++)

    #define READ_CONSTANT() ( frame->closure->function->chunk.constants.values[READ_BYTE()] )

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    #define READ_SHORT() \
        ( ip += 2, (uint16_t)((ip[-2] << 8 ) | ip[-1] ))

    // TODO: Optimize inplace stack binary operation
    #define BINARY_OP(valueType, op) \
        do { \
            if(!IS_NUMBER(peek_stack(0)) || !IS_NUMBER(peek_stack(1))) { \
                frame->ip = ip; \
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
                    frame->ip = ip;
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }        
            case OP_MINUS:          BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY:       BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:         BINARY_OP(NUMBER_VAL, /); break;
            case OP_MOD:{
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                if(b == 0){
                    frame->ip = ip;
                    runtimeError("ZeroDivisionError: modulo by zero is invalid.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push( NUMBER_VAL( fmod(a, b) ) ); 
                break;
            }

            case OP_NOT:
                *(vm.stackTop-1) = (BOOL_VAL(isFalsey(*(vm.stackTop-1))));
                break;

            // Unary Operators
            case OP_NEGATE:         
                if(!IS_NUMBER(peek_stack(0))){
                    frame->ip = ip;
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
                //     frame->ip = ip;
                //     runtimeError("Undefined variable '%s'.", name->chars);
                //     return INTERPRET_RUNTIME_ERROR;
                // }
                break;
            }

            case OP_GET_GLOBAL:{
                ObjString* name = READ_STRING();
                Value value;

                if(!tableGet(&vm.globals, name, &value)){
                    frame->ip = ip;
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
                if(isFalsey(peek_stack(0))) ip += offset;
                break;
            }

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                ip += offset;
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                ip -= offset;
                break;
            }

            case OP_CALL:{
                int argCount = READ_BYTE();
                frame->ip = ip;
                if(!callValue(peek_stack(argCount), argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                ip = frame->ip;
                break;
            }

            case OP_LIST:{
                int itemCount = READ_BYTE();
                ObjList* list = newList();
                for(int i=0; i < itemCount; i++){
                    writeValueArray(&list->array, pop());
                }

                ObjList* reversedList = newList();
                for(int i=itemCount - 1; i >= 0; i--){
                    writeValueArray(&reversedList->array, list->array.values[i]);
                }

                push(OBJ_VAL(reversedList));
                break;
            }

            case OP_MAP:{
                int itemCounts = READ_BYTE();                
                ObjMap* map = newMap();

                for(int i = itemCounts - 1; i >= 0; i = i - 1){
                    // Check item type of key
                    if(!IS_STRING(peek_stack(2*i + 1)) && !IS_NUMBER(peek_stack(2*i + 1))){
                        frame->ip = ip;
                        runtimeError("Map keys must be string or number type.");
                        return INTERPRET_RUNTIME_ERROR;
                    }

                    Value value = peek_stack(2*i);
                    Value key = peek_stack(2*i + 1);

                    mapSet(map, key, value);
                }

                // remove elements from top of stack
                vm.stackTop -= 2 * itemCounts;

                push(OBJ_VAL(map));
                break;
            }

            case OP_INDEX:{
                int item_count = READ_BYTE();
                
                // end Index
                Value endIndex = NULL_VAL;
                if(item_count > 1){
                    endIndex = pop();
                }

                // Start index
                Value index = pop();
                Value object = pop();
                if(!handleIndexOperator(object, index, endIndex)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SET_INDEX:{
                Value result = pop();
                Value index = pop(); 
                Value object = peek_stack(0);

                if(!handleIndexSetOperator(object, index, result)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                
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
                ip = frame->ip;
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
                if(!IS_INSTANCE(peek_stack(0)) && !IS_LIST(peek_stack(0))){
                    frame->ip = ip;
                    runtimeError("Only instances have property.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Get property on Class Objects
                if(IS_INSTANCE(peek_stack(0))){
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
                // Get property on List object
                else if(IS_LIST(peek_stack(0))){
                    ObjList* list = AS_LIST(peek_stack(0));
                    ObjString* name = READ_STRING();
                    
                    Value value;
                    if(tableGet(&list->nativeMethods, name, &value)){
                        pop();
                        push(value);
                        break;
                    } else {
                        frame->ip = ip;
                        runtimeError("List method '%s' not found.", name->chars);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                }
            }

            case OP_SET_PROPERTY:{
                if(!IS_INSTANCE(peek_stack(1))){
                    frame->ip = ip;
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
                frame->ip = ip;
                if(!invoke(method, argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                ip = frame->ip;
                break;
            }

            case OP_INHERIT:{
                Value superclass = peek_stack(1);
                if(!IS_CLASS(superclass)){
                    frame->ip = ip;
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjClass* subclass = AS_CLASS(peek_stack(0));

                tableAddAll(
                    &AS_CLASS(superclass)->methods,
                    &subclass->methods
                );
                pop();
                break;
            }

            case OP_GET_SUPER:{
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());

                if(!bindMethod(superclass, name)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUPER_INVOKE:{
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop());
                
                frame->ip = ip;
                if(!invokeFromClass(superclass, method, argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }

                frame = &vm.frames[vm.frameCount - 1];
                ip = frame->ip;
                break;
            }

            case OP_DUP: {
                push(peek_stack(0));
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
    size_t count = vm.stackTop - vm.stack;
    if(count == vm.stackCapacity){
        Value *oldStack = vm.stack;

        size_t oldCapacity = vm.stackCapacity;
        vm.stackCapacity = GROW_CAPACITY(oldCapacity);
        vm.stack = GROW_ARRAY(Value, vm.stack, oldCapacity, vm.stackCapacity);
        vm.stackTop = vm.stack + count;

        // Ref: https://github.com/lazara5/elox/blob/master/elox/lib/vm.c#L296
        // the stack moved, recalculate all pointers that point to the old stack
        if(oldStack != vm.stack){
            for(int i = 0; i < vm.frameCount; i++){
                CallFrame* frame = &vm.frames[i];
                frame->slots = vm.stack + (frame->slots - oldStack);
            }

            for(ObjUpvalue* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next){
                upvalue->location = vm.stack + (upvalue->location - oldStack);
            }
        }
    }

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

Value top = NULL_VAL;

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

bool callNativeObjMethod(Value self, Value callee, int argCount){
    ObjNative* obj = AS_NATIVE_OBJ(callee);
    NativeObjFn native = obj->function.objMethod;
    if(native(argCount, self, vm.stackTop - argCount)){
        vm.stackTop -= argCount;
        return true;
    } else {
        runtimeError(AS_STRING(vm.stackTop[- argCount - 1])->chars);
        return false;
    }
    // Value result = native(
    //     argCount,
    //     self,
    //     vm.stackTop - argCount
    // );
    // vm.stackTop -= argCount + 1;
    // push(result);
    // return true;
}

bool callValue(Value callee, int argCount){
    if(IS_OBJ(callee)){
        switch (OBJ_TYPE(callee)){
            case OBJ_FUNCTION:{
                ObjClosure* closure = newClosure(AS_FUNCTION(callee));
                return callFn(closure, argCount);
            }

            case OBJ_NATIVE:{
                ObjNative* obj = AS_NATIVE_OBJ(callee);

                switch(obj->type){
                    case NATIVE_METHOD: {
                        NativeFn native = obj->function.method;
                        if(native(argCount, vm.stackTop - argCount)){
                            vm.stackTop -= argCount;
                            return true;
                        } else {
                            runtimeError(AS_STRING(vm.stackTop[- argCount - 1])->chars);
                            return false;
                        }
                        return true;
                    }

                    default: {
                        runtimeError("Invalid built-in method type.");
                        return false;
                    }

                }
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

    if(!IS_INSTANCE(receiver) && !IsNativeMethodSupported(receiver)){
        runtimeError("Only instances & native objects have methods.");
        return false;
    }

    // Instance method call
    if(IS_INSTANCE(receiver)){
        ObjInstance* instance = AS_INSTANCE(receiver);

        Value value;
        if(tableGet(&instance->fields, name, &value)){
            vm.stackTop[-argCount-1] = value;
            return callValue(value, argCount);
        }

        return invokeFromClass(instance->kclass, name, argCount);
    } 
    // List method call
    else if(IS_LIST(receiver)){
        ObjList* list = AS_LIST(receiver);
        Value method;
        if(tableGet(&list->nativeMethods, name, &method)){
            vm.stackTop[-argCount-1] = method;
            return callNativeObjMethod(receiver, method, argCount);
        } else {
            runtimeError("List method '%s' not found.", name->chars);
            return false;
        }
    }
    // File method call
    else if(IS_FILE(receiver)){
        ObjFile* file = AS_FILE(receiver);
        Value method;
        if(tableGet(&file->nativeMethods, name, &method)){
            vm.stackTop[-argCount-1] = method;
            return callNativeObjMethod(receiver, method, argCount);
        } else {
            runtimeError("File method '%s' not found.", name->chars);
            return false;
        }
    }
}

int objectLength(Value object){
    if(IS_STRING(object)){
        return AS_STRING(object)->length;
    } else if(IS_LIST(object)){
        return AS_LIST(object)->array.count;
    } else if(IS_MAP(object)){
        return AS_MAP(object)->count;
    } else if(IS_INSTANCE(object)){
        // ObjInstance* instance = AS_INSTANCE(object);
        // ObjString* method = copyString("len", 3);

        // Value result;
        // push(method);
        // if(tableGet(&instance->kclass->methods, method, &result)){
            
        // }
        // push(method);
        // invoke(method, 0);
        // if(tableGet(&klass->methods, method, &lenFunction)){

        // }
        // return val;
    }
    return 0;
}

bool arrayIndexExpression(Value object, Value index, Value endIndex){
    if(!IS_NUMBER(index) || !isInteger(AS_NUMBER(index)) || 
        ( !IS_NULL(endIndex) && !isInteger(AS_NUMBER(endIndex))) ){
        runtimeError("Index expression must be integer literal.");
        return false;
    }
        
    int length = 1;
    int position = AS_NUMBER(index);
    int end_position;

    int object_length = objectLength(object);

    if(!IS_NULL(endIndex)){
        end_position = AS_NUMBER(endIndex);
        length = end_position - position;

        // Handle Max length
        length = length < object_length ? length : object_length; 
    }

    // Start Zero-indexed
    if(position < 0 || position >= object_length){
        runtimeError("String index out of bounds.");
        return false;
    }

    // End-index Zero-indexed
    if(!IS_NULL(endIndex) && (position > end_position || end_position < 0)){
        runtimeError("String index out of bounds.");
        return false;
    }
    
    if(IS_STRING(object)){
        ObjString* string = AS_STRING(object);
        ObjString* newString = copyString(string->chars + position, length);
        push(OBJ_VAL(newString));
    } else if (IS_LIST(object)){
        ObjList* list = AS_LIST(object);

        // Case 1 : more than 1 element
        if(!IS_NULL(endIndex) && end_position - position > 1){
            ObjList* new_list = newList();
            for(int i = position; i < end_position ; i++){
                writeValueArray(&new_list->array, list->array.values[i]);
            }
            push(OBJ_VAL(new_list));
        } else {
            // Case 2: only one element
            Value val = list->array.values[position];
            push(val);
        }

    }
    return true;
}


bool mapIndexExpression(Value object, Value index, Value endIndex){
    if(!IS_NULL(endIndex)){
        runtimeError("Invalid ':' seperator for Map index expression.");
        return false;
    }
    ObjMap* map = AS_MAP(object);
    
    Value item;
    
    bool found = mapGet(map, index, &item);
    
    if(found){
        push(item);
    } else {
        runtimeError("Unable to find key in Map.");
        return false;
    }

    return true;
}


bool handleIndexOperator(Value object, Value index, Value endIndex){
    if(!IS_MAP(object) && !IS_STRING(object) && !IS_LIST(object)){
        runtimeError("Only map, list and string object support index expression.");
        return false;
    }

    // String indexing
    if(IS_STRING(object) || IS_LIST(object)){
        return arrayIndexExpression(object, index, endIndex);
    }
    // Map indexing
    else if(IS_MAP(object)){
        return mapIndexExpression(object, index, endIndex);
    }

    return true;
}

bool arraySetOperator(Value object, Value index, Value result){
    if(!IS_NUMBER(index) || !isInteger(AS_NUMBER(index))){
        runtimeError("Index must be integer literal.");
        return false; 
    }

    int position = AS_NUMBER(index);
    int object_length = objectLength(object);

    // Start Zero-indexed
    if(position < 0 || position >= object_length){
        runtimeError("String index out of bounds.");
        return false;
    }
    
    // List Object
    if(IS_LIST(object)){
        ObjList* list = AS_LIST(object);
        Value* value = &list->array.values[object_length - position - 1];
        *value = result;
    } 

    // String Object
    else if(IS_STRING(object)){
        if(!IS_STRING(result) || AS_STRING(result)->length != 1){
            runtimeError("Target must be string data type with length 1.");
            return false;
        }

        char* string = AS_CSTRING(object);
        char* target_character = AS_CSTRING(result);

        string[position] = target_character[0];
    }
    return true;
}

bool mapSetOperator(Value object, Value index, Value result){
    ObjMap* map = AS_MAP(object);
    mapSet(map, index, result);
    return true;
}

bool handleIndexSetOperator(Value object, Value index, Value result){
    if(!IS_MAP(object) && !IS_STRING(object) && !IS_LIST(object)){
        runtimeError("Only map, list and string object support setting values.");
        return false;
    }

    // String/List index set
    if(IS_LIST(object) || IS_STRING(object)){
        return arraySetOperator(object, index, result);
    }
     // Map index set
    else if(IS_MAP(object)){
        return mapSetOperator(object, index, result);
    }

    return true;
}