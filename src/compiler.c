#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

ParseRule* getRule(Parser* parser, TokenType type);
void parsePrecedence(Parser* parser, Precedence precedence);
void expression(Parser* parser);
void number_constant(Parser* parser, bool canAssign);
void unary(Parser* parser, bool canAssign);
void grouping(Parser* parser, bool canAssign);
void binary(Parser* parser, bool canAssign);
void ternary(Parser* parser, bool);
void variable(Parser* parser, bool canAssign);
void literal(Parser* parser, bool canAssign);
void list_literal(Parser* parser, bool);
void map_literal(Parser* parser, bool);
void string_constant(Parser* parser, bool canAssign);
void call(Parser* parser, bool);
void namedVariable(Parser* parser, Token name, bool canAssign);
uint8_t argumentList(Parser* parser, TokenType);
void this_(Parser* parser, bool);
void super_(Parser* parser, bool);
void index_expr(Parser* parser, bool);

ParseRule rules[] = {
  [TOKEN_LEFT_BRACKET]  = {list_literal,     index_expr,   PREC_INDEX},
  [TOKEN_RIGHT_BRACKET] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_PAREN]    = {grouping, call,   PREC_CALL},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {map_literal,     NULL,   PREC_NONE}, 
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     dot,    PREC_CALL},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_ADD]           = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_QUESTION]      = {NULL,     ternary,   PREC_TERNARY},
  [TOKEN_DIVIDE]        = {NULL,     binary, PREC_FACTOR},
  [TOKEN_MULTIPLY]      = {NULL,     binary, PREC_FACTOR},
  [TOKEN_MOD]           = {NULL,     binary, PREC_FACTOR},
  [TOKEN_NOT]           = {unary,    NULL,   PREC_NONE},
  [TOKEN_NOT_EQUAL]     = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,     PREC_NONE},
  [TOKEN_ADD_EQUAL]     = {NULL,     binary,   PREC_NONE},
  [TOKEN_MINUS_EQUAL]   = {NULL,     binary,   PREC_NONE},
  [TOKEN_MULTIPLY_EQUAL]= {NULL,     binary,   PREC_NONE},
  [TOKEN_DIVIDE_EQUAL]  = {NULL,     binary,   PREC_NONE},
  [TOKEN_MOD_EQUAL]     = {NULL,     binary,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary,   PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary,   PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {variable, NULL,     PREC_NONE},
  [TOKEN_STRING]        = {string_constant,     NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number_constant,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     and_,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUNCTION]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NULL]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     or_,    PREC_OR},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {super_,   NULL,   PREC_NONE},
  [TOKEN_THIS]          = {this_,    NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BREAK]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CONTINUE]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_AS]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IMPORT]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

Compiler* current = NULL; // TODO: multiple compilers running in parallel
ClassCompiler* currentClass = NULL;

Chunk* currentChunk(){
    return &(current->function->chunk);
}

void initCompiler(Parser* parser, Compiler* compiler, FunctionType type){
    compiler->enclosing = (struct Compiler*) current;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;
    
    // Copy function name
    if(type != TYPE_SCRIPT){
        current->function->name = copyString(
            parser->previous.start,
            parser->previous.length
        );
    }

    // Compiler claims variable stack's 0th location
    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;

    // Store this keyword for class object
    if(type != TYPE_FUNCTION){
        local->name.start = "this";
        local->name.length = 4;
    } else{
        local->name.start = "";
        local->name.length = 0;
    }
}

