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
        nativeRaise("min() memerlukan minimal 2 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("min() hanya menerima angka");
        return NUMBER_VAL(0);
    }

    double min = AS_NUMBER(args[0]);
    for (int i = 1; i < argCount; i++) {
        if (!IS_NUMBER(args[i])) {
            nativeRaise("min() hanya menerima angka");
            return NUMBER_VAL(0);
        }
        double num = AS_NUMBER(args[i]);
        if (num < min) min = num;
    }
    return NUMBER_VAL(min);
}

Value nativeFnMax(int argCount, Value* args) {
    if (argCount < 2) {
        nativeRaise("max() memerlukan minimal 2 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("max() hanya menerima angka");
        return NUMBER_VAL(0);
    }

    double max = AS_NUMBER(args[0]);
    for (int i = 1; i < argCount; i++) {
        if (!IS_NUMBER(args[i])) {
            nativeRaise("max() hanya menerima angka");
            return NUMBER_VAL(0);
        }
        double num = AS_NUMBER(args[i]);
        if (num > max) max = num;
    }
    return NUMBER_VAL(max);
}

Value nativeFnAbs(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("abs() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("abs() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(fabs(AS_NUMBER(args[0])));
}

Value nativeFnAkar(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("akar() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("akar() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    double num = AS_NUMBER(args[0]);
    if (num < 0) {
        nativeRaise("akar() tidak dapat menerima angka negatif");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(sqrt(num));
}

Value nativeFnPangkat(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("pangkat() memerlukan 2 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        nativeRaise("pangkat() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(pow(AS_NUMBER(args[0]), AS_NUMBER(args[1])));
}

Value nativeFnBulat(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("bulat() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("bulat() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(round(AS_NUMBER(args[0])));
}

Value nativeFnLantai(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("lantai() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("lantai() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(floor(AS_NUMBER(args[0])));
}

Value nativeFnAtap(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("atap() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("atap() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(ceil(AS_NUMBER(args[0])));
}

Value nativeFnAcak(int argCount, Value* args) {
    if (argCount != 0) {
        nativeRaise("acak() tidak memerlukan argumen");
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
        nativeRaise("panjang() memerlukan 1 argumen");
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
    nativeRaise("panjang() hanya menerima string, list, atau kamus");
    return NUMBER_VAL(0);
}

Value nativeFnTambah(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("tambah() memerlukan 2 argumen (list, nilai)");
        return KOSONG_VAL;
    }
    if (!IS_LIST(args[0])) {
        nativeRaise("tambah() argumen pertama harus list");
        return KOSONG_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    writeValueArray(&list->items, args[1]);
    return args[0];
}

Value nativeFnHapus(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("hapus() memerlukan 1 argumen (list)");
        return KOSONG_VAL;
    }
    if (!IS_LIST(args[0])) {
        nativeRaise("hapus() hanya menerima list");
        return KOSONG_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    if (list->items.count == 0) {
        nativeRaise("hapus() pada list kosong");
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
        nativeRaise("kamus() tidak menerima argumen");
        return KOSONG_VAL;
    }
    return OBJ_VAL(newDict());
}

Value nativeFnPunya(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("punya() memerlukan 2 argumen (kamus, kunci)");
        return BOOL_VAL(false);
    }
    if (!IS_DICT(args[0]) || !IS_STRING(args[1])) {
        nativeRaise("punya() memerlukan (kamus, string)");
        return BOOL_VAL(false);
    }
    Value tmp;
    return BOOL_VAL(tableGet(&AS_DICT(args[0])->table, AS_STRING(args[1]), &tmp));
}

Value nativeFnHapusKunci(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("hapus_kunci() memerlukan 2 argumen (kamus, kunci)");
        return BOOL_VAL(false);
    }
    if (!IS_DICT(args[0]) || !IS_STRING(args[1])) {
        nativeRaise("hapus_kunci() memerlukan (kamus, string)");
        return BOOL_VAL(false);
    }
    return BOOL_VAL(tableDelete(&AS_DICT(args[0])->table, AS_STRING(args[1])));
}

Value nativeFnKunci(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("kunci() memerlukan 1 argumen (kamus)");
        return KOSONG_VAL;
    }
    if (!IS_DICT(args[0])) {
        nativeRaise("kunci() hanya menerima kamus");
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
        nativeRaise("huruf_besar() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("huruf_besar() hanya menerima string");
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
        nativeRaise("huruf_kecil() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("huruf_kecil() hanya menerima string");
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
        nativeRaise("ganti() memerlukan 3 argumen");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0]) || !IS_STRING(args[1]) || !IS_STRING(args[2])) {
        nativeRaise("ganti() hanya menerima string");
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
        nativeRaise("jenis() memerlukan 1 argumen");
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
    } else if (IS_FILE(args[0])) {
        return OBJ_VAL(copyString("berkas", 6));
    } else if (IS_OBJ(args[0])) {
        return OBJ_VAL(copyString("object", 6));
    }
    return OBJ_VAL(copyString("tidak diketahui", 15));
}

Value nativeFnInput(int argCount, Value* args) {
    if (argCount > 1) {
        nativeRaise("input() menerima maksimal 1 argumen");
        return KOSONG_VAL;
    }

    // Print prompt jika ada
    if (argCount == 1) {
        if (!IS_STRING(args[0])) {
            nativeRaise("prompt harus berupa string");
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
        nativeRaise("henti() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("henti() hanya menerima angka (detik)");
        return KOSONG_VAL;
    }

    double seconds = AS_NUMBER(args[0]);
    if (seconds < 0) {
        nativeRaise("henti() tidak boleh negatif");
        return KOSONG_VAL;
    }

#ifdef _WIN32
    Sleep((DWORD)(seconds * 1000));
#else
    usleep((unsigned int)(seconds * 1000000));
#endif

    return KOSONG_VAL;
}

// ============================================================================
// FUNGSI KONVERSI & FORMAT - IMPLEMENTASI
// ============================================================================

// Append a value's text representation into buf at *pos (bounded by size).
static void appendValueText(Value v, char* buf, size_t size, size_t* pos) {
    char tmp[64];
    const char* s = NULL;
    int len = 0;

    if (IS_BOOL(v)) {
        s = AS_BOOL(v) ? "benar" : "salah";
    } else if (IS_KOSONG(v)) {
        s = "kosong";
    } else if (IS_NUMBER(v)) {
        numberToString(AS_NUMBER(v), tmp, sizeof(tmp));
        s = tmp;
    } else if (IS_STRING(v)) {
        ObjString* str = AS_STRING(v);
        s = str->chars;
        len = str->length;
    } else {
        s = "<objek>";
    }

    if (len == 0) len = (int)strlen(s);
    for (int i = 0; i < len && *pos + 1 < size; i++) {
        buf[(*pos)++] = s[i];
    }
    buf[*pos] = '\0';
}

Value nativeFnKeAngka(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("ke_angka() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (IS_NUMBER(args[0])) return args[0];
    if (!IS_STRING(args[0])) {
        nativeRaise("ke_angka() hanya menerima string atau angka");
        return NUMBER_VAL(0);
    }
    ObjString* str = AS_STRING(args[0]);
    char* end = NULL;
    double v = strtod(str->chars, &end);
    // Reject empty input or trailing non-numeric garbage.
    if (end == str->chars || *end != '\0') {
        nativeRaise("ke_angka() gagal: '%s' bukan angka valid", str->chars);
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(v);
}

Value nativeFnKeTeks(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("ke_teks() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    char buf[512];
    size_t pos = 0;
    buf[0] = '\0';
    appendValueText(args[0], buf, sizeof(buf), &pos);
    return OBJ_VAL(copyString(buf, (int)pos));
}

// format(pola, ...) - ganti setiap '{}' di pola dengan argumen berikutnya.
Value nativeFnFormat(int argCount, Value* args) {
    if (argCount < 1) {
        nativeRaise("format() memerlukan minimal 1 argumen (pola)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("format() argumen pertama harus string pola");
        return KOSONG_VAL;
    }

    ObjString* pola = AS_STRING(args[0]);
    char buf[1024];
    size_t pos = 0;
    buf[0] = '\0';
    int nextArg = 1;

    for (int i = 0; i < pola->length && pos + 1 < sizeof(buf); i++) {
        if (pola->chars[i] == '{' && i + 1 < pola->length &&
            pola->chars[i + 1] == '}') {
            if (nextArg < argCount) {
                appendValueText(args[nextArg++], buf, sizeof(buf), &pos);
            } else {
                // Not enough args: leave the placeholder literally.
                if (pos + 2 < sizeof(buf)) { buf[pos++] = '{'; buf[pos++] = '}'; }
            }
            i++; // skip '}'
        } else {
            buf[pos++] = pola->chars[i];
        }
        buf[pos] = '\0';
    }

    return OBJ_VAL(copyString(buf, (int)pos));
}

// acak_bulat(min, maks) - bilangan bulat acak dalam [min, maks].
Value nativeFnAcakBulat(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("acak_bulat() memerlukan 2 argumen (min, maks)");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        nativeRaise("acak_bulat() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    long lo = (long)AS_NUMBER(args[0]);
    long hi = (long)AS_NUMBER(args[1]);
    if (lo > hi) {
        nativeRaise("acak_bulat(): min tidak boleh lebih besar dari maks");
        return NUMBER_VAL(0);
    }
    long range = hi - lo + 1;
    return NUMBER_VAL((double)(lo + (rand() % range)));
}

// ============================================================================
// FUNGSI BERKAS (FILE I/O) - IMPLEMENTASI
// Semua kegagalan memakai nativeRaise sehingga dapat ditangkap coba/kecuali.
// ============================================================================

// buka_berkas(path, mode) - Buka berkas, kembalikan handle.
// mode: "b" (baca), "t" (tulis), "s" (sambung/append).
Value nativeFnBukaBerkas(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("buka_berkas() memerlukan 2 argumen (path, mode)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        nativeRaise("buka_berkas() memerlukan (string, string)");
        return KOSONG_VAL;
    }

    ObjString* path = AS_STRING(args[0]);
    const char* m = AS_STRING(args[1])->chars;
    const char* cmode;
    if (strcmp(m, "b") == 0)      cmode = "rb";
    else if (strcmp(m, "t") == 0) cmode = "wb";
    else if (strcmp(m, "s") == 0) cmode = "ab";
    else {
        nativeRaise("buka_berkas() mode tidak dikenal '%s' (pakai \"b\", \"t\", atau \"s\")", m);
        return KOSONG_VAL;
    }

    FILE* fp = fopen(path->chars, cmode);
    if (fp == NULL) {
        nativeRaise("buka_berkas() gagal membuka '%s'", path->chars);
        return KOSONG_VAL;
    }
    return OBJ_VAL(newFile(fp, path));
}

Value nativeFnTutup(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("tutup() memerlukan 1 argumen (berkas)");
        return KOSONG_VAL;
    }
    if (!IS_FILE(args[0])) {
        nativeRaise("tutup() hanya menerima berkas");
        return KOSONG_VAL;
    }
    ObjFile* file = AS_FILE(args[0]);
    if (file->isOpen && file->handle != NULL) {
        fclose((FILE*)file->handle);
        file->handle = NULL;
        file->isOpen = false;
    }
    return KOSONG_VAL;
}

Value nativeFnBaca(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("baca() memerlukan 1 argumen (berkas)");
        return KOSONG_VAL;
    }
    if (!IS_FILE(args[0])) {
        nativeRaise("baca() hanya menerima berkas");
        return KOSONG_VAL;
    }
    ObjFile* file = AS_FILE(args[0]);
    if (!file->isOpen || file->handle == NULL) {
        nativeRaise("baca() pada berkas yang sudah tertutup");
        return KOSONG_VAL;
    }

    FILE* fp = (FILE*)file->handle;
    long cur = ftell(fp);
    fseek(fp, 0L, SEEK_END);
    long end = ftell(fp);
    fseek(fp, cur, SEEK_SET);
    long remaining = end - cur;
    if (remaining < 0) remaining = 0;

    char* buffer = ALLOCATE(char, remaining + 1);
    size_t read = fread(buffer, 1, (size_t)remaining, fp);
    buffer[read] = '\0';
    ObjString* result = takeString(buffer, (int)read);
    return OBJ_VAL(result);
}

Value nativeFnBacaBaris(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("baca_baris() memerlukan 1 argumen (berkas)");
        return KOSONG_VAL;
    }
    if (!IS_FILE(args[0])) {
        nativeRaise("baca_baris() hanya menerima berkas");
        return KOSONG_VAL;
    }
    ObjFile* file = AS_FILE(args[0]);
    if (!file->isOpen || file->handle == NULL) {
        nativeRaise("baca_baris() pada berkas yang sudah tertutup");
        return KOSONG_VAL;
    }

    FILE* fp = (FILE*)file->handle;
    int cap = 128, len = 0;
    char* buffer = ALLOCATE(char, cap);
    int c;
    bool any = false;
    while ((c = fgetc(fp)) != EOF) {
        any = true;
        if (c == '\n') break;
        if (len + 1 >= cap) {
            int oldCap = cap;
            cap *= 2;
            buffer = GROW_ARRAY(char, buffer, oldCap, cap);
        }
        buffer[len++] = (char)c;
    }
    if (!any && len == 0) {
        // EOF with nothing read -> kosong signals end of file.
        FREE_ARRAY(char, buffer, cap);
        return KOSONG_VAL;
    }
    // Strip a trailing '\r' (Windows line endings).
    if (len > 0 && buffer[len - 1] == '\r') len--;
    buffer[len] = '\0';
    ObjString* result = takeString(buffer, len);
    // takeString may free `buffer` if interned; the capacity we allocated is
    // len+1..cap, so shrink accounting is not tracked precisely here (small
    // transient). Acceptable for a line reader.
    return OBJ_VAL(result);
}

Value nativeFnTulis(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("tulis() memerlukan 2 argumen (berkas, teks)");
        return KOSONG_VAL;
    }
    if (!IS_FILE(args[0]) || !IS_STRING(args[1])) {
        nativeRaise("tulis() memerlukan (berkas, string)");
        return KOSONG_VAL;
    }
    ObjFile* file = AS_FILE(args[0]);
    if (!file->isOpen || file->handle == NULL) {
        nativeRaise("tulis() pada berkas yang sudah tertutup");
        return KOSONG_VAL;
    }
    ObjString* text = AS_STRING(args[1]);
    size_t written = fwrite(text->chars, 1, (size_t)text->length, (FILE*)file->handle);
    if (written != (size_t)text->length) {
        nativeRaise("tulis() gagal menulis ke '%s'", file->path->chars);
        return KOSONG_VAL;
    }
    return NUMBER_VAL((double)written);
}

Value nativeFnBacaSemua(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("baca_semua() memerlukan 1 argumen (path)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("baca_semua() hanya menerima string path");
        return KOSONG_VAL;
    }
    ObjString* path = AS_STRING(args[0]);
    FILE* fp = fopen(path->chars, "rb");
    if (fp == NULL) {
        nativeRaise("baca_semua() gagal membuka '%s'", path->chars);
        return KOSONG_VAL;
    }
    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    if (size < 0) size = 0;
    rewind(fp);
    char* buffer = ALLOCATE(char, size + 1);
    size_t read = fread(buffer, 1, (size_t)size, fp);
    buffer[read] = '\0';
    fclose(fp);
    return OBJ_VAL(takeString(buffer, (int)read));
}

static Value writeWholeFile(const char* path, ObjString* text, const char* mode, const char* fname) {
    FILE* fp = fopen(path, mode);
    if (fp == NULL) {
        nativeRaise("%s() gagal membuka '%s'", fname, path);
        return KOSONG_VAL;
    }
    size_t written = fwrite(text->chars, 1, (size_t)text->length, fp);
    fclose(fp);
    if (written != (size_t)text->length) {
        nativeRaise("%s() gagal menulis ke '%s'", fname, path);
        return KOSONG_VAL;
    }
    return NUMBER_VAL((double)written);
}

Value nativeFnTulisSemua(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("tulis_semua() memerlukan 2 argumen (path, teks)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        nativeRaise("tulis_semua() memerlukan (string, string)");
        return KOSONG_VAL;
    }
    return writeWholeFile(AS_STRING(args[0])->chars, AS_STRING(args[1]), "wb", "tulis_semua");
}

Value nativeFnTambahBerkas(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("tambah_berkas() memerlukan 2 argumen (path, teks)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0]) || !IS_STRING(args[1])) {
        nativeRaise("tambah_berkas() memerlukan (string, string)");
        return KOSONG_VAL;
    }
    return writeWholeFile(AS_STRING(args[0])->chars, AS_STRING(args[1]), "ab", "tambah_berkas");
}

Value nativeFnAdaBerkas(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("ada_berkas() memerlukan 1 argumen (path)");
        return BOOL_VAL(false);
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("ada_berkas() hanya menerima string path");
        return BOOL_VAL(false);
    }
    FILE* fp = fopen(AS_STRING(args[0])->chars, "rb");
    if (fp == NULL) return BOOL_VAL(false);
    fclose(fp);
    return BOOL_VAL(true);
}

Value nativeFnHapusBerkas(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("hapus_berkas() memerlukan 1 argumen (path)");
        return BOOL_VAL(false);
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("hapus_berkas() hanya menerima string path");
        return BOOL_VAL(false);
    }
    if (remove(AS_STRING(args[0])->chars) != 0) {
        nativeRaise("hapus_berkas() gagal menghapus '%s'", AS_STRING(args[0])->chars);
        return BOOL_VAL(false);
    }
    return BOOL_VAL(true);
}

// ============================================================================
// FUNGSI WAKTU & TANGGAL - IMPLEMENTASI
// ============================================================================

// waktu() - Detik sejak epoch (Unix time).
Value nativeFnWaktu(int argCount, Value* args) {
    (void)args;
    if (argCount != 0) {
        nativeRaise("waktu() tidak menerima argumen");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL((double)time(NULL));
}

// Helper: set a numeric entry on a dict (key is a C string).
static void dictSetNum(ObjDict* dict, const char* key, double num) {
    ObjString* k = copyString(key, (int)strlen(key));
    push(OBJ_VAL(k));  // keep reachable during tableSet allocation
    tableSet(&dict->table, k, NUMBER_VAL(num));
    pop();
}

// tanggal() - Kamus berisi komponen tanggal/waktu lokal saat ini:
// {tahun, bulan, hari, jam, menit, detik, hari_minggu, hari_tahun}
Value nativeFnTanggal(int argCount, Value* args) {
    (void)args;
    if (argCount != 0) {
        nativeRaise("tanggal() tidak menerima argumen");
        return KOSONG_VAL;
    }
    time_t t = time(NULL);
    struct tm* lt = localtime(&t);
    if (lt == NULL) {
        nativeRaise("tanggal() gagal membaca waktu sistem");
        return KOSONG_VAL;
    }

    ObjDict* dict = newDict();
    push(OBJ_VAL(dict));  // keep reachable while populating
    dictSetNum(dict, "tahun",       lt->tm_year + 1900);
    dictSetNum(dict, "bulan",       lt->tm_mon + 1);
    dictSetNum(dict, "hari",        lt->tm_mday);
    dictSetNum(dict, "jam",         lt->tm_hour);
    dictSetNum(dict, "menit",       lt->tm_min);
    dictSetNum(dict, "detik",       lt->tm_sec);
    dictSetNum(dict, "hari_minggu", lt->tm_wday);
    dictSetNum(dict, "hari_tahun",  lt->tm_yday + 1);
    pop();
    return OBJ_VAL(dict);
}

// format_tanggal(pola) - Format waktu lokal saat ini dengan pola strftime.
Value nativeFnFormatTanggal(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("format_tanggal() memerlukan 1 argumen (pola)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("format_tanggal() hanya menerima string pola");
        return KOSONG_VAL;
    }
    time_t t = time(NULL);
    struct tm* lt = localtime(&t);
    if (lt == NULL) {
        nativeRaise("format_tanggal() gagal membaca waktu sistem");
        return KOSONG_VAL;
    }
    char buf[256];
    size_t n = strftime(buf, sizeof(buf), AS_STRING(args[0])->chars, lt);
    return OBJ_VAL(copyString(buf, (int)n));
}

// ============================================================================
// PRIMITIF TEKS - untuk membangun pustaka teks di bahasa .idk
// ============================================================================

// karakter(teks, i) - Karakter (string 1-byte) pada indeks i.
Value nativeFnKarakter(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("karakter() memerlukan 2 argumen (teks, indeks)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0]) || !IS_NUMBER(args[1])) {
        nativeRaise("karakter() memerlukan (string, angka)");
        return KOSONG_VAL;
    }
    ObjString* s = AS_STRING(args[0]);
    int i = (int)AS_NUMBER(args[1]);
    if (i < 0 || i >= s->length) {
        nativeRaise("karakter() indeks di luar jangkauan (indeks %d, panjang %d)", i, s->length);
        return KOSONG_VAL;
    }
    return OBJ_VAL(copyString(s->chars + i, 1));
}

// potong(teks, mulai, jumlah) - Substring mulai dari 'mulai' sepanjang 'jumlah'.
// Jika jumlah melewati akhir string, dipotong sampai akhir.
Value nativeFnPotong(int argCount, Value* args) {
    if (argCount != 3) {
        nativeRaise("potong() memerlukan 3 argumen (teks, mulai, jumlah)");
        return KOSONG_VAL;
    }
    if (!IS_STRING(args[0]) || !IS_NUMBER(args[1]) || !IS_NUMBER(args[2])) {
        nativeRaise("potong() memerlukan (string, angka, angka)");
        return KOSONG_VAL;
    }
    ObjString* s = AS_STRING(args[0]);
    int start = (int)AS_NUMBER(args[1]);
    int count = (int)AS_NUMBER(args[2]);
    if (start < 0 || start > s->length) {
        nativeRaise("potong() 'mulai' di luar jangkauan (%d, panjang %d)", start, s->length);
        return KOSONG_VAL;
    }
    if (count < 0) count = 0;
    if (start + count > s->length) count = s->length - start;
    return OBJ_VAL(copyString(s->chars + start, count));
}
