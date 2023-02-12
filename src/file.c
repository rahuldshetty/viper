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

// TODO: Binary file handling, checking readonly mode
bool file_read(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to read method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);

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

    args[-1] = OBJ_VAL(copyString(buffer, file_size + 1));
    return true;
}

bool file_close(int argCount, Value self, Value* args){
    if(argCount != 0){
        args[-1] = errorOutput("Expected 0 argument to close method.");
        return false;
    }
    ObjFile* file = AS_FILE(self);
    if(file->isOpen){
        fclose(file->file);
        file->file = NULL;
        file->isOpen = false;
    }
    return true;
}

ObjFile* file_open(ObjString* path, ObjString* mode){
    ObjFile* file = newFile(path, mode);
    file->file = fopen(file->path->chars, mode->chars);
    if(file->file != NULL)
        file->isOpen = true; 
    return file;
}

void initFileNativeMethods(ObjFile* file){
    addNativeObjMethod(&file->nativeMethods, "read", file_read);
    addNativeObjMethod(&file->nativeMethods, "close", file_close);
}