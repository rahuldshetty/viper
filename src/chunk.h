#ifndef viper_chunk_h
#define viper_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT_LONG,
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN,
} OpCode;


typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines; // TODO: Run-length encoding
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

// TODO
void writeConstant(Chunk* chunk, Value value, int line);

#endif