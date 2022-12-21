#ifndef viper_compiler_h
#define viper_compiler_h

#include "chunk.h"
#include "object.h"

bool compile(const char* source, Chunk* chunk);

void expression();

void declaration();
void varDeclaraction();

void statement();
void printStatement();
void expressionStatement();

void synchronize();

#endif