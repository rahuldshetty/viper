#ifndef viper_scanner_h
#define viper_scanner_h

#include "common.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

void initScanner(const char* source);

bool skipWhitespace();

char advance();
char peek();
char peekNext();

bool isAtEnd();
bool match(char expected);
bool isDigit(char c);
bool isAlpha(char c);

Scanner scanner;

#endif