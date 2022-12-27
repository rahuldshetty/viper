#ifndef viper_object_h
#define viper_object_h

#include "common.h"
#include "chunk.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)

#define AS_STRING(value) (((ObjString*)AS_OBJ(value)))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_CLASS,
} ObjType;

struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj* next;
}; 

struct ObjString{
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

typedef struct ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    int arity;
    Chunk chunk;
    ObjString* name;
    int upvalueCount;
} ObjFunction;

typedef struct{
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct{
    Obj obj;
    ObjString* name;
} ObjClass;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct{
    Obj obj;
    NativeFn function;
} ObjNative;

ObjString* copyString(const char* chars, int length);
void printObject(Value value);
bool isObjType(Value value, ObjType type);
ObjString* takeString(char* chars, int length);

ObjFunction* newFunction();
ObjClosure* newClosure(ObjFunction* function);
ObjNative* newNative(NativeFn function);

ObjUpvalue* newObjUpvalue(Value* slot);

ObjClass* newClass(ObjString* name);

#endif