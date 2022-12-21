#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "token.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
   Token current;
   Token previous;
   bool hadError;
   bool panicMode;
} Parser;

// TODO: Ternary Operator

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
void literal();
void string_constant();

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
  [TOKEN_NOT]           = {unary,    NULL,   PREC_NONE},
  [TOKEN_NOT_EQUAL]     = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string_constant,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number_constant,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NULL]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
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

bool check(TokenType type){
    return parser.current.type == type;
}

bool match_parser(TokenType type){
    if(!check(type)) return false;
    advance_parser();
    return true;
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
#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError){
        disassembleChunk(currentChunk(), "code");
    }
#endif

}

void parsePrecedence(Precedence precedence) {
    advance_parser();
    ParseFn prefixRule = getRule(parser.previous.type) -> prefix;
    if(prefixRule == NULL){
        error("Expected expression.");
        return;
    }

    prefixRule();

    while(precedence <= getRule(parser.current.type)->precedence){
        advance_parser();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }

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
        case TOKEN_NOT_EQUAL:               emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:             emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:                 emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL:           emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:                    emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:              emitBytes(OP_GREATER, OP_NOT); break;

        case TOKEN_ADD:                     emitByte(OP_ADD); break;
        case TOKEN_MINUS:                   emitByte(OP_MINUS); break;
        case TOKEN_MULTIPLY:                emitByte(OP_MULTIPLY); break;
        case TOKEN_DIVIDE:                  emitByte(OP_DIVIDE); break;
        default: return;
    }
}

void literal(){
    switch (parser.previous.type) {
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        case TOKEN_NULL: emitByte(OP_NULL); break;
        
        default: return;
    }
}

// List of handler for tokens
void number_constant(){
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

void string_constant(){
    emitConstant(
        OBJ_VAL(
            copyString(parser.previous.start + 1, parser.previous.length - 2)
        )
    );
}

void unary(){
    TokenType operatorType = parser.previous.type;

    // compile the operand
    parsePrecedence(PREC_UNARY);

    switch (operatorType)
    {
        case TOKEN_NOT: emitByte(OP_NOT); break;
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

void declaration(){
    statement();

    if(parser.panicMode) synchronize();
}

void statement(){
    if(match_parser(TOKEN_PRINT)){
        printStatement();
    } else {
        expressionStatement();
    }
}

void printStatement(){
    expression();
    // Semi-colon optional at end of print statement
    match_parser(TOKEN_SEMICOLON);
    emitByte(OP_PRINT);
}

void expressionStatement(){
    expression();
    match_parser(TOKEN_SEMICOLON);
    emitByte(OP_POP);
}

bool compile(const char* source, Chunk* chunk){
    initScanner(source);
    complingChunk = chunk;
    
    parser.hadError = false;
    parser.panicMode = false;

    advance_parser();

    while(!match_parser(TOKEN_EOF)){
        declaration();
    }

    endCompiler();

    return !parser.hadError;
}

void synchronize(){
    parser.panicMode = false;

    while(parser.current.type != TOKEN_EOF){
        if(parser.previous.type == TOKEN_SEMICOLON) return;

        switch (parser.current.type)
        {
            case TOKEN_CLASS:
            case TOKEN_FUNCTION:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default: ;
        }

        advance_parser();
    }
}