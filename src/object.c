#include <stdio.h>
#include <string.h>
#include <math.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"
#include "table.h"

#define ALLOCATE_OBJ(type, objectType) \
    (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;
    object->isMarked = false;
    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu untuk tipe %d\n", (void*)object, size, type);
#endif

    return object;
}

ObjFunction* newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->requiredArity = 0;
    function->paramNames = NULL;
    function->upvalueCount = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjClosure* newClosure(ObjFunction* function) {
    ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
    for (int i = 0; i < function->upvalueCount; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;
    return closure;
}

ObjUpvalue* newUpvalue(Value* slot) {
    ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = KOSONG_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

ObjList* newList() {
    ObjList* list = ALLOCATE_OBJ(ObjList, OBJ_LIST);
    initValueArray(&list->items);
    return list;
}

ObjDict* newDict() {
    ObjDict* dict = ALLOCATE_OBJ(ObjDict, OBJ_DICT);
    initValueTable(&dict->table);
    initValueArray(&dict->keys);
    return dict;
}

bool dictSet(ObjDict* dict, Value key, Value value) {
    bool isNew = valueTableSet(&dict->table, key, value);
    if (isNew) writeValueArray(&dict->keys, key);
    return isNew;
}

bool dictDelete(ObjDict* dict, Value key) {
    if (!valueTableDelete(&dict->table, key)) return false;
    // Remove the key from the insertion-order list, keeping it contiguous so
    // iteration and dictKeyAt never have to skip tombstones.
    for (int i = 0; i < dict->keys.count; i++) {
        if (valuesEqual(dict->keys.values[i], key)) {
            for (int j = i; j < dict->keys.count - 1; j++) {
                dict->keys.values[j] = dict->keys.values[j + 1];
            }
            dict->keys.count--;
            break;
        }
    }
    return true;
}

Value dictKeyAt(ObjDict* dict, int index) {
    if (index < 0 || index >= dict->keys.count) return KOSONG_VAL;
    return dict->keys.values[index];
}

int dictLength(ObjDict* dict) {
    return dict->keys.count;
}

ObjRange* newRange(double start, double stop, double step) {
    ObjRange* range = ALLOCATE_OBJ(ObjRange, OBJ_RANGE);
    range->start = start;
    range->stop = stop;
    range->step = step;
    return range;
}

// Python's len(range): ceil((stop - start) / step), never negative.
double rangeLength(ObjRange* range) {
    double count = ceil((range->stop - range->start) / range->step);
    return count > 0 ? count : 0;
}

// K2: a rentang of bulat yields bulat elements, so `untuk i dalam rentang(3)`
// gives 0, 1, 2 as bulat (like Python's range).
Value rangeElement(ObjRange* range, int index) {
    double value = range->start + (double)index * range->step;
    if (value == floor(value) &&
        value >= -9223372036854775808.0 && value < 9223372036854775808.0) {
        return INT_VAL((int64_t)value);
    }
    return NUMBER_VAL(value);
}

bool rangeContains(ObjRange* range, double value) {
    if (value != floor(value)) return false;
    bool inBounds = range->step > 0
        ? (value >= range->start && value < range->stop)
        : (value <= range->start && value > range->stop);
    return inBounds && fmod(value - range->start, range->step) == 0;
}

ObjClass* newClass(ObjString* name) {
    ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    klass->superclass = NULL;
    initTable(&klass->methods);
    return klass;
}

ObjInstance* newInstance(ObjClass* klass) {
    ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    initTable(&instance->fields);
    return instance;
}

ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method) {
    ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjFile* newFile(void* handle, ObjString* path) {
    ObjFile* file = ALLOCATE_OBJ(ObjFile, OBJ_FILE);
    file->handle = handle;
    file->path = path;
    file->isOpen = true;
    return file;
}

ObjDb* newDb(void) {
    ObjDb* db = ALLOCATE_OBJ(ObjDb, OBJ_DB);
    db->db = NULL;
    return db;
}

ObjSocket* newSocket(long long fd) {
    ObjSocket* sock = ALLOCATE_OBJ(ObjSocket, OBJ_SOCKET);
    sock->fd = fd;
    sock->isOpen = true;
    return sock;
}

ObjTuple* newTuple(ValueArray items) {
    ObjTuple* tuple = ALLOCATE_OBJ(ObjTuple, OBJ_TUPLE);
    tuple->items = items;
    return tuple;
}

ObjSet* newSet() {
    ObjSet* set = ALLOCATE_OBJ(ObjSet, OBJ_SET);
    initValueTable(&set->table);
    initValueArray(&set->items);
    return set;
}

bool setAdd(ObjSet* set, Value value) {
    uint32_t hash;
    if (!hashValue(value, &hash)) {
        vm.runtimeErrorClass = vm.galatTipe;
        runtimeError("Tidak bisa menambah nilai yang tidak bisa di-hash ke himpunan.");
        return false;
    }
    if (valueTableGet(&set->table, value, NULL)) {
        return false;  // sudah ada
    }
    if (!valueTableSet(&set->table, value, value)) {
        runtimeError("Himpunan terlalu besar.");
        return false;
    }
    writeValueArray(&set->items, value);
    return true;
}

bool setRemove(ObjSet* set, Value value) {
    if (!valueTableDelete(&set->table, value)) return false;
    for (int i = 0; i < set->items.count; i++) {
        if (valuesEqual(set->items.values[i], value)) {
            for (int j = i; j < set->items.count - 1; j++) {
                set->items.values[j] = set->items.values[j + 1];
            }
            set->items.count--;
            return true;
        }
    }
    return false;
}

int setLength(ObjSet* set) {
    return set->items.count;
}

Value setMemberAt(ObjSet* set, int index) {
    if (index < 0 || index >= set->items.count) return KOSONG_VAL;
    return set->items.values[index];
}

ObjNative* newNative(NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

// A UTF-8 continuation byte has the form 10xxxxxx; every other byte starts a
// character. Invalid sequences still count one character per lead byte.
static bool isContinuationByte(char c) {
    return ((unsigned char)c & 0xC0) == 0x80;
}

static int countChars(const char* chars, int length) {
    int count = 0;
    for (int i = 0; i < length; i++) {
        if (!isContinuationByte(chars[i])) count++;
    }
    return count;
}

int utf8ByteOffset(ObjString* string, int charIndex) {
    if (string->charCount == string->length) return charIndex;  // ASCII: O(1)
    int seen = 0;
    for (int i = 0; i < string->length; i++) {
        if (!isContinuationByte(string->chars[i])) {
            if (seen == charIndex) return i;
            seen++;
        }
    }
    return string->length;
}

int utf8CharBytes(ObjString* string, int byteOffset) {
    int end = byteOffset + 1;
    while (end < string->length && isContinuationByte(string->chars[end])) end++;
    return end - byteOffset;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->charCount = countChars(chars, length);
    string->chars = chars;
    string->hash = hash;
    // Push onto the VM stack so the string is reachable if tableSet below
    // triggers a garbage collection (interning it into vm.strings).
    push(OBJ_VAL(string));
    tableSet(&vm.strings, string, KOSONG_VAL);
    pop();
    return string;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString* takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }
    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);
    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length, hash);
}

