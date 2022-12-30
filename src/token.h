#ifndef viper_token_h
#define viper_token_h

typedef enum{
    // Single-character tokens.
    TOKEN_LEFT_BRACKET, TOKEN_RIGHT_BRACKET,
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_ADD,
    TOKEN_COLON, TOKEN_SEMICOLON, TOKEN_DIVIDE, TOKEN_MULTIPLY,
    
    // One or two character tokens.
    TOKEN_NOT, TOKEN_NOT_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    
    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUNCTION, TOKEN_IF, TOKEN_NULL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_ERROR, TOKEN_EOF
} TokenType;

typedef struct{
    TokenType type;
    const char* start;
    int length;
    int line;
} Token;

Token scanToken();
Token makeToken(TokenType type);
Token errorToken(const char* message);

Token string();
Token number();
Token identifier();

TokenType identifierType();
TokenType checkKeyword(int start, int length, const char* rest, TokenType type);

#endif