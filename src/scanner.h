#ifndef viper_scanner_h
#define viper_scanner_h

#include "common.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

void initScanner(const char* source);

void skipWhitespace();

char advance();
char peek();
char peekNext();

bool match(char expected);
bool isDigit(char c);
bool isAlpha(char c);

Scanner scanner;

#endif