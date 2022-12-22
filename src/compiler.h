#ifndef viper_compiler_h
#define viper_compiler_h

#include "chunk.h"
#include "object.h"

bool compile(const char* source, Chunk* chunk);

void expression();

void declaration();
void varDeclaraction();

void block();
void beginScope();
void endScope();

void statement();
void printStatement();
void expressionStatement();

void ifStatement();

void synchronize();

#endif