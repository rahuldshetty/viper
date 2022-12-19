#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "token.h"

typedef struct {
   Token current;
   Token previous;
   bool hadError;
   bool panicMode;
} Parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

void expression();
ParseRule* getRule(TokenType type);
void parsePrecedence(Precedence precedence);
void expression();
void number_constant();
void unary();
void grouping();
void binary();

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_ADD]           = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DIVIDE]        = {NULL,     binary, PREC_FACTOR},
  [TOKEN_MULTIPLY]      = {NULL,     binary, PREC_FACTOR},
  [TOKEN_NOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NOT_EQUAL]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number_constant,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NULL]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

Parser parser;
Chunk* complingChunk;

Chunk* currentChunk(){
    return complingChunk;
}

void errorAt(Token* token, const char* message){
    if(parser.panicMode) return;

    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if(token->type == TOKEN_EOF){
        fprintf(stderr, " at end");
    } else if(token->type == TOKEN_ERROR){
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

void error(const char* message){
    errorAt(&parser.previous, message);
}

void errorAtCurrent(const char* message){
    errorAt(&parser.current, message);
}

void advance_parser(){
    parser.previous = parser.current;

    for(;;){
        parser.current = scanToken();

        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }

}

void consume(TokenType type, const char* message){
    if(parser.current.type == type){
        advance_parser();
        return;
    }

    errorAtCurrent(message);
}

void emitByte(uint8_t byte){
    writeChunk(currentChunk(), byte, parser.previous.line);
}

void emitBytes(uint8_t byte1, uint8_t byte2){
    emitByte(byte1);
    emitByte(byte2);
}

void emitReturn(){
    emitByte(OP_RETURN);
}

// handle number values
uint8_t makeConstant(Value value){
    int constant = addConstant(currentChunk(), value);

    if(constant > UINT8_MAX){
        error("Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t) constant;
}

void emitConstant(Value value){
    emitBytes(OP_CONSTANT, makeConstant(value));
}

void endCompiler(){
    emitReturn();
}

void parsePrecedence(Precedence precedence) {

}

ParseRule* getRule(TokenType type){
    return &rules[type];
}

// Handle binary arithmetic expression
void binary(){
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
        case TOKEN_ADD:                     emitByte(OP_ADD); break;
        case TOKEN_MINUS:                   emitByte(OP_MINUS); break;
        case TOKEN_MULTIPLY:                emitByte(OP_MULTIPLY); break;
        case TOKEN_DIVIDE:                  emitByte(OP_DIVIDE); break;
        default: return;
    }
}

// List of handler for tokens
void number_constant(){
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}

void unary(){
    TokenType operatorType = parser.previous.type;

    // compile the operand
    parsePrecedence(PREC_UNARY);

    switch (operatorType)
    {
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }
}

void grouping(){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected ')' after expressionl.");
}

void expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const char* source, Chunk* chunk){
    initScanner(source);
    complingChunk = chunk;
    
    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expected end of expression.");

    endCompiler();

    return !parser.hadError;
}