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

MapEntry* findMapEntry(MapEntry* entries, int capacity, Value key){
    uint32_t index = hashValue(key) % capacity;

    MapEntry* tombstone = NULL;

    for(;;){
        MapEntry* entry = &entries[index];

        if(IS_NULL(entry->key)){
            if(IS_NULL(entry->value)){
                // Empty entry
                return tombstone != NULL ? tombstone : entry;
            } else {
                // found tombstone
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (valuesEqual(entry->key, key)){
            // found key
            return entry;
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
        MapEntry* dest = findMapEntry(entries, capacity, entry->key);
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

    MapEntry* entry  = findMapEntry(map->entries, map->capacity, key);
    bool isNewKey = IS_NULL(entry->key);

    if(isNewKey && IS_NULL(entry->value)) map->count++;

    entry->key = key;
    entry->value = value;

    return isNewKey;
}

bool mapGet(ObjMap* map, Value key, Value* value){
    if(map->count == 0) return false;

    MapEntry* entry = findMapEntry(map->entries, map->capacity, key);
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
    MapEntry* entry = findMapEntry(map->entries, map->capacity, key);
    if(IS_NULL(entry->key)) return false;

    // Place tombstone in the entry field
    entry->key = NULL_VAL;
    entry->value = BOOL_VAL(true);
    return true;
}