void errorAt(Parser* parser, Token* token, const char* message){
    if(parser->panicMode) return;

    parser->panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if(token->type == TOKEN_EOF){
        fprintf(stderr, " at end");
    } else if(token->type == TOKEN_ERROR){
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}

void error(Parser* parser,const char* message){
    errorAt(parser, &parser->previous, message);
}

void errorAtCurrent(Parser* parser, const char* message){
    errorAt(parser, &parser->current, message);
}

void advance_parser(Parser* parser){
    parser->previous = parser->current;

    for(;;){
        parser->current = scanToken(parser->scanner);

        if(parser->current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser, parser->current.start);
    }

}

void consume(Parser* parser, TokenType type, const char* message){
    if(parser->current.type == type){
        advance_parser(parser);
        return;
    }

    errorAtCurrent(parser, message);
}

bool check(Parser* parser, TokenType type){
    return parser->current.type == type;
}

bool match_parser(Parser* parser, TokenType type){
    if(!check(parser, type)) return false;
    advance_parser(parser);
    return true;
}

void emitByte(Parser* parser, uint8_t byte){
    writeChunk(currentChunk(), byte, parser->previous.line);
}

void emitBytes(Parser* parser, uint8_t byte1, uint8_t byte2){
    emitByte(parser, byte1);
    emitByte(parser, byte2);
}

int emitJump(Parser* parser, uint8_t instruction){
    emitByte(parser, instruction);
    emitByte(parser, 0xff);
    emitByte(parser, 0xff);
    return currentChunk()->count - 2;
}

void emitLoop(Parser* parser, int loopStart){
    emitByte(parser, OP_LOOP);

    int offset = currentChunk()->count - loopStart + 2;
    if(offset > UINT16_MAX) error(parser, "Too large loop body.");

    emitByte(parser, (offset >> 8) & 0xff);
    emitByte(parser, offset & 0xff);
}

void emitReturn(Parser* parser){
    if(current->type == TYPE_INITIALIZER){
        emitBytes(parser, OP_GET_LOCAL, 0);
    } else{
        emitByte(parser, OP_NULL); // return NULL by default from function calls
    }
    
    emitByte(parser, OP_RETURN);
}

// handle number values
uint8_t makeConstant(Parser* parser, Value value){
    int constant = addConstant(currentChunk(), value);

    if(constant > UINT8_MAX){
        error(parser, "Too many constants in one chunk.");
        return 0;
    }

    return (uint8_t) constant;
}

void emitConstant(Parser* parser, Value value){
    // Support emitting large constant values
    writeConstant(currentChunk(), value, parser->previous.line);
}

void patchJump(Parser* parser, int offset){
    // -2 to adjust for bytecode of jump offset itself
    int jump = currentChunk()->count - offset - 2;

    if(jump > UINT16_MAX){
        error(parser, "Too much code to jump over.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;

}

ObjFunction* endCompiler(Parser* parser){
    emitReturn(parser);
    ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
    if(!parser.hadError){
        disassembleChunk(currentChunk(), function->name != NULL ? 
            function->name->chars: "<script>");
    }
#endif

    current = (Compiler*) current->enclosing;
    return function;
}

void parsePrecedence(Parser* parser, Precedence precedence) {
    advance_parser(parser);
    ParseFn prefixRule = getRule(parser, parser->previous.type) -> prefix;
    if(prefixRule == NULL){
        error(parser, "Expected expression.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(parser, canAssign);

    while(precedence <= getRule(parser, parser->current.type)->precedence){
        advance_parser(parser);
        ParseFn infixRule = getRule(parser, parser->previous.type)->infix;
        infixRule(parser, canAssign);
    }

    if(canAssign && match_parser(parser, TOKEN_EQUAL)){
        error(parser, "Invalid assignment target.");
    }

}

uint8_t identifierConstant(Parser* parser, Token* name){
    return makeConstant(
        parser, 
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

int resolveLocal(Parser* parser, Compiler* compiler, Token* name){
    for(int i = compiler->localCount-1; i >= 0 ; i--){
        Local* local = &compiler->locals[i];
        if(identifiersEqual(name, &local->name)){
            if(local->depth == -1){
                error(parser, "Can't read local variables in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

int addUpValue(Parser* parser, Compiler* compiler, uint8_t index, bool isLocal){
    int upvalueCount = compiler->function->upvalueCount;

    for(int i = 0; i < upvalueCount; i++){
        Upvalue* upvalue = &compiler->upvalues[i];
        if(upvalue->index == index && upvalue->isLocal == isLocal){
            return i;
        }
    }

    if(upvalueCount == UINT8_COUNT){
        error(parser, "Too many closure variables in function.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

int resolveUpValue(Parser* parser, Compiler* compiler, Token* name){
    if(compiler->enclosing == NULL) return -1;

    int local = resolveLocal(parser, ((Compiler*)compiler->enclosing), name);
    if(local != -1){
        ((Compiler*)(compiler->enclosing))->locals[local].isCaptured = true;
        return addUpValue(parser, compiler, (uint8_t)local, true);
    }

    int upvalue = resolveUpValue(parser, (Compiler *) compiler->enclosing, name);
    if(upvalue != -1){
        return addUpValue(parser, compiler, (uint8_t) upvalue, false);
    }

    return -1;
}

void addLocal(Parser* parser, Token name){
    // Reach max limit for the local variable array
    if(current->localCount == UINT8_COUNT){
        error(parser, "Too many local variables in block.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
}

// Variable added to scope but not ready to use yet.
void declareVariable(Parser* parser){
    if(current->scopeDepth == 0) return;

    Token* name = &parser->previous;

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
    addLocal(parser, *name);
}

uint8_t parseVariable(Parser* parser, const char* errorMessage){
    consume(parser, TOKEN_IDENTIFIER, errorMessage);

    declareVariable(parser);

    // Exit scope if we're in local scope 
    // at runtime, locals are not looked up
    if(current->scopeDepth > 0) return 0;

    return identifierConstant(parser, &parser->previous);
}

void markInitialized(){
    if(current->scopeDepth == 0) return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

ParseRule* getRule(Parser* parser, TokenType type){
    return &rules[type];
}

// Handle binary arithmetic expression
void binary(Parser* parser, bool canAssign){
    TokenType operatorType = parser->previous.type;
    ParseRule* rule = getRule(parser, operatorType);
    parsePrecedence(parser, (Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
        case TOKEN_NOT_EQUAL:               emitBytes(parser, OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:             emitByte(parser, OP_EQUAL); break;
        case TOKEN_GREATER:                 emitByte(parser, OP_GREATER); break;
        case TOKEN_GREATER_EQUAL:           emitBytes(parser, OP_LESS, OP_NOT); break;
        case TOKEN_LESS:                    emitByte(parser, OP_LESS); break;
        case TOKEN_LESS_EQUAL:              emitBytes(parser, OP_GREATER, OP_NOT); break;

        case TOKEN_ADD:                     emitByte(parser, OP_ADD); break;
        case TOKEN_MINUS:                   emitByte(parser, OP_MINUS); break;
        case TOKEN_MULTIPLY:                emitByte(parser, OP_MULTIPLY); break;
        case TOKEN_DIVIDE:                  emitByte(parser, OP_DIVIDE); break;
        case TOKEN_MOD:                     emitByte(parser, OP_MOD); break;
        default: return;
    }
}

void literal(Parser* parser, bool canAssign){
    switch (parser->previous.type) {
        case TOKEN_FALSE: emitByte(parser, OP_FALSE); break;
        case TOKEN_TRUE: emitByte(parser, OP_TRUE); break;
        case TOKEN_NULL: emitByte(parser, OP_NULL); break;
        
        default: return;
    }
}

// List Literal
void list_literal(Parser* parser, bool canAssign){
    uint8_t itemCount = argumentList(parser, TOKEN_RIGHT_BRACKET);
    emitBytes(parser, OP_LIST, itemCount);
}

// Map/Hash Object
void map_literal(Parser* parser, bool canAssign){
    uint8_t itemCount = 0;
    do{
        if(check(parser, TOKEN_RIGHT_BRACE)){
            break;
        }

        expression(parser);
        
        consume(parser, TOKEN_COLON, "Expected ':' after Map key.");

        expression(parser);

        itemCount++;

    } while(match_parser(parser, TOKEN_COMMA));
    
    emitBytes(parser, OP_MAP, itemCount);
    
    consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after Map body.");
    match_parser(parser, TOKEN_SEMICOLON);
}

// List of handler for tokens
void number_constant(Parser* parser, bool canAssign){
    double value = strtod(parser->previous.start, NULL);
    emitConstant(parser, NUMBER_VAL(value));
}

void string_constant(Parser* parser, bool canAssign){
    emitConstant(
        parser, 
        OBJ_VAL(
            copyString(parser->previous.start + 1, parser->previous.length - 2)
        )
    );
}

void ternary(Parser* parser, bool canAssign){
    // Similar to if-then/else statement but instead of executing statement, we look for expression values
    int thenJump = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByte(parser, OP_POP);
    expression(parser);

    int elseJump = emitJump(parser, OP_JUMP);

    patchJump(parser, thenJump);
    emitByte(parser, OP_POP); // Discared condition values

    consume(parser, TOKEN_COLON, "Expected ':' separator.");

    expression(parser);

    patchJump(parser, elseJump);

    match_parser(parser, TOKEN_SEMICOLON);
}

void unary(Parser* parser, bool canAssign){
    TokenType operatorType = parser->previous.type;

    // compile the operand
    parsePrecedence(parser, PREC_UNARY);

    switch (operatorType)
    {
        case TOKEN_NOT: emitByte(parser, OP_NOT); break;
        case TOKEN_MINUS: emitByte(parser, OP_NEGATE); break;
        default: return;
    }
}

void grouping(Parser* parser, bool canAssign){
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after expressionl.");
}

void expression(Parser* parser){
    parsePrecedence(parser, PREC_ASSIGNMENT);
}

void declaration(Parser* parser){
    if(match_parser(parser, TOKEN_CLASS)){
        classDeclaration(parser);
    }
    else if(match_parser(parser, TOKEN_FUNCTION)){
        functionDeclaration(parser);
    } else if(match_parser(parser, TOKEN_VAR)){
        varDeclaraction(parser);
    } else {
        statement(parser);
    }

    if(parser->panicMode) synchronize(parser);
}


// Variable ready to use.
void defineVariable(Parser* parser, uint8_t global){
    // local variable not processed until runtime
    if(current->scopeDepth > 0){
        markInitialized();
        return;
    }

    emitBytes(parser, OP_DEFINE_GLOBAL, global);
}

// AND statement
void and_(Parser* parser, bool canAssign){
    int endJump = emitJump(parser, OP_JUMP_IF_FALSE);

    emitByte(parser, OP_POP);
    parsePrecedence(parser, PREC_AND);

    patchJump(parser, endJump);
}

void or_(Parser* parser, bool canAssign){
    int elseJump = emitJump(parser, OP_JUMP_IF_FALSE);
    int endJump = emitJump(parser, OP_JUMP);

    patchJump(parser, elseJump);
    emitByte(parser, OP_POP);

    parsePrecedence(parser, PREC_OR);
    patchJump(parser, endJump);
}

// Variable Declaraction
void varDeclaraction(Parser* parser){
    uint8_t global = parseVariable(parser, "Expected variable name.");

    if(match_parser(parser, TOKEN_EQUAL)){
        expression(parser);
    } else {
        emitByte(parser, OP_NULL);
    }

    match_parser(parser, TOKEN_SEMICOLON);
    defineVariable(parser, global);
}

// Function Declaration
void functionDeclaration(Parser* parser){
    uint8_t global = parseVariable(parser, "Expected function name.");
    markInitialized();
    function(parser, TYPE_FUNCTION);
    defineVariable(parser, global);
}


void function(Parser* parser, FunctionType type){
    Compiler compiler;
    initCompiler(parser, &compiler, type);
    beginScope();

    consume(parser, TOKEN_LEFT_PAREN, "Expected '(' after function name.");

    // Parameter processing
    if(!check(parser, TOKEN_RIGHT_PAREN)){
        do{
            current->function->arity++;
            if(current->function->arity > 255){
                errorAtCurrent(parser, "Can't have more than 255 parameters.");
            }
            uint8_t constant = parseVariable(parser, "Expected parameter name");
            defineVariable(parser, constant);
        } while(match_parser(parser, TOKEN_COMMA));
    }

    consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after parameters.");
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before function body.");
    block(parser);

    ObjFunction* function = endCompiler(parser);
    emitBytes(parser, OP_CLOSURE, makeConstant(parser, OBJ_VAL(function)));

    for(int i = 0; i < function->upvalueCount; i++){
        emitByte(parser, compiler.upvalues[i].isLocal ? 1:0);
        emitByte(parser, compiler.upvalues[i].index);
    }
}

// Class Method declaration
void method(Parser* parser, Token* className){
    consume(parser, TOKEN_FUNCTION, "Expected method declaration.");
    consume(parser, TOKEN_IDENTIFIER, "Expected method name.");
    uint8_t constant = identifierConstant(parser, &parser->previous);

    FunctionType type = TYPE_METHOD;

    // check if method is initializer
    if(
        parser->previous.length == className->length &&
        memcmp(parser->previous.start, className->start, className->length) == 0
    ){
        type = TYPE_INITIALIZER;
    }

    function(parser, type);

    emitBytes(parser, OP_METHOD, constant);
}

Token syntheticToken(const char* text){
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

// Class Declaration
void classDeclaration(Parser* parser){
    consume(parser, TOKEN_IDENTIFIER, "Expected class name.");
    Token className = parser->previous;
    uint8_t nameConstant = identifierConstant(parser, &className);
    declareVariable(parser);

    emitBytes(parser, OP_CLASS, nameConstant);
    defineVariable(parser, nameConstant);

    ClassCompiler classCompiler;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = (struct ClassCompiler*) currentClass;
    currentClass = &classCompiler;

    // Accept Parent class for inheritence
    if(match_parser(parser, TOKEN_LEFT_PAREN)){
        consume(parser, TOKEN_IDENTIFIER, "Expected superclass name.");
        
        variable(parser, false);

        if(identifiersEqual(&className, &parser->previous)){
            error(parser, "A class cannot inherit from itself.");
        }

        beginScope();
        addLocal(parser, syntheticToken("super"));
        defineVariable(parser, 0);

        namedVariable(parser, className, false);
        
        consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' after superclass name.");
        emitByte(parser, OP_INHERIT);
        classCompiler.hasSuperclass = true;
    }

    namedVariable(parser, className, false); // insert class name at top of stack
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before class body.");

    while(!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)){
        method(parser, &className);
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after class body.");
    emitByte(parser, OP_POP); // remove class name from top of stack

    if(classCompiler.hasSuperclass){
        endScope(parser);
    }

    currentClass = (ClassCompiler*) currentClass->enclosing;
}

void dot(Parser* parser, bool canAssign){
    consume(parser, TOKEN_IDENTIFIER, "Expected property name after dot operator.");
    uint8_t name = identifierConstant(parser, &parser->previous);

    if(canAssign && match_parser(parser, TOKEN_EQUAL)){
        expression(parser);
        emitBytes(parser, OP_SET_PROPERTY, name);
    } else if(match_parser(parser, TOKEN_LEFT_PAREN)){
        // method invocation
        uint8_t argCount = argumentList(parser, TOKEN_RIGHT_PAREN);
        emitBytes(parser, OP_INVOKE, name);
        emitByte(parser, argCount);
    } else {
        emitBytes(parser, OP_GET_PROPERTY, name);
    }
}

// Argument processing for function call
uint8_t argumentList(Parser* parser, TokenType endToken){
    uint8_t argCount = 0;

    if(!check(parser, endToken)){
        do{
            expression(parser);
            // Make check for non list objects
            if(endToken != TOKEN_RIGHT_BRACKET && argCount==255){
                error(parser, "Can't have more than 255 arguments.");
            }
            argCount++;
        } while(match_parser(parser, TOKEN_COMMA));
    }

    consume(parser, endToken, "Expected ')' afer arguments.");
    return argCount;
}

// Function Call
void call(Parser* parser, bool canAsign){
    uint8_t argCount = argumentList(parser, TOKEN_RIGHT_PAREN);
    emitBytes(parser, OP_CALL, argCount);
}

void emitShorthandAssign(Parser* parser, uint8_t getOp, uint8_t setOp, OpCode op, uint8_t args){
    emitBytes(parser, getOp, args);
    expression(parser);
    emitByte(parser, op);
    emitBytes(parser, setOp, args);
}


// Identifer named vairiable access
void namedVariable(Parser* parser, Token name, bool canAssign){
    uint8_t getOp, setOp;

    int arg = resolveLocal(parser, current, &name);
    
    if(arg != -1){
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if((arg = resolveUpValue(parser, current, &name)) != -1){
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(parser, &name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }
    
    if(canAssign && match_parser(parser, TOKEN_EQUAL)){
        expression(parser);
        emitBytes(parser, setOp, (uint8_t) arg);
    } else if(canAssign && match_parser(parser, TOKEN_ADD_EQUAL)){
        emitShorthandAssign(parser, getOp, setOp, OP_ADD, (uint8_t) arg);
    } else if(canAssign && match_parser(parser, TOKEN_MINUS_EQUAL)){
        emitShorthandAssign(parser, getOp, setOp, OP_MINUS, (uint8_t) arg);
    } else if(canAssign && match_parser(parser, TOKEN_MULTIPLY_EQUAL)){
        emitShorthandAssign(parser, getOp, setOp, OP_MULTIPLY, (uint8_t) arg);
    } else if(canAssign && match_parser(parser, TOKEN_DIVIDE_EQUAL)){
        emitShorthandAssign(parser, getOp, setOp, OP_DIVIDE, (uint8_t) arg);
    } else if(canAssign && match_parser(parser, TOKEN_MOD_EQUAL)){
        emitShorthandAssign(parser, getOp, setOp, OP_MOD, (uint8_t) arg);
    }
    else {
        emitBytes(parser, getOp, (uint8_t) arg);
    }
}


// Variable access
void variable(Parser* parser, bool canAssign){
    namedVariable(parser, parser->previous, canAssign);
}

// Block
void block(Parser* parser){
    while(!check(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)){
        declaration(parser);
    }

    consume(parser, TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}

void statement(Parser* parser){
    if(match_parser(parser, TOKEN_PRINT)){
        // Parse Print keyword statement
        printStatement(parser);
    } else if(match_parser(parser, TOKEN_IF)){
        ifStatement(parser);
    } else if(match_parser(parser, TOKEN_RETURN)){
        returnStatement(parser);
    } else if(match_parser(parser, TOKEN_FOR)){
        forStatement(parser);
    } else if(match_parser(parser, TOKEN_WHILE)){
        whileStatement(parser);
    } else if(match_parser(parser, TOKEN_LEFT_BRACE)){
        // Parse Block statements
        beginScope();
        block(parser);
        endScope(parser);
    } else if(match_parser(parser, TOKEN_BREAK)){
        breakStatement(parser);
    } else if(match_parser(parser, TOKEN_CONTINUE)){
        continueStatement(parser);
    } else if(match_parser(parser, TOKEN_SWITCH)){
        switchStatement(parser);
    } else if(match_parser(parser, TOKEN_IMPORT)){
        importStatement(parser);
    }
    else {
        expressionStatement(parser);
    }
}

void returnStatement(Parser* parser){
    if(current->type == TYPE_SCRIPT){
        error(parser, "Can't return from top-level code.");
    }
    
    if(!match_parser(parser, TOKEN_SEMICOLON)){
        if(current->type == TYPE_INITIALIZER){
            error(parser, "Can't return value from constructor.");
        }

        expression(parser);   
        match_parser(parser, TOKEN_SEMICOLON);
        emitByte(parser, OP_RETURN);
    } else {
        // if semicolon is found or not, then its direct return.
        emitReturn(parser);
    }
}

void printStatement(Parser* parser){
    expression(parser);
    // Semi-colon optional at end of print statement
    match_parser(parser, TOKEN_SEMICOLON);
    emitByte(parser, OP_PRINT);
}

// Loop - While Statement
void whileStatement(Parser* parser){
    int loopStart = currentChunk()->count;

    // Enclosing condition inside '(' ')' is optional 
    bool paranFound = match_parser(parser, TOKEN_LEFT_PAREN);
    
    expression(parser);

    if(paranFound){
        consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' for closing condition.");
    }

    // Capture Jump to statement if condition fails
    int exitJump = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByte(parser, OP_POP);

    // Run body of while statement
    statement(parser);

    // Add instruction to loop back to initial condition of stack
    emitLoop(parser, loopStart);

    patchJump(parser, exitJump);
    emitByte(parser, OP_POP);
}


// Loop - For Statement
void forStatement(Parser* parser){
    // Variable declared within loop statement are scoped internally only.
    beginScope();

    // Enclosing condition inside '(' ')' is optional 
    bool paranFound = match_parser(parser, TOKEN_LEFT_PAREN);
    // Initializer
    if(match_parser(parser, TOKEN_SEMICOLON)){
        // empty initializer field in for-loop
    } else if(match_parser(parser, TOKEN_VAR) || check(parser, TOKEN_IDENTIFIER) ){
        varDeclaraction(parser);
    } else {
        expressionStatement(parser);
    }
    
    // Condition
    int loopStart = currentChunk()->count;
    int exitJump = -1;
    if(!match_parser(parser, TOKEN_SEMICOLON)){
        expression(parser);
        consume(parser, TOKEN_SEMICOLON, "Expected ';' after loop condition.");

        // Jump out of the loop if condition is false
        exitJump = emitJump(parser, OP_JUMP_IF_FALSE);
        emitByte(parser, OP_POP);
    }

    // Increment expression evaluate
    if(!match_parser(parser, TOKEN_RIGHT_PAREN)){
        // track where increment expression begins in stack
        int bodyJump = emitJump(parser, OP_JUMP);
        int incrementStart = currentChunk()->count;

        // parse increment expression
        expression(parser);
        emitByte(parser, OP_POP);
        if(paranFound){
            consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' for closing condition.");
        }

        // go to start of increment 
        emitLoop(parser, loopStart);
        loopStart = incrementStart;
        patchJump(parser, bodyJump);
    }

    // Body of loop
    statement(parser);
    emitLoop(parser, loopStart);

    if(exitJump != -1){
        patchJump(parser, exitJump);
        emitByte(parser, OP_POP);
    }

    // End scope
    endScope(parser);
}

// TODO
void breakStatement(Parser* parser){
    match_parser(parser, TOKEN_SEMICOLON);
}

// TODO
void continueStatement(Parser* parser){
    
    match_parser(parser, TOKEN_SEMICOLON);
}

// TODO
void importStatement(Parser* parser){
    char* module_name = NULL;
    char* module_file = NULL;

    

    match_parser(parser, TOKEN_SEMICOLON);
}

void switchStatement(Parser* parser){
    bool paranFound = match_parser(parser, TOKEN_LEFT_PAREN);
    expression(parser);
    if(paranFound){
        consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' for switch condition.");
    }
    
    consume(parser, TOKEN_LEFT_BRACE, "Expected '{' before switch cases.");

    int state = 0; // 0: before all cases, 1: before default, 2: after default
    int caseEnds[MAX_SWITCH_CASES];
    int caseCount = 0;
    int previousCaseSkip = -1;

    while(!match_parser(parser, TOKEN_RIGHT_BRACE) && !check(parser, TOKEN_EOF)){
        if(match_parser(parser, TOKEN_CASE) || match_parser(parser, TOKEN_DEFAULT)){
            TokenType caseType = parser->previous.type;

            if(state == 2){
                error(parser, "Can't have another case or default after the default case.");
            }

            if(state == 1){
                // at end of previous casse, jump over others
                caseEnds[caseCount++] = emitJump(parser, OP_JUMP);

                // patch its condition to jump to the next case (current case)
                patchJump(parser, previousCaseSkip);
                emitByte(parser, OP_POP);
            }

            if(caseType == TOKEN_CASE){
                state = 1;

                // See if the case is equal to the value
                emitByte(parser, OP_DUP);
                expression(parser);

                consume(parser, TOKEN_COLON, "Expected ':' after case value.");

                emitByte(parser, OP_EQUAL);
                previousCaseSkip = emitJump(parser, OP_JUMP_IF_FALSE);

                // pop comparison result
                emitByte(parser, OP_POP);
            } else {
                state = 2;
                consume(parser, TOKEN_COLON, "Expected ':' after default.");
                previousCaseSkip = -1;
            }
        } else {
            if(state == 0){
                error(parser, "Can't have statements before any case.");
            }
            statement(parser);
        }
    }
    
    // If ended without default case, then patch its jump
    if(state==1){
        patchJump(parser, previousCaseSkip);
        emitByte(parser, OP_POP);
    }

    // Patch all jump to the end.
    for(int i = 0; i < caseCount; i++){
        patchJump(parser, caseEnds[i]);
    }
    
    emitByte(parser, OP_POP);
}

void expressionStatement(Parser* parser){
    expression(parser);
    match_parser(parser, TOKEN_SEMICOLON);
    emitByte(parser, OP_POP);
}

ObjFunction* compile(const char* source){
    Scanner scanner;
    initScanner(&scanner, source);

    Parser parser;

    parser.scanner = &scanner;    
    parser.hadError = false;
    parser.panicMode = false;

    Compiler compiler;
    initCompiler(&parser, &compiler, TYPE_SCRIPT);

    advance_parser(&parser);

    while(!match_parser(&parser, TOKEN_EOF)){
        declaration(&parser);
    }

    ObjFunction* function = endCompiler(&parser);
    return parser.hadError ? NULL : function;
}

void synchronize(Parser* parser){
    parser->panicMode = false;

    while(parser->current.type != TOKEN_EOF){
        if(parser->previous.type == TOKEN_SEMICOLON) return;

        switch (parser->current.type)
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

        advance_parser(parser);
    }
}

void beginScope(){
    current->scopeDepth++;
}

void endScope(Parser* parser){
    current->scopeDepth--;

    // TODO: Optimization by OP_POPN if sequence of stack items to be removed
    while(current->localCount > 0 && 
        current->locals[current->localCount - 1].depth > current->scopeDepth
    ){
        if(current->locals[current->localCount - 1].isCaptured){
            emitByte(parser, OP_CLOSE_UPVALUE);
        } else {
            emitByte(parser, OP_POP);
        }
        current->localCount--;
    }

}

// If statement control flow
void ifStatement(Parser* parser){
    // encapsulating "(" ")" is optional
    bool paranFound = match_parser(parser, TOKEN_LEFT_PAREN);
    
    expression(parser);

    if(paranFound){
        consume(parser, TOKEN_RIGHT_PAREN, "Expected ')' for closing condition.");
    }

    /*
        if(A){
            B
        } else {
            C
        },
        D

        if A is true, then B should be executed -> After that D should continue executing. // need to jump over else
        if A is false, then C should be executed -> After that D should continue executing. // as usual

    */
    int thenJump = emitJump(parser, OP_JUMP_IF_FALSE);
    emitByte(parser,OP_POP);
    statement(parser);

    int elseJump = emitJump(parser,OP_JUMP);

    patchJump(parser,thenJump);
    emitByte(parser,OP_POP); // Discared condition values

    if(match_parser(parser,TOKEN_ELSE)) statement(parser);
    patchJump(parser, elseJump);
    
}

void markCompilerRoots(){
    Compiler* compiler = current;
    while(compiler != NULL){
        markObject((Obj*)compiler->function);
        compiler = (Compiler *)(compiler->enclosing);
    }
}

void this_(Parser* parser, bool canAssign){
    if(currentClass == NULL){
        error(parser, "Can't use 'this' outside of a class.");
        return;
    }

    variable(parser, false);
}

void super_(Parser* parser, bool canAssign){
    if(currentClass == NULL){
        error(parser, "Can't use 'super' outside of a class.");
    } else if(!currentClass->hasSuperclass) {
        error(parser, "Can't use 'super' in a class with no superclass.");
    }
    consume(parser, TOKEN_DOT, "Expected dot operator after 'super'.");
    consume(parser, TOKEN_IDENTIFIER, "Expected superclass method name.");
    uint8_t name = identifierConstant(parser, &parser->previous);
    
    namedVariable(parser, syntheticToken("this"), false);

    if(match_parser(parser, TOKEN_LEFT_PAREN)){
        uint8_t argCount = argumentList(parser, TOKEN_RIGHT_PAREN);
        namedVariable(parser, syntheticToken("super"), false);
        emitBytes(parser, OP_SUPER_INVOKE, name);
        emitByte(parser, argCount);
    } else {
        namedVariable(parser, syntheticToken("super"), false);
        emitBytes(parser, OP_GET_SUPER, name);
    }
}


void index_expr(Parser* parser, bool canAssign){
    int index_item_count = 1;
    
    expression(parser);

    if(match_parser(parser, TOKEN_COLON)){
        expression(parser);
        index_item_count++;
    }

    consume(parser, TOKEN_RIGHT_BRACKET, "Expected ']' after index expression.");

    // Set Index Expression
    if(canAssign && match_parser(parser, TOKEN_EQUAL)){
        expression(parser);
        emitByte(parser, OP_SET_INDEX);
    }
    // Read Expression
    else {
        emitBytes(parser, OP_INDEX, index_item_count);
    }

    match_parser(parser, TOKEN_SEMICOLON);
}