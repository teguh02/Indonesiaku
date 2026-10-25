#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

// --- K8: Value-keyed table (dictionaries) --------------------------------

void initValueTable(ValueTable* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeValueTable(ValueTable* table) {
    FREE_ARRAY(ValueEntry, table->entries, table->capacity);
    initValueTable(table);
}

static ValueEntry* findValueEntry(ValueEntry* entries, int capacity,
                                  Value key, uint32_t hash) {
    uint32_t index = hash % (uint32_t)capacity;
    ValueEntry* tombstone = NULL;

    for (;;) {
        ValueEntry* entry = &entries[index];
        if (entry->state == 0) {
            return tombstone != NULL ? tombstone : entry;
        }
        if (entry->state == 2) {
            if (tombstone == NULL) tombstone = entry;
        } else if (valuesEqual(entry->key, key)) {
            return entry;
        }
        index = (index + 1) % (uint32_t)capacity;
    }
}

bool valueTableGet(ValueTable* table, Value key, Value* value) {
    if (table->count == 0) return false;
    uint32_t hash;
    if (!hashValue(key, &hash)) return false;

    ValueEntry* entry = findValueEntry(table->entries, table->capacity, key, hash);
    if (entry->state != 1) return false;
    *value = entry->value;
    return true;
}

static void adjustValueCapacity(ValueTable* table, int capacity) {
    ValueEntry* entries = ALLOCATE(ValueEntry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].state = 0;
        entries[i].key = KOSONG_VAL;
        entries[i].value = KOSONG_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        ValueEntry* entry = &table->entries[i];
        if (entry->state != 1) continue;
        uint32_t hash;
        if (!hashValue(entry->key, &hash)) continue;
        ValueEntry* dest = findValueEntry(entries, capacity, entry->key, hash);
        dest->state = 1;
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(ValueEntry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

// Returns true when the key is new. The key must be hashable; callers check
// hashValue() first so an unhashable key reads as a clear GalatTipe.
bool valueTableSet(ValueTable* table, Value key, Value value) {
    uint32_t hash;
    if (!hashValue(key, &hash)) return false;

    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        adjustValueCapacity(table, GROW_CAPACITY(table->capacity));
    }

    ValueEntry* entry = findValueEntry(table->entries, table->capacity, key, hash);
    bool isNewKey = entry->state != 1;
    if (isNewKey && entry->state == 0) table->count++;

    entry->state = 1;
    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool valueTableDelete(ValueTable* table, Value key) {
    if (table->count == 0) return false;
    uint32_t hash;
    if (!hashValue(key, &hash)) return false;

    ValueEntry* entry = findValueEntry(table->entries, table->capacity, key, hash);
    if (entry->state != 1) return false;
    entry->state = 2;   // tombstone
    entry->key = KOSONG_VAL;
    entry->value = KOSONG_VAL;
    return true;
}

void markValueTable(ValueTable* table) {
    for (int i = 0; i < table->capacity; i++) {
        ValueEntry* entry = &table->entries[i];
        if (entry->state != 1) continue;
        markValue(entry->key);
        markValue(entry->value);
    }
}

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t index = key->hash % capacity;
    Entry* tombstone = NULL;

    for (;;) {
        Entry* entry = &entries[index];
        if (entry->key == NULL) {
            if (IS_KOSONG(entry->value)) {
                // Empty entry.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // We found a tombstone.
                if (tombstone == NULL) tombstone = entry;
            }
        } else if (entry->key == key) {
            // We found the key.
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

bool tableGet(Table* table, ObjString* key, Value* value) {
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

static void adjustCapacity(Table* table, int capacity) {
    Entry* entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = KOSONG_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

bool tableSet(Table* table, ObjString* key, Value value) {
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    if (isNewKey && IS_KOSONG(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table* table, ObjString* key) {
    if (table->count == 0) return false;

    // Find the entry.
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // Place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

void tableAddAll(Table* from, Table* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;

    uint32_t index = hash % table->capacity;
    for (;;) {
        Entry* entry = &table->entries[index];
        if (entry->key == NULL) {
            // Stop if we find an empty non-tombstone entry.
            if (IS_KOSONG(entry->value)) return NULL;
        } else if (entry->key->length == length &&
                   entry->key->hash == hash &&
                   memcmp(entry->key->chars, chars, length) == 0) {
            // We found it.
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}

void tableRemoveWhite(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key != NULL && !entry->key->obj.isMarked) {
            tableDelete(table, entry->key);
        }
    }
}

void markTable(Table* table) {
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        markObject((Obj*)entry->key);
        markValue(entry->value);
    }
}
