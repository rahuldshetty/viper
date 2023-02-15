#ifndef viper_scanner_h
#define viper_scanner_h

#include "common.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

void initScanner(Scanner*, const char*);

bool skipWhitespace(Scanner*);

char advance(Scanner*);
char peek(Scanner*);
char peekNext(Scanner*);

bool isAtEnd(Scanner*);
bool match(Scanner*, char expected);
bool isDigit(char c);
bool isAlpha(char c);

#endif