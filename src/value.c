#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "memory.h"
#include "value.h"
#include "object.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values,
                                   oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "benar" : "salah");
            break;
        case VAL_KOSONG: printf("kosong"); break;
        case VAL_NUMBER: {
            char buf[32];
            numberToString(AS_NUMBER(value), buf, sizeof(buf));
            printf("%s", buf);
            break;
        }
        case VAL_OBJ: printObject(value); break;
    }
}

// Format a double the way the language prints numbers: whole values without a
// trailing ".0", and other values with the SHORTEST representation that reads
// back as the same double (Python's repr): 0.1 + 0.2 prints
// 0.30000000000000004, and 2 ** 0.5 prints 1.4142135623730951.
void numberToString(double value, char* buf, size_t size) {
    if (isnan(value)) { snprintf(buf, size, "nan"); return; }
    if (isinf(value)) { snprintf(buf, size, value < 0 ? "-takhingga" : "takhingga"); return; }

    // Integer-valued below 1e16 (where Python's repr switches to exponent
    // form): %.0f prints these doubles exactly. "+ 0.0" turns -0 into 0.
    if (value == floor(value) && fabs(value) < 1e16) {
        snprintf(buf, size, "%.0f", value + 0.0);
        return;
    }

    // 17 significant digits always round-trip; try fewer first.
    for (int precision = 1; precision <= 17; precision++) {
        snprintf(buf, size, "%.*g", precision, value);
        if (strtod(buf, NULL) == value) return;
    }
}

// Identity comparison: the semantics of `adalah` / `bukan adalah`.
// Objects are the same only when they are the same heap object; primitives
// compare by value. Strings are interned, so "a" adalah "a" is benar.
bool valuesIdentical(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_KOSONG: return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
        default:         return false; // Unreachable.
    }
}

// K7: '==' compares lists, dicts and ranges by STRUCTURE, recursively.
// Everything else (instances, functions, files, sockets) stays identity-based.
// Self-referential containers would recurse forever, so the depth is capped and
// anything deeper is reported as not equal rather than crashing the VM.
#define EQUAL_MAX_DEPTH 64

static bool valuesEqualDepth(Value a, Value b, int depth) {
    if (a.type != b.type) return false;
    if (!IS_OBJ(a)) return valuesIdentical(a, b);
    if (AS_OBJ(a) == AS_OBJ(b)) return true;      // same object: always equal
    if (OBJ_TYPE(a) != OBJ_TYPE(b)) return false;
    if (depth >= EQUAL_MAX_DEPTH) return false;   // ponytail: cap instead of cycle detection

    switch (OBJ_TYPE(a)) {
        case OBJ_LIST: {
            ValueArray* x = &AS_LIST(a)->items;
            ValueArray* y = &AS_LIST(b)->items;
            if (x->count != y->count) return false;
            for (int i = 0; i < x->count; i++) {
                if (!valuesEqualDepth(x->values[i], y->values[i], depth + 1)) return false;
            }
            return true;
        }
        case OBJ_DICT: {
            ObjDict* x = AS_DICT(a);
            ObjDict* y = AS_DICT(b);
            if (dictLength(x) != dictLength(y)) return false;
            // Insertion order does not affect equality: look every live key up.
            for (int i = 0; i < x->keys.count; i++) {
                Value key = x->keys.values[i];
                if (!IS_STRING(key)) continue;    // tombstone
                Value xv, yv;
                if (!tableGet(&x->table, AS_STRING(key), &xv)) continue;
                if (!tableGet(&y->table, AS_STRING(key), &yv)) return false;
                if (!valuesEqualDepth(xv, yv, depth + 1)) return false;
            }
            return true;
        }
        case OBJ_RANGE: {
            ObjRange* x = AS_RANGE(a);
            ObjRange* y = AS_RANGE(b);
            return x->start == y->start && x->stop == y->stop && x->step == y->step;
        }
        default:
            return false;   // strings are interned; other objects use identity
    }
}

bool valuesEqual(Value a, Value b) {
    return valuesEqualDepth(a, b, 0);
}
