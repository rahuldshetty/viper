#ifndef viper_compiler_h
#define viper_compiler_h

#include "chunk.h"
#include "object.h"

ObjFunction* compile(const char* source);

void expression();

void declaration();
void varDeclaraction();

void functionDeclaration();
void function();

void classDeclaration();

void dot(bool);

void block();
void beginScope();
void endScope();

void statement();
void returnStatement();
void printStatement();
void expressionStatement();

void ifStatement();
void whileStatement();
void forStatement();

void and_(bool);
void or_(bool);

void synchronize();

void markCompilerRoots();

#endif