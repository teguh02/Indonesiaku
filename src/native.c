#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include "native.h"
#include "vm.h"
#include "object.h"
#include "memory.h"
#include "common.h"

// ============================================================================
// FUNGSI MATEMATIKA - IMPLEMENTASI
// ============================================================================

Value nativeFnMin(int argCount, Value* args) {
    if (argCount < 2) {
        runtimeError("min() memerlukan minimal 2 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("min() hanya menerima angka");
        return NUMBER_VAL(0);
    }

    double min = AS_NUMBER(args[0]);
    for (int i = 1; i < argCount; i++) {
        if (!IS_NUMBER(args[i])) {
            runtimeError("min() hanya menerima angka");
            return NUMBER_VAL(0);
        }
        double num = AS_NUMBER(args[i]);
        if (num < min) min = num;
    }
    return NUMBER_VAL(min);
}

Value nativeFnMax(int argCount, Value* args) {
    if (argCount < 2) {
        runtimeError("max() memerlukan minimal 2 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("max() hanya menerima angka");
        return NUMBER_VAL(0);
    }

    double max = AS_NUMBER(args[0]);
    for (int i = 1; i < argCount; i++) {
        if (!IS_NUMBER(args[i])) {
            runtimeError("max() hanya menerima angka");
            return NUMBER_VAL(0);
        }
        double num = AS_NUMBER(args[i]);
        if (num > max) max = num;
    }
    return NUMBER_VAL(max);
}

Value nativeFnAbs(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("abs() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("abs() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

Value nativeFnAkar(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("akar() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("akar() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    double num = AS_NUMBER(args[0]);
    if (num < 0) {
        runtimeError("akar() tidak dapat menerima angka negatif");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(sqrt(num));
}

Value nativeFnPangkat(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("pangkat() memerlukan 2 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        runtimeError("pangkat() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(pow(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

Value nativeFnBulat(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("bulat() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("bulat() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

Value nativeFnLantai(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("lantai() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("lantai() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

Value nativeFnAtap(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("atap() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("atap() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

Value nativeFnAcak(int argCount, Value* args) {
    if (argCount != 0) {
        runtimeError("acak() tidak memerlukan argumen");
        return NUMBER_VAL(0);
    }
    // Return random number between 0 and 1
    return NUMBER_VAL((double)rand() / RAND_MAX);
}

// ============================================================================
// FUNGSI STRING - IMPLEMENTASI
// ============================================================================

Value nativeFnPanjang(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("panjang() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (IS_STRING(args[0])) {
        return NUMBER_VAL(AS_STRING(args[0])->length);
    }
    if (IS_LIST(args[0])) {
        return NUMBER_VAL(AS_LIST(args[0])->items.count);
    }
    if (IS_DICT(args[0])) {
        // table.count includes tombstones; count live entries instead.
        ObjDict* dict = AS_DICT(args[0]);
        int n = 0;
        for (int i = 0; i < dict->table.capacity; i++) {
            if (dict->table.entries[i].key != NULL) n++;
        }
        return NUMBER_VAL(n);
    }
    runtimeError("panjang() hanya menerima string, list, atau kamus");
    return NUMBER_VAL(0);
}

Value nativeFnTambah(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("tambah() memerlukan 2 argumen (list, nilai)");
        return KOSONG_VAL;
    }
    if (!IS_LIST(args[0])) {
        runtimeError("tambah() argumen pertama harus list");
        return KOSONG_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    writeValueArray(&list->items, args[1]);
    return args[0];
}

Value nativeFnHapus(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("hapus() memerlukan 1 argumen (list)");
        return KOSONG_VAL;
    }
    if (!IS_LIST(args[0])) {
        runtimeError("hapus() hanya menerima list");
        return KOSONG_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    if (list->items.count == 0) {
        runtimeError("hapus() pada list kosong");
        return KOSONG_VAL;
    }
    Value last = list->items.values[list->items.count - 1];
    list->items.count--;
    return last;
}

// ============================================================================
// FUNGSI KAMUS (DICTIONARY) - IMPLEMENTASI
// ============================================================================

Value nativeFnKamus(int argCount, Value* args) {
    (void)args;
    if (argCount != 0) {
        runtimeError("kamus() tidak menerima argumen");
        return KOSONG_VAL;
    }
    return OBJ_VAL(newDict());
}

Value nativeFnPunya(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("punya() memerlukan 2 argumen (kamus, kunci)");
        return BOOL_VAL(false);
    }
    if (!IS_DICT(args[0]) || !IS_STRING(args[1])) {
        runtimeError("punya() memerlukan (kamus, string)");
        return BOOL_VAL(false);
    }
    Value tmp;
    return BOOL_VAL(tableGet(&AS_DICT(args[0])->table, AS_STRING(args[1]), &tmp));
}

Value nativeFnHapusKunci(int argCount, Value* args) {
    if (argCount != 2) {
        runtimeError("hapus_kunci() memerlukan 2 argumen (kamus, kunci)");
        return BOOL_VAL(false);
    }
    if (!IS_DICT(args[0]) || !IS_STRING(args[1])) {
        runtimeError("hapus_kunci() memerlukan (kamus, string)");
        return BOOL_VAL(false);
    }
    return BOOL_VAL(tableDelete(&AS_DICT(args[0])->table, AS_STRING(args[1])));
}

Value nativeFnKunci(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("kunci() memerlukan 1 argumen (kamus)");
        return KOSONG_VAL;
    }
    if (!IS_DICT(args[0])) {
        runtimeError("kunci() hanya menerima kamus");
        return KOSONG_VAL;
    }
    ObjDict* dict = AS_DICT(args[0]);
    ObjList* keys = newList();
    // Protect the new list from GC while we populate it.
    push(OBJ_VAL(keys));
    for (int i = 0; i < dict->table.capacity; i++) {
        Entry* entry = &dict->table.entries[i];
        if (entry->key != NULL) {
            writeValueArray(&keys->items, OBJ_VAL(entry->key));
        }
    }
    pop();
    return OBJ_VAL(keys);
}

Value nativeFnHurofBesar(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("huruf_besar() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0])) {
        runtimeError("huruf_besar() hanya menerima string");
        return KOSONG_VAL;
    }

    ObjString* original = AS_STRING(args[0]);
    char* result = ALLOCATE(char, original->length + 1);
    
    for (int i = 0; i < original->length; i++) {
        result[i] = toupper(original->chars[i]);
    }
    result[original->length] = '\0';

    ObjString* str = takeString(result, original->length);
    return OBJ_VAL(str);
}

Value nativeFnHurofKecil(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("huruf_kecil() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0])) {
        runtimeError("huruf_kecil() hanya menerima string");
        return KOSONG_VAL;
    }

    ObjString* original = AS_STRING(args[0]);
    char* result = ALLOCATE(char, original->length + 1);
    
    for (int i = 0; i < original->length; i++) {
        result[i] = tolower(original->chars[i]);
    }
    result[original->length] = '\0';

    ObjString* str = takeString(result, original->length);
    return OBJ_VAL(str);
}

Value nativeFnGanti(int argCount, Value* args) {
    if (argCount != 3) {
        runtimeError("ganti() memerlukan 3 argumen");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0]) || !IS_STRING(args[1]) || !IS_STRING(args[2])) {
        runtimeError("ganti() hanya menerima string");
        return KOSONG_VAL;
    }

    ObjString* text = AS_STRING(args[0]);
    ObjString* cari = AS_STRING(args[1]);
    ObjString* ganti = AS_STRING(args[2]);

    if (cari->length == 0) {
        return OBJ_VAL(text);
    }

    // Hitung jumlah occurrence
    int count = 0;
    const char* p = text->chars;
    while ((p = strstr(p, cari->chars)) != NULL) {
        count++;
        p += cari->length;
    }

    if (count == 0) {
        return OBJ_VAL(text);
    }

    // Alokasi memory untuk hasil
    int newLength = text->length + count * (ganti->length - cari->length);
    char* result = ALLOCATE(char, newLength + 1);
    
    // Replace
    const char* src = text->chars;
    char* dst = result;
    while (*src) {
        if (strncmp(src, cari->chars, cari->length) == 0) {
            memcpy(dst, ganti->chars, ganti->length);
            dst += ganti->length;
            src += cari->length;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    ObjString* str = takeString(result, newLength);
    return OBJ_VAL(str);
}

// ============================================================================
// FUNGSI UTILITAS - IMPLEMENTASI
// ============================================================================

Value nativeFnJenis(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("jenis() memerlukan 1 argumen");
        return OBJ_VAL(copyString("kosong", 6));
    }

    if (IS_KOSONG(args[0])) {
        return OBJ_VAL(copyString("kosong", 6));
    } else if (IS_BOOL(args[0])) {
        return OBJ_VAL(copyString("boolean", 7));
    } else if (IS_NUMBER(args[0])) {
        return OBJ_VAL(copyString("angka", 5));
    } else if (IS_STRING(args[0])) {
        return OBJ_VAL(copyString("string", 6));
    } else if (IS_LIST(args[0])) {
        return OBJ_VAL(copyString("list", 4));
    } else if (IS_DICT(args[0])) {
        return OBJ_VAL(copyString("kamus", 5));
    } else if (IS_INSTANCE(args[0])) {
        return OBJ_VAL(copyString("objek", 5));
    } else if (IS_CLASS(args[0])) {
        return OBJ_VAL(copyString("kelas", 5));
    } else if (IS_OBJ(args[0])) {
        return OBJ_VAL(copyString("object", 6));
    }
    return OBJ_VAL(copyString("tidak diketahui", 15));
}

Value nativeFnInput(int argCount, Value* args) {
    if (argCount > 1) {
        runtimeError("input() menerima maksimal 1 argumen");
        return KOSONG_VAL;
    }

    // Print prompt jika ada
    if (argCount == 1) {
        if (!IS_STRING(args[0])) {
            runtimeError("prompt harus berupa string");
            return KOSONG_VAL;
        }
        ObjString* prompt = AS_STRING(args[0]);
        printf("%s", prompt->chars);
        fflush(stdout);
    }

    // Read input
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return KOSONG_VAL;
    }

    // Remove newline
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
        len--;
    }

    return OBJ_VAL(copyString(buffer, len));
}

Value nativeFnHenti(int argCount, Value* args) {
    if (argCount != 1) {
        runtimeError("henti() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        runtimeError("henti() hanya menerima angka (detik)");
        return KOSONG_VAL;
    }

    double seconds = AS_NUMBER(args[0]);
    if (seconds < 0) {
        runtimeError("henti() tidak boleh negatif");
        return KOSONG_VAL;
    }

#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#else
    usleep((unsigned int)(seconds * 1000000));
#endif

    return KOSONG_VAL;
}
