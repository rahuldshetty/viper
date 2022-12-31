#include <string.h>

#include "scanner.h"
#include "token.h"

// TODO: String interpolation ${}
// TODO: Contextual Keywords - async, await, 

Token scanToken(){
    skipWhitespace();
    if(isAtEnd()) return makeToken(TOKEN_EOF);

    scanner.start = scanner.current;
    
    if(isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    // Make sure current  token is not EOF
    if(isAtEnd()) return makeToken(TOKEN_EOF);

    if(isAlpha(c)) return identifier();
    if(isDigit(c)) return number();

    switch (c)
    {
        case '[': return makeToken(TOKEN_LEFT_BRACKET);
        case ']': return makeToken(TOKEN_RIGHT_BRACKET);
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_ADD);
        case '/': return makeToken(TOKEN_DIVIDE);
        case '*': return makeToken(TOKEN_MULTIPLY);
        case ':': return makeToken(TOKEN_COLON);

        case '!':
            return makeToken(
                match('=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);
        case '=':
            return makeToken(
                match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
                match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
                match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        case '"': return string();

    }

    return errorToken("Unexpected character.");
}


Token makeToken(TokenType type){
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

Token errorToken(const char* message){
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = scanner.line;
    return token;
}

Token string(){
    while(peek()!='"' && !isAtEnd()){
        if(peek() == '\n') scanner.line++;
        advance();
    }

    if(isAtEnd()) return errorToken("Unterminated string.");

    // closing quote
    advance();
    return makeToken(TOKEN_STRING);
}

Token number(){
    while(isDigit(peek())) advance();

    // Look for fractional part by matching dot
    if(peek() == '.' && isDigit(peekNext())){
        // consume dot
        advance();

        while(isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

Token identifier(){
    while(isAlpha(peek()) || isDigit(peek()) ) advance();
    return makeToken(identifierType());
}

TokenType identifierType(){
    switch (scanner.start[0])
    {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f':
            if(scanner.current - scanner.start == 2 && scanner.start[1] == 'n'){
                return TOKEN_FUNCTION;
            } else if(scanner.current - scanner.start > 1){
                switch (scanner.start[1])
                {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                }
            }
            break;
        case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 3, "ull", TOKEN_NULL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

TokenType checkKeyword(int start, int length, const char* rest, TokenType type){
    if(scanner.current - scanner.start == start + length &&
            memcmp(scanner.start + start, rest, length) == 0){
        return type;            
    }
    return TOKEN_IDENTIFIER;
}