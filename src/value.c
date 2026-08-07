#include <stdio.h>
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
// trailing ".0", and other values with enough precision to round-trip while
// avoiding accidental scientific notation for ordinary magnitudes.
void numberToString(double value, char* buf, size_t size) {
    if (isnan(value)) { snprintf(buf, size, "nan"); return; }
    if (isinf(value)) { snprintf(buf, size, value < 0 ? "-takhingga" : "takhingga"); return; }

    // Integer-valued and within a range that %.0f prints exactly.
    if (value == floor(value) && fabs(value) < 1e15) {
        snprintf(buf, size, "%.0f", value);
        return;
    }

    // Non-integer: use up to 15 significant digits, then trim trailing zeros.
    snprintf(buf, size, "%.15g", value);
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
