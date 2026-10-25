#ifndef indonesiaku_object_h
#define indonesiaku_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define IS_STRING(value)       isObjType(value, OBJ_STRING)
#define IS_FUNCTION(value)     isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)
#define IS_CLOSURE(value)      isObjType(value, OBJ_CLOSURE)
#define IS_LIST(value)         isObjType(value, OBJ_LIST)
#define IS_DICT(value)         isObjType(value, OBJ_DICT)
#define IS_CLASS(value)        isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value)     isObjType(value, OBJ_INSTANCE)
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_FILE(value)         isObjType(value, OBJ_FILE)
#define IS_DB(value)           isObjType(value, OBJ_DB)
#define AS_DB(value)           ((ObjDb*)AS_OBJ(value))

#define IS_SOCKET(value)       isObjType(value, OBJ_SOCKET)
#define IS_RANGE(value)        isObjType(value, OBJ_RANGE)

#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_NATIVE(value)       (((ObjNative*)AS_OBJ(value))->function)
#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))
#define AS_LIST(value)         ((ObjList*)AS_OBJ(value))
#define AS_DICT(value)         ((ObjDict*)AS_OBJ(value))
#define AS_CLASS(value)        ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))
#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_FILE(value)         ((ObjFile*)AS_OBJ(value))
#define AS_SOCKET(value)       ((ObjSocket*)AS_OBJ(value))
#define AS_RANGE(value)        ((ObjRange*)AS_OBJ(value))

typedef enum {
    OBJ_STRING,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_CLOSURE,
    OBJ_UPVALUE,
    OBJ_LIST,
    OBJ_DICT,
    OBJ_CLASS,
    OBJ_INSTANCE,
    OBJ_BOUND_METHOD,
    OBJ_FILE,
    OBJ_SOCKET,
    OBJ_DB,
    OBJ_RANGE,
} ObjType;

struct Obj {
    ObjType type;
    bool isMarked;
    struct Obj* next;
};

struct ObjString {
    Obj obj;
    int length;      // bytes (UTF-8)
    int charCount;   // characters (code points); == length for pure ASCII
    char* chars;
    uint32_t hash;
};

typedef struct {
    Obj obj;
    int arity;           // total parameters
    int requiredArity;   // parameters without a default value
    // Parameter names, in declaration order, length `arity`. Needed at
    // runtime because keyword arguments are matched in the VM (the callee is
    // only known then). These are ObjString* and MUST be GC-marked.
    ObjString** paramNames;
    int upvalueCount;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

typedef struct ObjUpvalue {
    Obj obj;
    Value* location;
    Value closed;
    struct ObjUpvalue* next;
} ObjUpvalue;

typedef struct {
    Obj obj;
    ObjFunction* function;
    ObjUpvalue** upvalues;
    int upvalueCount;
} ObjClosure;

typedef struct {
    Obj obj;
    ValueArray items;
} ObjList;

typedef struct {
    Obj obj;
    Table table;
    ValueArray keys;  // insertion order; deleted keys become kosong
} ObjDict;

// rentang(awal, akhir, langkah): a lazy arithmetic sequence (Python range).
// Elements are computed on demand; no list is allocated.
typedef struct {
    Obj obj;
    double start;
    double stop;
    double step;
} ObjRange;

typedef struct ObjClass {
    Obj obj;
    ObjString* name;
    struct ObjClass* superclass;
    Table methods;
} ObjClass;

typedef struct {
    Obj obj;
    ObjClass* klass;
    Table fields;
} ObjInstance;

typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure* method;
} ObjBoundMethod;

typedef struct {
    Obj obj;
    void* handle;        // FILE* (void* to keep <stdio.h> out of this header)
    ObjString* path;
    bool isOpen;
} ObjFile;

typedef struct {
    Obj obj;
    long long fd;        // OS socket descriptor (int on POSIX, SOCKET on Win)
    bool isOpen;
} ObjSocket;

typedef struct sqlite3 sqlite3;
typedef struct {
    Obj obj;
    sqlite3* db; // NULL after close; no VM-owned children.
} ObjDb;

ObjDb* newDb(void);
ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);
ObjClosure* newClosure(ObjFunction* function);
ObjUpvalue* newUpvalue(Value* slot);
ObjList* newList();
ObjDict* newDict();
int dictLength(ObjDict* dict);
bool dictSet(ObjDict* dict, ObjString* key, Value value);
bool dictDelete(ObjDict* dict, ObjString* key);
Value dictKeyAt(ObjDict* dict, int index);
ObjRange* newRange(double start, double stop, double step);
double rangeLength(ObjRange* range);
bool rangeContains(ObjRange* range, double value);
ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* klass);
ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method);
ObjFile* newFile(void* handle, ObjString* path);
ObjSocket* newSocket(long long fd);
ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
// Character-based (code point) access to UTF-8 strings. Lengths, indexes and
// substrings in the language count characters, not bytes.
int utf8ByteOffset(ObjString* string, int charIndex);
int utf8CharBytes(ObjString* string, int byteOffset);
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif
