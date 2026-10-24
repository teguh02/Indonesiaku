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

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;
    switch (a.type) {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_KOSONG: return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:    return AS_OBJ(a) == AS_OBJ(b);
        default:         return false; // Unreachable.
    }
}
