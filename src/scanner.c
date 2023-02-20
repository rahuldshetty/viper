#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

void initScanner(Scanner* scanner, const char* source){
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
}

bool isAtEnd(Scanner* scanner){
    return *scanner->current == '\0';
}

char previous(Scanner* scanner){
    return scanner->current[-1];
}

char peek(Scanner* scanner){
    return *scanner->current;
}

char peekNext(Scanner* scanner){
    if(isAtEnd(scanner)) return '\0';
    return scanner->current[1];
}

char advance(Scanner* scanner){
    scanner->current++;
    return scanner->current[-1];
}

bool match(Scanner* scanner, char expected){
    if(isAtEnd(scanner)) return false;
    if(*scanner->current != expected) return false;
    scanner->current++;
    return true;
}

bool skipBlockComments(Scanner* scanner){
    if(match(scanner, '/') && match(scanner, '*')){
        int nesting = 1;

        while(nesting > 0){
            if(isAtEnd(scanner)){
                return false;
            }

            if(peek(scanner) == '/' && peekNext(scanner) == '*'){
                advance(scanner);
                advance(scanner);
                nesting++;
            }
            
            if(peek(scanner) == '*' && peekNext(scanner) == '/'){
                advance(scanner);
                advance(scanner);
                nesting--;
            } 

            advance(scanner);
        }
    }
    return true;
}

bool skipWhitespace(Scanner* scanner){
    for(;;){
        char c = peek(scanner);

        switch(c){
            case ' ':
            case '\r':
            case '\t':
                advance(scanner);
                break;

            case '\n':
                // advance scanner line
                scanner->line++;
                advance(scanner);
                break;

            case '/':
                // Single line comment
                if(peekNext(scanner) == '/'){
                    // skip comment 
                    while (peek(scanner) != '\n' && !isAtEnd(scanner)) advance(scanner);
                    
                    // ignore new line character
                    scanner->line++;
                    advance(scanner);
                    continue;
                }
                else if(peekNext(scanner) == '*'){
                    // Skip multi-line comments
                    if(!skipBlockComments(scanner)){
                        return false;
                    }
                    continue;
                }
                else {
                    return true;
                }
            default:
                return true;
        }
    }
}

bool isDigit(char c){
    return c >= '0' && c <= '9';
}

bool isAlpha(char c){
    return (
        (c >= 'a' && c <= 'z') || 
        (c >= 'A' && c <= 'Z') ||
        c == '_'
    );
}

bool isBinary(char c){
    return c == '0' || c == '1';
}

bool isOctal(char c){
    return c >= '0' && c <= '7';
}

bool isHex(char c){
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}