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
        // Should be unreachable: the marker never escapes a call prologue.
        // Printing something recognisable beats printing garbage if it does.
        case VAL_TAKTERISI: printf("<takterisi>"); break;
        case VAL_INT: printf("%lld", (long long)AS_INT(value)); break;
        case VAL_NUMBER: {
            char buf[32];
            numberToString(AS_NUMBER(value), buf, sizeof(buf));
            printf("%s", buf);
            break;
        }
        case VAL_OBJ: printObject(value); break;
    }
}

// Format a double the way the language prints a desimal (K2): the SHORTEST
// representation that reads back as the same double (Python's repr), and
// always visibly a desimal — 3.0 prints "3.0", never "3". 0.1 + 0.2 prints
// 0.30000000000000004, and 2 ** 0.5 prints 1.4142135623730951.
void numberToString(double value, char* buf, size_t size) {
    if (isnan(value)) { snprintf(buf, size, "nan"); return; }
    if (isinf(value)) { snprintf(buf, size, value < 0 ? "-takhingga" : "takhingga"); return; }

    // Whole values below 1e16 (where Python's repr switches to exponent form)
    // print as digits plus ".0": 3.0 -> "3.0", 1200.0 -> "1200.0". %.g alone
    // would choose "1.2e+03" here, which Python never does.
    if (value == floor(value) && fabs(value) < 1e16) {
        snprintf(buf, size, "%.0f.0", value + 0.0);
        return;
    }

    // 17 significant digits always round-trip; try fewer first.
    for (int precision = 1; precision <= 17; precision++) {
        snprintf(buf, size, "%.*g", precision, value);
        if (strtod(buf, NULL) == value) break;
    }

    // %.g drops the fractional part for whole values ("3"), so add it back.
    if (strpbrk(buf, ".eE") == NULL) {
        size_t len = strlen(buf);
        if (len + 2 < size) {
            buf[len] = '.';
            buf[len + 1] = '0';
            buf[len + 2] = '\0';
        }
    }
}

// --- K2: int64 arithmetic -------------------------------------------------

#if defined(__GNUC__) || defined(__clang__)
#define INT_OVERFLOW_BUILTINS 1
#else
#define INT_OVERFLOW_BUILTINS 0
#endif

bool intAddOverflow(int64_t a, int64_t b, int64_t* out) {
#if INT_OVERFLOW_BUILTINS
    return __builtin_add_overflow(a, b, out);
#else
    if ((b > 0 && a > INT64_MAX - b) || (b < 0 && a < INT64_MIN - b)) return true;
    *out = a + b;
    return false;
#endif
}

bool intSubOverflow(int64_t a, int64_t b, int64_t* out) {
#if INT_OVERFLOW_BUILTINS
    return __builtin_sub_overflow(a, b, out);
#else
    if ((b < 0 && a > INT64_MAX + b) || (b > 0 && a < INT64_MIN + b)) return true;
    *out = a - b;
    return false;
#endif
}

bool intMulOverflow(int64_t a, int64_t b, int64_t* out) {
#if INT_OVERFLOW_BUILTINS
    return __builtin_mul_overflow(a, b, out);
#else
    if (a == 0 || b == 0) { *out = 0; return false; }
    if ((a == -1 && b == INT64_MIN) || (b == -1 && a == INT64_MIN)) return true;
    long double r = (long double)a * (long double)b;
    if (r > (long double)INT64_MAX || r < (long double)INT64_MIN) return true;
    *out = (int64_t)r;
    return false;
#endif
}

// Floor division, Python style. Returns false for a zero divisor and for
// INT64_MIN / -1 (which does not fit in int64); callers check the divisor
// first, so false means overflow here.
bool intFloorDiv(int64_t a, int64_t b, int64_t* out) {
    if (b == 0) return false;
    if (a == INT64_MIN && b == -1) return false;
    int64_t q = a / b;
    int64_t r = a % b;
    if (r != 0 && ((r < 0) != (b < 0))) q--;
    *out = q;
    return true;
}

// Modulo whose result takes the divisor's sign, like Python (-7 % 3 == 2).
bool intModulo(int64_t a, int64_t b, int64_t* out) {
    if (b == 0) return false;
    if (a == INT64_MIN && b == -1) { *out = 0; return true; }
    int64_t r = a % b;
    if (r != 0 && ((r < 0) != (b < 0))) r += b;
    *out = r;
    return true;
}

int64_t intPow(int64_t base, int64_t exp, bool* overflow) {
    int64_t result = 1;
    int64_t factor = base;
    *overflow = false;
    while (exp > 0) {
        if (exp & 1) {
            if (intMulOverflow(result, factor, &result)) { *overflow = true; return 0; }
        }
        exp >>= 1;
        if (exp > 0 && intMulOverflow(factor, factor, &factor)) { *overflow = true; return 0; }
    }
    return result;
}

// Compare an int64 with a double without losing precision. *out is -1, 0, 1,
// or 2 when the double is NaN (all comparisons with NaN are false).
static void compareIntDouble(int64_t i, double d, int* out) {
    if (isnan(d)) { *out = 2; return; }
    if (d >= 9223372036854775808.0) { *out = -1; return; }   // d > INT64_MAX
    if (d < -9223372036854775808.0) { *out = 1; return; }    // d < INT64_MIN
    double whole = floor(d);
    int64_t t = (int64_t)d;
    if (d != whole) {
        *out = (i <= t) ? -1 : 1;   // t is the integer part, so i <= t means i < d
        return;
    }
    *out = i < t ? -1 : i > t ? 1 : 0;
}

bool numberCompare(Value a, Value b, int* result) {
    if (!IS_NUMBER(a) || !IS_NUMBER(b)) return false;
    if (IS_INT(a) && IS_INT(b)) {
        int64_t x = AS_INT(a), y = AS_INT(b);
        *result = x < y ? -1 : x > y ? 1 : 0;
        return true;
    }
    if (IS_INT(a)) { compareIntDouble(AS_INT(a), b.as.number, result); return true; }
    if (IS_INT(b)) { compareIntDouble(AS_INT(b), a.as.number, result); *result = *result == 2 ? 2 : -*result; return true; }
    double x = asNumber(a), y = asNumber(b);
    if (isnan(x) || isnan(y)) { *result = 2; return true; }
    *result = x < y ? -1 : x > y ? 1 : 0;
    return true;
}

// Range bounds are stored as doubles, but a whole bound prints like a bulat:
// rentang(0, 3), not rentang(0.0, 3.0) — the arguments may well have been bulat.
void rangeNumberToString(double value, char* buf, size_t size) {
    if (value == floor(value) && fabs(value) < 9.2e18) {
        snprintf(buf, size, "%.0f", value);
        return;
    }
    numberToString(value, buf, size);
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
        case VAL_INT:    return AS_INT(a) == AS_INT(b);
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
    // K2: 1 == 1.0 must be true even though the types differ (Python).
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        int cmp;
        return numberCompare(a, b, &cmp) && cmp == 0;
    }
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
