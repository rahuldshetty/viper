#ifndef viper_compiler_h
#define viper_compiler_h

#include "chunk.h"
#include "object.h"

#define MAX_SWITCH_CASES 256

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
void breakStatement();
void continueStatement();
void switchStatement();

void ifStatement();
void whileStatement();
void forStatement();

void and_(bool);
void or_(bool);

void synchronize();

void markCompilerRoots();

#endif