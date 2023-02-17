#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

void repl(VM* vm){
    char line[1024];
    for(;;){
        printf(">> ");

        if(!fgets(line, sizeof(line), stdin)){
            printf("\n");
            break;
        }
        interpret(vm, line);
    }
}

char* readFile(const char* path){
    FILE* file = fopen(path, "rb");

    if(file==NULL){
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    // Read file size
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    // Allocate dynamic string
    char* buffer = (char*) malloc(fileSize + 2);
    if(buffer==NULL){
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if(bytesRead < fileSize){
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytesRead] = '\n';
    buffer[bytesRead + 1] = '\0';

    fclose(file);
    return buffer;
}

void runFile(VM* vm, const char* path){
    char* source = readFile(path);
    InterpretResult result = interpret(vm, source);
    free(source);

    if(result == INTERPRET_COMPILE_ERROR) exit(65);
    if(result == INTERPRET_RUNTIME_ERROR) exit(70);
}

int main(int argc, const char* argv[]){
    initVM();
    
    // runFile("examples/functions/function.viper");
    //runFile("examples/functions/built_in_function.viper");
    if (argc == 1){
        repl(&vm);
    } else if(argc == 2){
        runFile(&vm, argv[1]);
    } else {
        fprintf(stderr, "Usage: viper [path]\n");
        exit(64);
    }

    freeVM();        
    return 0;
}