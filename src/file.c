#include <string.h>
#include <sys/stat.h>

#include "builtin.h"
#include "file.h"
#include "memory.h"
#include "object.h"
#include "runtime.h"

#ifdef _WIN32

/* Symbolic links aren't really a 'thing' on Windows, so just use plain-old
 * stat() instead of lstat(). */
#define lstat stat

#endif /* ifdef _WIN32 */

bool is_valid_mode(const char* mode){

}


void _file_open(ObjFile* file){
    if(!file->isOpen){
        char* mode = file->mode->chars;
        file->file = fopen(file->path->chars, mode);
        if(file->file != NULL)
            file->isOpen = true;
    }
}

int _file_close(ObjFile* file){
    int result = -1;
    if(file->isOpen){
        fflush(file->file);
        result = fclose(file->file);
        file->file = NULL;
        file->isOpen = false;
    }
    return result;
}

bool is_binary_mode(ObjFile* file){
    return strstr(file->mode->chars, "b") != NULL;
}

bool is_file_open(ObjFile* file){
    return file->isOpen && file->file != NULL;
}

// TODO: Binary file handling, checking readonly mode
bool file_exists(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to read method.");
        return false;
    }

    ObjFile* file = AS_FILE(self);
    _file_open(file);

    bool exists = file->isOpen;

    _file_close(file);

    args[-1] = BOOL_VAL(exists);
    return true;
}

// TODO: checking readonly mode
bool file_read(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to read method.");
        return false;
    }

    ObjFile* file = AS_FILE(self);

    _file_open(file);

    if(!file->isOpen){
        args[-1] = errorOutput("Unable to read file, make sure it is active before reading.");
        return false;
    }

    // Get file size
    size_t file_size = -1;
    size_t file_size_real = -1;

    struct stat stats;
    if (lstat(file->path->chars, &stats) == 0) {
      file_size_real = (size_t) stats.st_size;
    } else {
      // fallback
      fseek(file->file, 0L, SEEK_END);
      file_size_real = ftell(file->file);
      rewind(file->file);
    }

    if (file_size == (size_t) -1 || file_size > file_size_real) {
      file_size = file_size_real;
    }

    // Allocate memory for reading data
    char *buffer = ALLOCATE(char, file_size + 1);

    if(buffer == NULL && file_size != 0){
        // TODO: error unable to create new memory
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, file->file);
    rewind(file->file);

    if (bytes_read == 0 && file_size != 0 && file_size == file_size_real) {
        // TODO:  couldn't read file contents
    }

    if(buffer != NULL) buffer[bytes_read] = '\0';

    if(is_binary_mode(file)){
        ObjByte* obj_byte = takeBytes((unsigned char* )(buffer), bytes_read);
        args[-1] = OBJ_VAL(obj_byte);
    } else {
        args[-1] = OBJ_VAL(copyString(buffer, file_size + 1));
    }

    return true;
}

// TODO: Binary file handling, checking readonly mode
bool file_write(int argCount, Value self, Value* args){
    if(argCount != 1){
        args[-1] = errorOutput("Expected 1 argument to write method.");
        return false;
    }

    if(!IS_STRING(args[0]) && !IS_BYTE(args[0])){
        args[-1] = errorOutput("Expected string or bytes datatype for argument to file write method.");
        return false;
    }

    ObjFile* file = AS_FILE(self);

    // file is opened in read-only mode
    if(strstr(file->mode->chars, "r") != NULL && strstr(file->mode->chars, "+") == NULL){
        args[-1] = errorOutput("Unable to write data into read-only file.");
        return false;
    }

    _file_open(file);

    if(!file->isOpen){
        args[-1] = errorOutput("Unable to open file for writing.");
        return false;
    }

    unsigned char* data;
    int length;

    if(is_binary_mode(file)){
        ObjByte* bytes = AS_BYTE(args[0]);
        data = bytes->bytes.byte;
        length = bytes->bytes.count;       
    } else {
        ObjString* string = AS_STRING(args[0]);
        data = (unsigned char *) string->chars;
        length = string->length;
    }

    size_t count = fwrite(data, sizeof(unsigned char), length, file->file);
    fflush(file->file);

    if(count > (size_t) 0){
        args[-1] = NUMBER_VAL(count);
        return true;
    } else {
        args[-1] = errorOutput("Unable to write data.");
        return false;
    }
}

bool mfile_open(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to open method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    _file_open(file);

    if(!file->isOpen){
        args[-1] = errorOutput("Unable to open file.");
        return false;
    }

    return true;
}

bool mfile_close(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to close method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    _file_close(file);
    return true;
}

bool file_is_open(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to is_open method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    args[-1] = BOOL_VAL(is_file_open(file));
    return true;
}

bool file_is_closed(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to is_open method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    args[-1] = BOOL_VAL(!is_file_open(file));
    return true;
}

bool file_mode(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to is_open method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    args[-1] = OBJ_VAL(file->mode);
    return true;
}

bool file_path(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to is_open method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    args[-1] = OBJ_VAL(file->path);
    return true;
}

ObjFile* file_open(ObjString* path, ObjString* mode){
    ObjFile* file = newFile(path, mode);
    _file_open(file);
    return file;
}

void initFileNativeMethods(ObjFile* file){
    addNativeObjMethod(&file->nativeMethods, "read", file_read);
    addNativeObjMethod(&file->nativeMethods, "exists", file_exists);
    addNativeObjMethod(&file->nativeMethods, "write", file_write);
    addNativeObjMethod(&file->nativeMethods, "open", mfile_open);
    addNativeObjMethod(&file->nativeMethods, "close", mfile_close);

    addNativeObjMethod(&file->nativeMethods, "is_open", file_is_open);
    addNativeObjMethod(&file->nativeMethods, "mode", file_mode);
    addNativeObjMethod(&file->nativeMethods, "path", file_path);
    addNativeObjMethod(&file->nativeMethods, "is_closed", file_is_closed);
}