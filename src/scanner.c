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

bool skipBlockComments(){
    if(match('/') && match('*')){
        int nesting = 1;

        while(nesting > 0){
            if(isAtEnd()){
                return false;
            }

            if(peek() == '/' && peekNext() == '*'){
                advance();
                advance();
                nesting++;
            }
            
            if(peek() == '*' && peekNext() == '/'){
                advance();
                advance();
                nesting--;
            } 

            advance();
        }
    }
    return true;
}

bool skipWhitespace(){
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
                // Single line comment
                if(peekNext() == '/'){
                    // skip comment 
                    while (peek() != '\n' && !isAtEnd()) advance();
                    
                    // ignore new line character
                    scanner.line++;
                    advance();
                    continue;
                }
                else if(peekNext() == '*'){
                    // Skip multi-line comments
                    if(!skipBlockComments()){
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