static void printFunction(ObjFunction* function) {
    if (function->name == NULL) {
        printf("<skrip>");
    } else {
        printf("<fungsi %s>", function->name->chars);
    }
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_CLOSURE:
            printFunction(AS_CLOSURE(value)->function);
            break;
        case OBJ_UPVALUE:
            printf("upvalue");
            break;
        case OBJ_LIST: {
            ObjList* list = AS_LIST(value);
            printf("[");
            for (int i = 0; i < list->items.count; i++) {
                if (i > 0) printf(", ");
                printValue(list->items.values[i]);
            }
            printf("]");
            break;
        }
        case OBJ_DICT: {
            ObjDict* dict = AS_DICT(value);
            printf("{");
            bool first = true;
            // Insertion order, and any hashable key type (K8).
            for (int i = 0; i < dict->keys.count; i++) {
                Value key = dict->keys.values[i];
                Value entryValue;
                if (!valueTableGet(&dict->table, key, &entryValue)) continue;
                if (!first) printf(", ");
                first = false;
                if (IS_STRING(key)) {
                    printf("\"%s\": ", AS_CSTRING(key));
                } else {
                    printValue(key);
                    printf(": ");
                }
                printValue(entryValue);
            }
            printf("}");
            break;
        }
        case OBJ_NATIVE:
            printf("<fungsi bawaan>");
            break;
        case OBJ_CLASS:
            printf("<kelas %s>", AS_CLASS(value)->name->chars);
            break;
        case OBJ_INSTANCE: {
            ObjInstance* instance = AS_INSTANCE(value);
            if (strncmp(instance->klass->name->chars, "Galat", 5) == 0) {
                for (int i = 0; i < instance->fields.capacity; i++) {
                    Entry* entry = &instance->fields.entries[i];
                    if (entry->key != NULL && strcmp(entry->key->chars, "pesan") == 0) {
                        printValue(entry->value);
                        return;
                    }
                }
            }
            printf("<objek %s>", instance->klass->name->chars);
            break;
        }
        case OBJ_BOUND_METHOD:
            printFunction(AS_BOUND_METHOD(value)->method->function);
            break;
        case OBJ_FILE: {
            ObjFile* file = AS_FILE(value);
            printf("<berkas %s%s>", file->path->chars,
                   file->isOpen ? "" : " (tertutup)");
            break;
        }
        case OBJ_DB:
            printf("<db%s>", AS_DB(value)->db ? "" : " (tertutup)");
            break;
        case OBJ_SOCKET: {
            ObjSocket* sock = AS_SOCKET(value);
            printf("<soket%s>", sock->isOpen ? "" : " (tertutup)");
            break;
        }
        case OBJ_RANGE: {
            // Printed like Python: range(0, 5) / range(0, 10, 2).
            ObjRange* range = AS_RANGE(value);
            char start[32], stop[32], step[32];
            rangeNumberToString(range->start, start, sizeof(start));
            rangeNumberToString(range->stop, stop, sizeof(stop));
            if (range->step == 1) {
                printf("rentang(%s, %s)", start, stop);
            } else {
                rangeNumberToString(range->step, step, sizeof(step));
                printf("rentang(%s, %s, %s)", start, stop, step);
            }
            break;
        }
    }
}
