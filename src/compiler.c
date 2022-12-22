#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct{
    Token name;
    int depth;
} Local;

typedef struct {
    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;

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

typedef void (*ParseFn)(bool canAssign);

typedef struct{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

void expression();
ParseRule* getRule(TokenType type);
void parsePrecedence(Precedence precedence);
void expression();
void number_constant(bool canAssign);
void unary(bool canAssign);
void grouping(bool canAssign);
void binary(bool canAssign);
void variable(bool canAssign);
void literal(bool canAssign);
void string_constant(bool canAssign);

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
  [TOKEN_IDENTIFIER]    = {variable,     NULL,   PREC_NONE},
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
Compiler* current = NULL; // TODO: multiple compilers running in parallel
Chunk* complingChunk;

Chunk* currentChunk(){
    return complingChunk;
}

void initCompiler(Compiler* compiler){
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    current = compiler;
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

int emitJump(uint8_t instruction){
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
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

void patchJump(int offset){
    // -2 to adjust for bytecode of jump offset itself
    int jump = currentChunk()->count - offset - 2;

    if(jump > UINT16_MAX){
        error("Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;

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

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while(precedence <= getRule(parser.current.type)->precedence){
        advance_parser();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if(canAssign && match_parser(TOKEN_EQUAL)){
        error("Invalid assignment target.");
    }

}

uint8_t identifierConstant(Token* name){
    return makeConstant(
        OBJ_VAL(
            copyString(
                name->start,
                name->length
            )
        )
    );
}

bool identifiersEqual(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

int resolveLocal(Compiler* compiler, Token* name){
    for(int i = compiler->localCount-1; i >= 0 ; i--){
        Local* local = &compiler->locals[i];
        if(identifiersEqual(name, &local->name)){
            if(local->depth == -1){
                error("Can't read local variables in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

void addLocal(Token name){
    // Reach max limit for the local variable array
    if(current->localCount == UINT8_COUNT){
        error("Too many local variables in block.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

// Variable added to scope but not ready to use yet.
void declareVariable(){
    if(current->scopeDepth == 0) return;

    Token* name = &parser.previous;

    // Uncomment if you need to check for variable name already exists error
    /*
    for(int i = current->localCount - 1; i >=0;i--){
        Local* local = &current->locals[i];

        if(local->depth != -1 && local->depth < current->scopeDepth){
            break;
        }

        if(identifiersEqual(name, &local->name)){
            error("Already a variable with this name in this scope.");
        }

    }
    */

    // Assign name to temporary location in constant array to be named variable
    addLocal(*name);
}

uint8_t parseVariable(const char* errorMessage){
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();

    // Exit scope if we're in local scope 
    // at runtime, locals are not looked up
    if(current->scopeDepth > 0) return 0;

    return identifierConstant(&parser.previous);
}

void markInitialized(){
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

ParseRule* getRule(TokenType type){
    return &rules[type];
}

// Handle binary arithmetic expression
void binary(bool canAssign){
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

void literal(bool canAssign){
    switch (parser.previous.type) {
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        case TOKEN_NULL: emitByte(OP_NULL); break;
        
        default: return;
    }
}

// List of handler for tokens
void number_constant(bool canAssign){
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

void string_constant(bool canAssign){
    emitConstant(
        OBJ_VAL(
            copyString(parser.previous.start + 1, parser.previous.length - 2)
        )
    );
}

void unary(bool canAssign){
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

void grouping(bool canAssign){
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expected ')' after expressionl.");
}

void expression(){
    parsePrecedence(PREC_ASSIGNMENT);
}

void declaration(){
    if(match_parser(TOKEN_VAR)){
        varDeclaraction();
    } else {
        statement();
    }

    if(parser.panicMode) synchronize();
}

// Variable ready to use.
void defineVariable(uint8_t global){
    // local variable not processed until runtime
    if(current->scopeDepth > 0){
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

// Variable Declaraction
void varDeclaraction(){
    uint8_t global = parseVariable("Expected variable name.");

    if(match_parser(TOKEN_EQUAL)){
        expression();
    } else {
        emitByte(OP_NULL);
    }

    match_parser(TOKEN_SEMICOLON);
    defineVariable(global);
}


// Identifer named vairiable access
void namedVariable(Token name, bool canAssign){
    uint8_t getOp, setOp;

    int arg = resolveLocal(current, &name);
    
    if(arg != -1){
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    
    if(canAssign && match_parser(TOKEN_EQUAL)){
        expression();
        emitBytes(setOp, (uint8_t) arg);
    } else {
        emitBytes(getOp, (uint8_t) arg);
    }
}


// Variable access
void variable(bool canAssign){
    namedVariable(parser.previous, canAssign);
}

// Block
void block(){
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)){
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}



void statement(){
    if(match_parser(TOKEN_PRINT)){
        // Parse Print keyword statement
        printStatement();
    } else if(match_parser(TOKEN_IF)){
        ifStatement();
    } else if(match_parser(TOKEN_LEFT_BRACE)){
        // Parse Block statements
        beginScope();
        block();
        endScope();
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

    Compiler compiler;
    initCompiler(&compiler);

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

void beginScope(){
    current->scopeDepth++;
}

void endScope(){
    current->scopeDepth--;

    // TODO: Optimization by OP_POPN if sequence of stack items to be removed
    while(current->localCount > 0 && 
        current->locals[current->localCount - 1].depth > current->scopeDepth
    ){
        emitByte(OP_POP);
        current->localCount--;
    }

}

// If statement control flow
void ifStatement(){
    // encapsulating "(" ")" is optional
    bool paranFound = match_parser(TOKEN_LEFT_PAREN);
    
    expression();

    if(paranFound){
        consume(TOKEN_RIGHT_PAREN, "Expected ')' for closing condition.");
    }

    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    statement();

    patchJump(thenJump);
}