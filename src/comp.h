#ifndef viper_comp_h
#define viper_comp_h

#include "common.h"
#include "object.h"
#include "token.h"

typedef struct{
    Token name;
    int depth;
    bool isCaptured;
} Local;

typedef struct 
{
    uint8_t index;
    bool isLocal;
} Upvalue;


typedef struct {
    struct Compiler* enclosing; // Used to return control from function to parent callee flow

    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;

    Upvalue upvalues[UINT8_COUNT];
} Compiler;

#endif