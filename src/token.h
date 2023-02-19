#ifndef viper_token_h
#define viper_token_h

#include "scanner.h"

typedef enum{
    // Single-character tokens.
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_ADD,
    TOKEN_COLON, TOKEN_SEMICOLON, TOKEN_DIVIDE, TOKEN_MULTIPLY, TOKEN_MOD,
    TOKEN_QUESTION,

    // Assign-Operators
    TOKEN_MINUS_EQUAL, TOKEN_ADD_EQUAL,
    TOKEN_DIVIDE_EQUAL, TOKEN_MULTIPLY_EQUAL, TOKEN_MOD_EQUAL,
    
    // One or two character tokens.
    TOKEN_NOT, TOKEN_NOT_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    
    // Literals.
    TOKEN_IDENTIFIER, TOKEN_NUMBER,
    TOKEN_STRING, TOKEN_STRING_INTERPOLATION,

    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_EACH, TOKEN_FUNCTION, TOKEN_IF, TOKEN_NULL, TOKEN_OR, TOKEN_IN,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, TOKEN_BREAK, TOKEN_CONTINUE,
    TOKEN_CASE, TOKEN_DEFAULT, TOKEN_SWITCH, 
    TOKEN_AS, TOKEN_IMPORT,

    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct{
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

Token scanToken(Scanner*);
Token makeToken(Scanner*, TokenType);
Token errorToken(Scanner*, const char*);

Token string(Scanner*, char);
Token number(Scanner*);
Token identifier(Scanner*);

TokenType identifierType(Scanner*);
TokenType checkKeyword(Scanner*, int start, int length, const char* rest, TokenType type);

#endif