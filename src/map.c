#include <stdlib.h>
#include <string.h>

#include "map.h"

// TODO: Tunable paramete - pick best
#define MAP_MAX_LOAD 0.75

void initMap(ObjMap* map){
    map->capacity = 0;
    map->count = 0;
    map->entries = NULL;
}

void freeMap(ObjMap* map){
    FREE_ARRAY(MapEntry, map->entries, map->capacity);
    initMap(map);
}

uint32_t findMapEntry(MapEntry* entries, int capacity, Value key, int count){
    // uint32_t index = key->hash % capacity;
    // Faster modulo calculation
    // A Mod B = A & (B-1), when B is power of 2

    // TODO: FIX HARDCODING OF HASH VALUES
    //  print {"h": 1, "hello": 2, 4:4}
    uint32_t index = 0;
    
    // if(count==0) index = 0;
    // else index = hashValue(key) % capacity;

    for(;;){
        MapEntry* entry = &entries[index];

        if(IS_NULL(entry->key) || valuesEqual(entry->key, key)){
            // printf("INDEX: %u\n", index);
            return index;
        }

        index = (index + 1) % capacity;
    }
}

void adjustMapCapacity(ObjMap* map, int capacity){
    MapEntry* entries = ALLOCATE(MapEntry, capacity);
    for(int i=0; i < capacity; i++){
        entries[i].key = NULL_VAL;
        entries[i].value = NULL_VAL;
    }

    map->count = 0;
    for(int i=0; i < map->capacity; i++){
        MapEntry* entry = &map->entries[i];

        if(IS_NULL(entry->key)) continue;
        uint32_t index = findMapEntry(entries, capacity, entry->key, map->count);
        MapEntry* dest = &map->entries[index];
        dest->key = entry->key;
        dest->value = entry->value;
        map->count++;
    }

    // Garbage collect and remove old hash table
    FREE_ARRAY(MapEntry, map->entries, map->capacity);

    map->entries = entries;
    map->capacity = capacity;
}

bool mapSet(ObjMap* map, Value key, Value value){
    // Ensure bucket never reach TABLE_MAX_LOAD capacity so grow array when it does
    if(map->count + 1 > map->capacity * MAP_MAX_LOAD){
        int capacity = GROW_CAPACITY(map->capacity);
        adjustMapCapacity(map, capacity);
    }

    uint32_t index = findMapEntry(map->entries, map->capacity, key, map->count);
    MapEntry* entry = &map->entries[index];
    bool isNewKey = IS_NULL(entry->key);

    if(isNewKey && IS_NULL(entry->value)) map->count++;

    // printf("ADDRESS:%x\n", entry);
    entry->key = key;
    entry->value = value;

    return isNewKey;
}

bool mapGet(ObjMap* map, Value key, Value* value){
    if(map->count == 0) return false;

    uint32_t index = findMapEntry(map->entries, map->capacity, key, map->count);
    MapEntry* entry = &map->entries[index];
    if(IS_NULL(entry->key)) return false;

    *value =entry->value;
    return true;
}

void mapAddAll(ObjMap* from, ObjMap* to){
    for(int i=0; i < from->capacity; i++){
        MapEntry* entry = &from->entries[i];
        if(!IS_NULL(entry->key)){
            mapSet(to, entry->key, entry->value);
        }
    }
}

bool mapDelete(ObjMap* map, Value key){
    if(map->count == 0) return false;

    //Find entry
    uint32_t index = findMapEntry(map->entries, map->capacity, key, map->count);
    MapEntry* entry = &map->entries[index];
    if(IS_NULL(entry->key)) return false;

    // Place tombstone in the entry field
    entry->key = NULL_VAL;
    entry->value = BOOL_VAL(true);
    return true;
}