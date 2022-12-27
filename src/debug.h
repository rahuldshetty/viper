#ifndef viper_debug_h
#define viper_debug_h

#include "chunk.h"
#include "object.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

int simpleInstruction(const char* name, int offset);
int byteInstruction(const char* name, Chunk* chunk, int offset);
int constantInstruction(const char* name, Chunk* chunk, int offset);
int jumpInstruction(const char* name, int sign, Chunk* chunk, int offset);

#endif