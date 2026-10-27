#ifndef indonesiaku_object_h
#define indonesiaku_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)

#define IS_STRING(value)       isObjType(value, OBJ_STRING)
#define IS_ITERATOR(value)     isObjType(value, OBJ_ITERATOR)
#define IS_MODULE(value)       isObjType(value, OBJ_MODULE)
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
#define IS_TUPLE(value)        isObjType(value, OBJ_TUPLE)
#define IS_SET(value)          isObjType(value, OBJ_SET)

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
#define AS_TUPLE(value)        ((ObjTuple*)AS_OBJ(value))
#define AS_SET(value)          ((ObjSet*)AS_OBJ(value))
#define AS_ITERATOR(value)     ((ObjIterator*)AS_OBJ(value))
#define AS_MODULE(value)       ((ObjModule*)AS_OBJ(value))

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
    OBJ_TUPLE,
    OBJ_SET,
    OBJ_ITERATOR,
    OBJ_MODULE,
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

typedef struct ObjModule ObjModule;

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
    // The module this function was compiled in. A function defined in module A
    // reads A's globals even when called from B, so global resolution follows
    // the callee, never the caller. GC-marked.
    struct ObjModule* module;
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

// tupel (K17): an immutable sequence. Same layout as ObjList on purpose, so
// every read path can share sequenceItems(); only writes tell them apart.
typedef struct {
    Obj obj;
    ValueArray items;
} ObjTuple;

typedef struct {
    Obj obj;
    ValueTable table;   // K8: any hashable key, not just strings
    ValueArray keys;    // insertion order; a deleted key is removed from it
} ObjDict;

// himpunan (K17): members are the keys of a dict-shaped table (values unused).
// The layout is IDENTICAL to ObjDict on purpose, so the tested dict helpers
// (dictSet, dictDelete, dictLength, dictKeyAt) work on it via setAsDict().
typedef struct {
    Obj obj;
    ValueTable table;
    ValueArray keys;    // insertion order of members
    bool frozen;        // himpunan_beku: hashable, and every write path refuses it
} ObjSet;

// Only the leading fields must line up: setAsDict() hands an ObjSet to the
// dict helpers, which touch nothing past `keys`. The sizes differ (ObjSet has
// `frozen`), and that is fine -- no dict is ever allocated from an ObjSet.
_Static_assert(offsetof(ObjSet, table) == offsetof(ObjDict, table) &&
               offsetof(ObjSet, keys) == offsetof(ObjDict, keys) &&
               sizeof(ObjSet) >= sizeof(ObjDict),
               "ObjSet must keep ObjDict's leading layout (setAsDict casts between them)");

// iter(urutan): an explicit cursor over a snapshot-free sequence. `index` is
// the next element to hand out; the length is re-read every lanjutkan(), so
// shrinking the source ends iteration early instead of reading out of bounds.
typedef struct {
    Obj obj;
    Value source;
    int index;
} ObjIterator;

// A module owns its global namespace. EVERY execution runs inside one,
// including the entry script (whose module is named "__utama__"), so global
// lookup never needs a special case for "no module".
struct ObjModule {
    Obj obj;
    ObjString* name;     // "matematika", or "__utama__" for the entry script
    ObjString* path;     // resolved absolute path, the cache key
    Table globals;       // this module's own globals; builtins live in vm.globals
};

// rentang(awal, akhir, langkah): a lazy arithmetic sequence (Python range).
// Elements are computed on demand; no list is allocated.
typedef struct {
    Obj obj;
    double start;
    double stop;
    double step;
} ObjRange;

