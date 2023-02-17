#ifndef viper_compiler_h
#define viper_compiler_h

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

#include "common.h"
#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "token.h"
#include "value.h"
#include "vm.h"

#define MAX_SWITCH_CASES 256

typedef struct {
   Token current;
   Token previous;
   bool hadError;
   bool panicMode;

   VM* vm;

   Scanner* scanner;
} Parser;


typedef struct{
    struct ClassCompiler* enclosing;
    bool hasSuperclass;
} ClassCompiler;

// TODO: Ternary Operator

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_TERNARY,     // ?:
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_INDEX,       // array[INDEX], map[INDEX]
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser* parser, bool canAssign);

typedef struct{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

ObjFunction* compile(VM* vm, const char* source);

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
void importStatement(Parser* parser);

void ifStatement(Parser* parser);
void whileStatement(Parser* parser);
void forStatement(Parser* parser);

void and_(Parser* parser, bool);
void or_(Parser* parser, bool);

void synchronize(Parser* parser);

void markCompilerRoots();

#endif