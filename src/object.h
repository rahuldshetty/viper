#ifndef viper_object_h
#define viper_object_h

#include <stdio.h>
#include <stdbool.h>

#include "common.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define IS_NATIVE(value) isObjType(value, OBJ_NATIVE)
#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define IS_CLASS(value) isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_LIST(value) isObjType(value, OBJ_LIST)
#define IS_MAP(value) isObjType(value, OBJ_MAP)
#define IS_FILE(value) isObjType(value, OBJ_FILE)

#define AS_STRING(value) (((ObjString*)AS_OBJ(value)))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value) ((ObjFunction*)AS_OBJ(value))
#define AS_CLOSURE(value) ((ObjClosure*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)
#define AS_NATIVE_OBJ(value) ((ObjNative*)AS_OBJ(value))
#define AS_CLASS(value) ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_LIST(value) ((ObjList*)AS_OBJ(value))
#define AS_MAP(value) ((ObjMap*)AS_OBJ(value))
#define AS_FILE(value) ((ObjFile*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD,
    OBJ_LIST,
    OBJ_MAP,
    OBJ_FILE,
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
    Table methods;
} ObjClass;

typedef struct{
    Obj obj;
    ObjClass* kclass;
    Table fields;
} ObjInstance;

typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

typedef struct{
    Obj obj;
    ValueArray array;
    Table nativeMethods;
} ObjList;

typedef struct {
    Value key;
    Value value;
} MapEntry;

typedef struct{
    Obj obj;
    int capacity;
    int count;
    MapEntry* entries;
} ObjMap;

typedef struct {
    Obj obj;
    bool isOpen;
    FILE *file;
    ObjString* mode;
    ObjString* path;
    Table nativeMethods;
} ObjFile;

typedef bool (*NativeFn)(int argCount, Value* args);
typedef bool (*NativeObjFn)(int argCount, Value obj, Value* args);

typedef enum {
    NATIVE_METHOD,     // called directly - len, str
    NATIVE_OBJ_METHOD, // part of list/map/etc
} NativeType;

typedef struct{
    Obj obj;
    NativeType type;
    union
    {
        NativeObjFn objMethod;
        NativeFn method;
    } function;
} ObjNative;

ObjString* copyString(const char* chars, int length);
void printObject(Value value);
bool isObjType(Value value, ObjType type);
ObjString* takeString(char* chars, int length);
ObjString* allocateString(char* chars, int length, uint32_t hash);

ObjFunction* newFunction();
ObjClosure* newClosure(ObjFunction* function);
ObjNative* newNative(NativeFn function);
ObjNative* newObjNative(NativeObjFn function);

ObjUpvalue* newObjUpvalue(Value* slot);

ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* klass);
ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjList* newList();
ObjMap* newMap();

ObjFile* newFile(ObjString* path, ObjString* mode);

ObjString* strObject(Value obj);

#endif