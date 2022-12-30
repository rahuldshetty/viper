#ifndef viper_map_h
#define viper_map_h

#include "common.h"
#include "memory.h"
#include "object.h"
#include "value.h"

void initMap(ObjMap* map);
void freeMap(ObjMap* map);
bool mapGet(ObjMap* map, Value key, Value* value);
bool mapSet(ObjMap* map, Value key, Value value);
bool mapDelete(ObjMap* map, Value key);
void mapAddAll(ObjMap* from, ObjMap* to);

#endif