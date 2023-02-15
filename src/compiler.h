#ifndef viper_compiler_h
#define viper_compiler_h

#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "token.h"

#define MAX_SWITCH_CASES 256

typedef struct {
   Token current;
   Token previous;
   bool hadError;
   bool panicMode;

   Scanner* scanner;
} Parser;

ObjFunction* compile(const char* source);

void expression(Parser* parser);

void declaration(Parser* parser);
void varDeclaraction(Parser* parser);

void functionDeclaration(Parser* parser);
void function(Parser* parser, FunctionType type);

void classDeclaration(Parser* parser);

void dot(Parser* parser, bool);

void block(Parser* parser);
void beginScope();
void endScope(Parser* parser);

void statement(Parser* parser);
void returnStatement(Parser* parser);
void printStatement(Parser* parser);
void expressionStatement(Parser* parser);
void breakStatement(Parser* parser);
void continueStatement(Parser* parser);
void switchStatement(Parser* parser);

void ifStatement(Parser* parser);
void whileStatement(Parser* parser);
void forStatement(Parser* parser);

void and_(Parser* parser, bool);
void or_(Parser* parser, bool);

void synchronize(Parser* parser);

void markCompilerRoots();

#endif