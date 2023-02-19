#include <string.h>

#include "token.h"

// TODO: String interpolation ${}
// TODO: Contextual Keywords - async, await, 

Token scanToken(Scanner* scanner){
    if(!skipWhitespace(scanner)){
        return errorToken(scanner, "Unexepcted whitespace character.");
    }

    if(isAtEnd(scanner)) return makeToken(scanner, TOKEN_EOF);

    scanner->start = scanner->current;
    
    if(isAtEnd(scanner)) return makeToken(scanner, TOKEN_EOF);

    char c = advance(scanner);

    // Make sure current  token is not EOF
    if(isAtEnd(scanner)) return makeToken(scanner, TOKEN_EOF);
    if(isAlpha(c)) return identifier(scanner);
    if(isDigit(c)) return number(scanner);

    switch (c)
    {
        case '[': return makeToken(scanner, TOKEN_LEFT_BRACKET);
        case ']': return makeToken(scanner, TOKEN_RIGHT_BRACKET);
        case '(': return makeToken(scanner, TOKEN_LEFT_PAREN);
        case ')': return makeToken(scanner, TOKEN_RIGHT_PAREN);
        case '{': return makeToken(scanner, TOKEN_LEFT_BRACE);
        case '}': return makeToken(scanner, TOKEN_RIGHT_BRACE);
        case ';': return makeToken(scanner, TOKEN_SEMICOLON);
        case ',': return makeToken(scanner, TOKEN_COMMA);
        case '.': return makeToken(scanner, TOKEN_DOT);
        case '-': return makeToken(
            scanner, 
            match(scanner, '=') ?
            TOKEN_MINUS_EQUAL:
            TOKEN_MINUS
        );
        case '+': return makeToken(
            scanner, 
            match(scanner, '=') ?
            TOKEN_ADD_EQUAL:    
            TOKEN_ADD
        );
        case '/': return makeToken(
            scanner, 
            match(scanner, '=') ?
            TOKEN_DIVIDE_EQUAL:
            TOKEN_DIVIDE
        );
        case '*': return makeToken(
            scanner, 
            match(scanner, '=') ? 
            TOKEN_MULTIPLY_EQUAL:
            TOKEN_MULTIPLY
        );
        case '%': return makeToken(
            scanner, 
            match(scanner, '=') ?
            TOKEN_MOD_EQUAL:
            TOKEN_MOD
        );
        case ':': return makeToken(scanner, TOKEN_COLON);
        case '?': return makeToken(scanner, TOKEN_QUESTION);

        case '!':
            return makeToken(
                scanner, 
                match(scanner, '=') ? TOKEN_NOT_EQUAL : TOKEN_NOT);
        case '=':
            return makeToken(
                scanner, 
                match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(
                scanner, 
                match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(
                scanner, 
                match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

        case '"': return string(scanner, '"');
        case '\'': return string(scanner, '\'');

    }

    return errorToken(scanner, "Unexpected character.");
}


Token makeToken(Scanner* scanner, TokenType type){
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    return token;
}

Token errorToken(Scanner* scanner, const char* message){
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = scanner->line;
    return token;
}

Token string(Scanner* scanner, char delimiter){
    while( peek(scanner) != delimiter && !isAtEnd(scanner)){
        if(peek(scanner) == '\n') scanner->line++;
        advance(scanner);
    }

    if(isAtEnd(scanner)) return errorToken(scanner, "Unterminated string.");

    // closing quote
    advance(scanner);
    return makeToken(scanner, TOKEN_STRING);
}

Token number(Scanner* scanner){
    while(isDigit(peek(scanner))) advance(scanner);

    // Look for fractional part by matching dot
    if(peek(scanner) == '.' && isDigit(peekNext(scanner))){
        // consume dot
        advance(scanner);

        while(isDigit(peek(scanner))) advance(scanner);
    }

    return makeToken(scanner, TOKEN_NUMBER);
}

Token identifier(Scanner* scanner){
    while(isAlpha(peek(scanner)) || isDigit(peek(scanner)) ) advance(scanner);
    return makeToken(scanner, identifierType(scanner));
}

TokenType identifierType(Scanner* scanner){
    switch (scanner->start[0])
    {
        case 'a': 
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'n': return checkKeyword(scanner, 2, 1, "d", TOKEN_AND);
                    case 's': return checkKeyword(scanner, 2, 0, "",  TOKEN_AS);
                }
            }
        case 'b': return checkKeyword(scanner, 1, 4, "reak", TOKEN_BREAK);
        case 'c':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'a': return checkKeyword(scanner, 2, 2, "se", TOKEN_CASE);
                    case 'l': return checkKeyword(scanner, 2, 3, "ass", TOKEN_CLASS);
                    case 'o': return checkKeyword(scanner, 2, 6, "ntinue", TOKEN_CONTINUE);
                }
            }
        case 'd': return checkKeyword(scanner, 1, 6, "efault", TOKEN_DEFAULT);
        case 'e':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'a': return checkKeyword(scanner, 2, 2, "ch", TOKEN_EACH);
                    case 'l': return checkKeyword(scanner, 2, 2, "se", TOKEN_ELSE);
                }
            }
        case 'f':
            if(scanner->current - scanner->start == 2 && scanner->start[1] == 'n'){
                return TOKEN_FUNCTION;
            } else if(scanner->current - scanner->start > 1){
                switch (scanner->start[1])
                {
                    case 'a': return checkKeyword(scanner, 2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(scanner, 2, 1, "r", TOKEN_FOR);
                }
            }
            break;
        case 'i': 
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'f': return checkKeyword(scanner, 2, 0, "", TOKEN_IF);
                    case 'm': return checkKeyword(scanner, 2, 4, "port", TOKEN_IMPORT);
                    case 'n': return checkKeyword(scanner, 2, 0, "", TOKEN_IN);
                }
            }
        case 'n': return checkKeyword(scanner, 1, 3, "ull", TOKEN_NULL);
        case 'o': return checkKeyword(scanner, 1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(scanner, 1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(scanner, 1, 5, "eturn", TOKEN_RETURN);
        case 's': if (scanner->current - scanner->start > 1) {
            switch (scanner->start[1]) {
                case 'w': return checkKeyword(scanner, 2, 4, "itch", TOKEN_SWITCH);
                case 'u': return checkKeyword(scanner, 2, 3, "per", TOKEN_SUPER);
            }
        }
        case 't':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'h': return checkKeyword(scanner, 2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(scanner, 2, 2, "ue", TOKEN_TRUE);
                }
            }
            break;
        case 'v': return checkKeyword(scanner, 1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(scanner, 1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

TokenType checkKeyword(Scanner* scanner, int start, int length, const char* rest, TokenType type){
    if(scanner->current - scanner->start == start + length &&
            memcmp(scanner->start + start, rest, length) == 0){
        return type;            
    }
    return TOKEN_IDENTIFIER;
}