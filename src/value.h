#ifndef indonesiaku_value_h
#define indonesiaku_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
    VAL_BOOL,
    VAL_KOSONG,  // None/null
    // Internal only: marks a parameter slot the caller did not fill, so the
    // function prologue knows which defaults to evaluate. Never reachable
    // from user code — call() rejects missing required parameters first, and
    // every optional slot is overwritten by its default.
    VAL_TAKTERISI,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_KOSONG(value)  ((value).type == VAL_KOSONG)
#define IS_TAKTERISI(value) ((value).type == VAL_TAKTERISI)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJ(value)     ((value).type == VAL_OBJ)

#define AS_OBJ(value)     ((value).as.obj)
#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define KOSONG_VAL        ((Value){VAL_KOSONG, {.number = 0}})
#define TAKTERISI_VAL     ((Value){VAL_TAKTERISI, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object)   ((Value){VAL_OBJ, {.obj = (Obj*)object}})

typedef struct {
    int capacity;
    int count;
    Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
bool valuesIdentical(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);
void numberToString(double value, char* buf, size_t size);

#endif
