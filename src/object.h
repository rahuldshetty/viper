#ifndef viper_object_h
#define viper_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) (((ObjString*)AS_OBJ(value)))
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
} ObjType;

struct Obj {
    ObjType type;
    struct Obj* next;
}; 

struct ObjString{
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

ObjString* copyString(const char* chars, int length);
void printObject(Value value);
bool isObjType(Value value, ObjType type);
ObjString* takeString(char* chars, int length);

#endif