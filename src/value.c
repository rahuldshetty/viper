#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"

void initValueArray(ValueArray* array){
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

void writeValueArray(ValueArray* array, Value value){
    if (array->capacity < array->count + 1){
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}


void freeValueArray(ValueArray* array){
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value){
#ifdef NAN_BOXING
    if(IS_BOOL(value)){
        printf(AS_BOOL(value) ? "true":"false");
    } else if(IS_NULL(value)){
        printf("null");
    } else if(IS_NUMBER(value)){
        printf("%g", AS_NUMBER(value));
    } else if(IS_OBJ(value)){
        printObject(value);
    }
#else
    switch(value.type){
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true":"false");
            break;

        case VAL_NULL: printf("null"); break;
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
        case VAL_OBJ: printObject(value); break;
    }
#endif
}

bool valuesEqual(Value a, Value b){
#ifdef NAN_BOXING
    if(IS_NUMBER(a) && IS_NUMBER(b)){
        return AS_NUMBER(a) == AS_NUMBER(b);
    }
    return a == b;
#else
    if(a.type != b.type) return false;
    switch(a.type){
        case VAL_BOOL: return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NULL: return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ: return AS_OBJ(a) == AS_OBJ(b);

        default: return false;
    }
#endif
}

// Hack for Hash function
uint32_t hashBits(double hash)
{
    return (int) hash;
}

uint32_t hashObject(Obj* obj){
    switch(obj->type){
        case OBJ_STRING:
            return ((ObjString*) obj)->hash;

        default:
            return 0;
    }
}

uint32_t hashNumber(double num){
    return hashBits(num);
}

uint32_t hashValue(Value value){
#ifdef NAN_BOXING
    if(IS_OBJ(value)){
        return hashObject(AS_OBJ(value));
    } else if(IS_NUMBER(value)){
        return hashBits(AS_NUMBER(value));
    }

    return 0;

#else 
    switch (value.type)
    {
        case VAL_BOOL:  return (uint32_t) AS_BOOL(value);
        case VAL_NULL:  return 2;
        case VAL_NUMBER:   return hashNumber(AS_NUMBER(value));
        case VAL_OBJ:   return hashObject(AS_OBJ(value));
    }
    
    return 0;
#endif
}

bool isInteger(double num){
    return (num - (int)num) == 0;
}

ObjString* strValue(Value obj){
    // String
    if(IS_STRING(obj)){
        return AS_STRING(obj);
    
    // Number
    } else if(IS_NUMBER(obj)){
        double number = AS_NUMBER(obj);
        int len = snprintf(NULL, 0, "%g", number);
        
        char *output = ALLOCATE(char, len+1);
        
        snprintf(output, len + 1, "%g", number);
        
        ObjString* string = copyString(output, len);
        
        FREE(char, output);
        
        return string;
    }

    // Bool Values
    else if(IS_BOOL(obj)){
        // True
        if(AS_BOOL(obj)){
            return copyString("true", 4);
        }
        // False 
        else {
            return copyString("false", 5);
        }

    }

    // NULL Value
    else if(IS_NULL(obj)){
        return copyString("null", 4);
    }

    // Object Value
    else if(IS_OBJ(obj)){
        return strObject(obj);
    }
}