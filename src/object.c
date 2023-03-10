#include <stdio.h>
#include <string.h>

#include "file.h"
#include "list.h"
#include "memory.h"
#include "map.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

Obj* allocateObject(size_t size, ObjType type);

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType);

bool isObjType(Value value, ObjType type){
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

ObjString* allocateString(char* chars, int length, uint32_t hash){
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, NULL_VAL);
    pop();
    return string;
}

// Hash Function: FNV-1a
uint32_t hashString(const char* key, int length){
    uint32_t hash = 2166136261u;
    for(int i=0;i<length;i++){
        hash ^= (uint32_t) key[i];
        hash *= 16777619; 
    }
    return hash;
}

ObjString* copyString(const char* chars, int length){
    uint32_t hash = hashString(chars, length);

    // Cache string object and re-use
    ObjString* interned = tableFindString(
        &vm.strings, chars, length, hash
    );
    if(interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

void printFunction(ObjFunction* function){
    if(function->name == NULL){
        printf("<script>");
        return;
    }

    printf("<fn '%s'>", function->name->chars);
}

void printBytes(ObjByte* bytes){
    printf("(");
    for(int i = 0; i < bytes->bytes.count; i++){
        printf("%x", bytes->bytes.byte[i]);
        if(i > 100){ // as bytes can get really heavy
            printf("...");
            break;
        }

        if(i != bytes->bytes.count - 1){
            printf(", ");
        }
    }
    printf(")");
}

Obj* allocateObject(size_t size, ObjType type){
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;

    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

    return object;
}

ObjFunction* newFunction(){
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity=0;
    function->name = NULL;
    function->upvalueCount = 0;
    initChunk(&function->chunk);
    return function;   
}

void printObject(Value value){
    switch(OBJ_TYPE(value)){
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        
        case OBJ_FUNCTION:{
            printFunction(AS_FUNCTION(value));
            break;
        }

        case OBJ_NATIVE:{
            printf("<built-in fn>");
            break;
        }

        case OBJ_BYTE: {
            printBytes(AS_BYTE(value));
            break;
        }

        case OBJ_CLOSURE:{
            printFunction(AS_CLOSURE(value)->function);
            break;
        }

        case OBJ_UPVALUE:{
            printf("upvalue");
            break;      
        }

        case OBJ_CLASS:{
            printf("<class '%s'>", AS_CLASS(value)->name->chars);
            break;
        }

        case OBJ_INSTANCE:{
            printf("<instance '%s'>", AS_INSTANCE(value)->kclass->name->chars);
            break;
        }

        case OBJ_BOUND_METHOD:{
            printFunction(AS_BOUND_METHOD(value)->method->function);
            break;
        }

        case OBJ_LIST:{
            ObjList* list = AS_LIST(value);
            printf("[");
            
            int total = list->array.count;

            for(int i = 0; i < total; i++){
                printValue(list->array.values[i]);
                if(i != total - 1){
                    printf(", ");
                }
            }

            printf("]");
            break;
        }

        case OBJ_MAP:{
            ObjMap* map = AS_MAP(value);
            printf("{");

            int total = map->capacity;
            int count = map->count;

            for(int i = 0; i < total; i++){
                MapEntry* entry = &map->entries[i];

                if(!IS_NULL(entry->key)){
                    printValue(entry->key);
                    printf(": ");
                    printValue(entry->value);
                    count--;

                    if(count != 0){
                        printf(", ");
                    }
                }
            }

            printf("}");
            break;
        }

        case OBJ_FILE:{
            ObjFile* file = AS_FILE(value);
            printf(
                "<file path:'%s' mode:'%s' %s>", \
                file->path->chars,
                file->mode->chars,
                file->isOpen? "active" : "not-active"
            );
            break;
        }

    }
}

ObjString* takeString(char* chars, int length){
    uint32_t hash = hashString(chars, length);

    // Cache string object and re-use
    ObjString* interned = tableFindString(
        &vm.strings, chars, length, hash
    );
    if(interned != NULL){
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjNative* newNative(NativeFn function){
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->type = NATIVE_METHOD;
    native->function.method = function;
    return native;
}

ObjNative* newObjNative(NativeObjFn function){
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->type = NATIVE_OBJ_METHOD;
    native->function.objMethod = function;
    return native;
}

ObjClosure* newClosure(ObjFunction* function){
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    
    for(int i = 0; i < function->upvalueCount; i++){
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;

    closure->upvalueCount = function->upvalueCount;
    closure->upvalues = upvalues;

    return closure;
}

ObjUpvalue* newObjUpvalue(Value* slot){
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->next = NULL;
    upvalue->closed = NULL_VAL;
    return upvalue;
}

ObjClass* newClass(ObjString* name){
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    initTable(&klass->methods);
    return klass;
}


ObjInstance* newInstance(ObjClass* klass){
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->kclass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method){
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);

    bound->receiver = receiver;
    bound->method = method;

    return bound;
}

ObjList* newList(){
    ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    initValueArray(&list->array);
    initTable(&list->nativeMethods);
    initListNativeMethods(list);
    return list;
}

ObjMap* newMap(){
    ObjMap* map = ALLOCATE_OBJ(ObjMap, OBJ_MAP);
    initMap(map);
    return map;
}

ObjFile* newFile(ObjString* path, ObjString* mode){
    ObjFile* file = ALLOCATE_OBJ(ObjFile, OBJ_FILE);
    initTable(&file->nativeMethods);
    initFileNativeMethods(file);
    file->mode = mode;
    file->path = path;
    file->isOpen = false;
    return file;
}

// Format string as <$tag '$name'>
ObjString* sprintTaggedString(const char* tag, const char* name){
    char* result1 = concat("<", tag);
    char* result2 = concat(result1, " '");
    char* result3 = concat(result2, name);
    char* result = concat(result3, "'>");
    
    ObjString* str_result = copyString(result, strlen(result));

    free(result1);
    free(result2);
    free(result3);
    free(result);

    return str_result;
}


// Format string as [str(values)....]
ObjString* sprintList(ObjList* list){
    char *result = "[";
    int total = list->array.count;
    for(int i = 0; i < total; i++){
        result = concat(result, strValue(list->array.values[i])->chars);

        if(i != total-1){
            result = concat(result, ", ");
        }
    }

    result = concat(result, "]");

    ObjString* str_result = copyString(result, strlen(result));

    free(result);

    return str_result;
}

// Format string as {str(key):str(values)...}
ObjString* sprintMap(ObjMap* map){ 
    char *result = "{";

    int count = map->count;
    for(int i = 0; i < map->capacity; i++){
        if(!IS_NULL(map->entries[i].key)){
            MapEntry entry = map->entries[i];
            result = concat(result, strValue(entry.key)->chars);
            result = concat(result, ":");
            result = concat(result, strValue(entry.value)->chars);
            count--;

            if(count!=0){
                result = concat(result, ", ");
            }
        }
    }

    result = concat(result, "}");

    ObjString* str_result = copyString(result, strlen(result));

    free(result);

    return str_result;
}

ObjString* sprintFunction(ObjFunction* function){
    if(function->name == NULL){
        return copyString("<script>", 8);
    }
    return sprintTaggedString("fn", function->name->chars);
}

char *appendString(char *old, char *new_str) {
  // quick exit...
  if(new_str == NULL) {
    return old;
  }

  // find the size of the string to allocate
  const size_t old_len = strlen(old), new_len = strlen(new_str);
  const size_t out_len = old_len + new_len;

  // allocate a pointer to the new string
  char *out = (char *) realloc((void *) old, out_len + 1);

  // concat both strings and return
  if (out != NULL) {
    memcpy(out + old_len, new_str, new_len);
    out[out_len] = '\0';
    return out;
  }

  return old;
}

ObjString* sprintByte(ObjByte* bytes){
    char *str = strdup("(");
    for (int i = 0; i < bytes->bytes.count; i++) {
        char *chars = ALLOCATE(char, snprintf(NULL, 0, "0x%x", bytes->bytes.byte[i]));
        if (chars != NULL) {
            sprintf(chars, "0x%x", bytes->bytes.byte[i]);
            str = appendString(str, chars);
        }

        if (i != bytes->bytes.count - 1) {
            str = appendString(str, ", ");
        }
    }
    str = appendString(str, ")");
    return copyString(str, strlen(str));
}

ObjString* strObject(Value obj){
    switch OBJ_TYPE(obj){
        case OBJ_STRING:
            return AS_STRING(obj);

        case OBJ_FUNCTION:
            return sprintFunction(AS_FUNCTION(obj));
        
        case OBJ_NATIVE:
            return copyString("<built-in fn>", 13);
        
        case OBJ_CLOSURE:
            return sprintFunction(AS_CLOSURE(obj)->function);

        case OBJ_BYTE:
            return sprintByte(AS_BYTE(obj));

        case OBJ_CLASS:
            return sprintTaggedString("class", AS_CLASS(obj)->name->chars);
        
        case OBJ_INSTANCE:{
            ObjClass* class = AS_INSTANCE(obj)->kclass;
            ObjString* string = copyString("str", 3);

            ObjString* result = NULL;
            Value strFunction;

            if(tableGet(&class->methods, string, &strFunction)){
                // callFn(AS_CLOSURE(strFunction), 0);
                // result = AS_STRING(peek_stack(0));
                // push(strFunction);
                // invoke(string, 0);
            }
            if(result != NULL){
                return result;
            }

            return sprintTaggedString("instance", AS_INSTANCE(obj)->kclass->name->chars);
        }

        case OBJ_LIST:
            return sprintList(AS_LIST(obj));

        case OBJ_MAP:
            return sprintMap(AS_MAP(obj));

    }

    return copyString("null", 4);
}

ObjByte* newBytes(int length){
    ObjByte* bytes = ALLOCATE_OBJ(ObjByte, OBJ_BYTE);
    initByteArray(&bytes->bytes, length);
    return bytes;
}

ObjByte* takeBytes(unsigned char* buffer, int length){
    ObjByte* bytes = ALLOCATE_OBJ(ObjByte, OBJ_BYTE);
    bytes->bytes.byte = buffer;
    bytes->bytes.count = length;
    return bytes;
}