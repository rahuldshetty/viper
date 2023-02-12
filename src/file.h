#ifndef viper_file_h
#define viper_file_h

#include "common.h"
#include "object.h"

bool is_valid_mode(const char* mode);

ObjFile* file_open(ObjString* path, ObjString* mode);
void initFileNativeMethods(ObjFile* file);

#endif