// Magic methods (K19). The VM interns their names once (vm.magicNames) and
// every class keeps a bitmask of the ones it defines, so an operator on an
// instance tests one bit before it ever searches the method table.
typedef enum {
    MAGIC_TEKS,          // _teks_         cetak, ke_teks, f-string
    MAGIC_REPR,          // _repr_         repr, elements inside a collection
    MAGIC_SAMA,          // _sama_         ==, dalam, == inside collections
    MAGIC_TIDAK_SAMA,    // _tidak_sama_   !=
    MAGIC_KURANG_DARI,   // _kurang_dari_  <
    MAGIC_KURANG_SAMA,   // _kurang_sama_  <=
    MAGIC_LEBIH_DARI,    // _lebih_dari_   >
    MAGIC_LEBIH_SAMA,    // _lebih_sama_   >=
    MAGIC_TAMBAH,        // _tambah_       +
    MAGIC_KURANGI,       // _kurangi_      -
    MAGIC_KALI,          // _kali_         *
    MAGIC_BAGI,          // _bagi_         /
    MAGIC_BAGI_BULAT,    // _bagi_bulat_   //
    MAGIC_SISA,          // _sisa_         %
    MAGIC_PANGKAT,       // _pangkat_      **
    MAGIC_NEGATIF,       // _negatif_      -x
    MAGIC_PANJANG,       // _panjang_      panjang(), truthiness fallback
    MAGIC_LOGIKA,        // _logika_       jika, selagi, bukan, dan/atau
    MAGIC_AMBIL,         // _ambil_        objek[kunci]
    MAGIC_ATUR,          // _atur_         objek[kunci] = nilai
    MAGIC_HASH,          // _hash_         kunci kamus / anggota himpunan
    // Cerminan aritmetika: dipakai saat objek ada di KANAN operator.
    MAGIC_KTAMBAH,       // _ktambah_      nilai + objek
    MAGIC_KKURANGI,      // _kkurangi_     nilai - objek
    MAGIC_KKALI,         // _kkali_        nilai * objek
    MAGIC_KBAGI,         // _kbagi_        nilai / objek
    MAGIC_KBAGI_BULAT,   // _kbagi_bulat_  nilai // objek
    MAGIC_KSISA,         // _ksisa_        nilai % objek
    MAGIC_KPANGKAT,      // _kpangkat_     nilai ** objek
    MAGIC_ITER,          // _iter_         untuk ... dalam objek
    MAGIC_PANGGIL,       // _panggil_      objek(...)
    MAGIC_COUNT,
    MAGIC_NONE = MAGIC_COUNT
} MagicKind;

#define MAGIC_BIT(kind) (1u << (kind))

typedef struct ObjClass {
    Obj obj;
    ObjString* name;
    struct ObjClass* superclass;
    Table methods;
    uint32_t magic;      // MAGIC_BIT of every magic method in `methods`
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
ObjTuple* newTuple();
ObjDict* newDict();
ObjSet* newSet();
ObjIterator* newIterator(Value source);
ObjModule* newModule(ObjString* name, ObjString* path);
// Adds value; false (nothing added) when it is not hashable. Caller roots both.
bool setAdd(ObjSet* set, Value value);
int dictLength(ObjDict* dict);
bool dictSet(ObjDict* dict, Value key, Value value);
bool dictDelete(ObjDict* dict, Value key);
Value dictKeyAt(ObjDict* dict, int index);
ObjRange* newRange(double start, double stop, double step);
double rangeLength(ObjRange* range);
bool rangeContains(ObjRange* range, double value);
// K2: element `index` of a rentang, as a bulat when the arithmetic stays
// whole (a rentang of bulat yields bulat, like Python's range).
Value rangeElement(ObjRange* range, int index);
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
// Escape sequence for a character inside a quoted string, NULL if none is
// needed. Shared by every path that prints a string inside a collection.
const char* stringEscape(char c);
void printObject(Value value);

// Growable text buffer for formatValue. Plain malloc, not the GC heap: user
// code (_teks_/_repr_) may run while a buffer is half built.
typedef struct {
    char* chars;
    int length;
    int capacity;
} TextBuf;

void initTextBuf(TextBuf* buf);
void freeTextBuf(TextBuf* buf);
void textAppend(TextBuf* buf, const char* chars, int length);
void textAppendCString(TextBuf* buf, const char* chars);

// _teks_/_repr_ for an instance (K19), installed by the VM: object.c cannot
// run Indonesiaku code itself. Returns 1 when it appended text, 0 when the
// class has no such method, -1 when the method raised (error is pending).
typedef int (*FormatInstanceFn)(TextBuf* out, Value instance, bool repr);
extern FormatInstanceFn formatInstanceHook;

// The ONE formatter behind cetak, ke_teks, repr, format and f-strings, so
// they can never drift apart. `repr` quotes strings (elements of a collection
// always are). `callMagic` lets instances use _teks_/_repr_; debug and GC
// logs pass false so they never run user code. Returns false only when a
// magic method raised.
bool formatValue(TextBuf* out, Value value, bool repr, bool callMagic);

static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

// Items of a daftar or tupel, for READ-only paths (index, slice, iterate,
// unpack, compare, hash). NULL for anything else. Never write through this
// pointer: a tupel must stay immutable.
// A himpunan viewed as the dict it is laid out like (see ObjSet).
static inline ObjDict* setAsDict(ObjSet* set) {
    return (ObjDict*)set;
}

static inline ValueArray* sequenceItems(Value value) {
    if (IS_LIST(value)) return &AS_LIST(value)->items;
    if (IS_TUPLE(value)) return &AS_TUPLE(value)->items;
    return NULL;
}

#endif
