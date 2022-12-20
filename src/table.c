#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

// TODO: Tunable paramete - pick best
#define TABLE_MAX_LOAD 0.75

void initTable(Table* table){
    table->capacity = 0;
    table->count = 0;
    table->entires = NULL;
}

void freeTable(Table* table){
    FREE_ARRAY(Entry, table->entires, table->capacity);
    initTable(table);
}

Entry* findEntry(Entry* entires, int capacity, ObjString* key){
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;

    for(;;){
        Entry* entry = &entires[index];

        if(entry->key == NULL){
            if(IS_NULL(entry->value)){
                // Empty entry
                return tombstone != NULL ? tombstone : entry;
            } else {
                // found tombstone
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key){
            // found key
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

void adjustCapacity(Table* table, int capacity){
    Entry* entries = ALLOCATE(Entry, capacity);
    for(int i=0; i < capacity; i++){
        entries[i].key = NULL;
        entries[i].value = NULL_VAL;
    }

    table->count = 0;
    for(int i=0; i < table->capacity; i++){
        Entry* entry = &table->entires[i];
        if(entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    // Garbage collect and remove old hash table
    FREE_ARRAY(Entry, table->entires, table->capacity);
    table->entires = entries;
    table->capacity = capacity;
}

bool tableSet(Table* table, ObjString* key, Value value){
    // Ensure bucket never reach TABLE_MAX_LOAD capacity so grow array when it does
    if(table->count + 1 > table->capacity * TABLE_MAX_LOAD){
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entires, table->capacity, key);
    bool isNewKey = entry->key == NULL;

    if(isNewKey && IS_NULL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableGet(Table* table, ObjString* key, Value* value){
    if(table->count == 0) return false;

    Entry* entry = findEntry(table->entires, table->capacity, key);
    if(entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

void tableAddAll(Table* from, Table* to){
    for(int i=0; i < from->capacity; i++){
        Entry* entry = &from->entires[i];
        if(entry->key != NULL){
            tableSet(to, entry->key, entry->value);
        }
    }
}

bool tableDelete(Table* table, ObjString* key){
    if(table->count == 0) return false;

    //Find entry
    Entry* entry = findEntry(table->entires, table->capacity, key);
    if(entry->key == NULL) return false;

    // Place tombstone in the entry field
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

// String interning
ObjString* tableFindString(
    Table* table, 
    const char* chars,
    int length, 
    uint32_t hash
){
    if(table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;

    for(;;){
        Entry* entry = &table->entires[index];

        if(entry->key == NULL){
            // Stop if we find an empty non-tombstone entry.
            if (IS_NULL(entry->value)) return NULL;
        } else if(entry->key->length == length &&
            entry->key->hash == hash &&
            memcmp(entry->key->chars, chars, length) == 0){
            // We found it.
            return entry->key;
        }
        index = (index+1) % table -> capacity;
    }
}