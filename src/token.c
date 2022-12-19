#include <string.h>

#include "scanner.h"
#include "token.h"

Token scanToken(){
    skipWhitespace();
    scanner.start = scanner.current;

    if(isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if(isAlpha(c)) return identifier();
    if(isDigit(c)) return number();

    switch (c)
    {
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
    if(peek() == "." && isDigit(peekNext())){
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
    return TOKEN_IDENTIFIER;
}