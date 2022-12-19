#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

void initScanner(const char* source){
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

bool isAtEnd(){
    return *scanner.current == '\0';
}

char peek(){
    return *scanner.current;
}

char peekNext(){
    if(isAtEnd()) return '\0';
    return scanner.current[1];
}

char advance(){
    scanner.current++;
    return scanner.current[-1];
}

bool match(char expected){
    if(isAtEnd()) return false;
    if(*scanner.current != expected) return false;
    scanner.current++;
    return true;
}

void skipWhitespace(){
    for(;;){
        char c = peek();
        switch(c){
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                // advance scanner line
                scanner.line++;
                advance();
                break;
            case '/':
                if(peekNext() == '/'){
                    // skip comment 
                    while (peek() != "\n" && !isAtEnd()) advance();
                } else {
                    return;
                }
            default:
                return;
        }
    }
}

bool isDigit(char c){
    return c >= '0' && c <= '9';
}