// usleep() is POSIX, not C11: glibc hides it under -std=c11 unless a
// feature-test macro is set before the first system header (GCC 14 treats
// the implicit declaration as an error).
#if !defined(_WIN32)
#define _DEFAULT_SOURCE
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef SOCKET indk_socket_t;
    #define INDK_INVALID_SOCKET INVALID_SOCKET
    #define INDK_CLOSESOCKET(s) closesocket(s)
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    typedef int indk_socket_t;
    #define INDK_INVALID_SOCKET (-1)
    #define INDK_CLOSESOCKET(s) close(s)
#endif

#include "native.h"
#include "vm.h"
#include "object.h"
#include "memory.h"
#include "common.h"

// ============================================================================
// FUNGSI MATEMATIKA - IMPLEMENTASI
// ============================================================================

// Keduanya menerima bentuk varargs LAMA (min(3, 1, 2)) dan satu argumen
// urutan (min([3, 1, 2])), seperti Python. Lihat minMax() di bawah.
static Value minMax(int argCount, Value* args, bool wantMax, const char* nama);

Value nativeFnMin(int argCount, Value* args) {
    return minMax(argCount, args, false, "min");
}

Value nativeFnMax(int argCount, Value* args) {
    return minMax(argCount, args, true, "max");
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
    if (IS_INT(args[0])) {
        int64_t n = AS_INT(args[0]);
        if (n == INT64_MIN) {
            nativeRaiseKelas(vm.galatLuapan, "abs() melebihi jangkauan bilangan bulat");
            return NUMBER_VAL(0);
        }
        return INT_VAL(n < 0 ? -n : n);       // abs() of a bulat stays bulat
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
    // pangkat() mirrors the '**' operator: bulat ** bulat >= 0 stays bulat.
    if (IS_INT(args[0]) && IS_INT(args[1]) && AS_INT(args[1]) >= 0) {
        bool overflow;
        int64_t hasil = intPow(AS_INT(args[0]), AS_INT(args[1]), &overflow);
        if (overflow) {
            nativeRaiseKelas(vm.galatLuapan, "pangkat() melebihi jangkauan bilangan bulat");
            return NUMBER_VAL(0);
        }
        return INT_VAL(hasil);
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
    // Python's round() returns an int.
    double hasil = round(AS_NUMBER(args[0]));
    if (hasil >= 9223372036854775808.0 || hasil < -9223372036854775808.0) {
        nativeRaiseKelas(vm.galatLuapan, "bulat() melebihi jangkauan bilangan bulat");
        return NUMBER_VAL(0);
    }
    return INT_VAL((int64_t)hasil);
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
    // math.floor() returns an int in Python.
    double hasil = floor(AS_NUMBER(args[0]));
    if (hasil >= 9223372036854775808.0 || hasil < -9223372036854775808.0) {
        nativeRaiseKelas(vm.galatLuapan, "lantai() melebihi jangkauan bilangan bulat");
        return NUMBER_VAL(0);
    }
    return INT_VAL((int64_t)hasil);
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
    // math.ceil() returns an int in Python.
    double hasil = ceil(AS_NUMBER(args[0]));
    if (hasil >= 9223372036854775808.0 || hasil < -9223372036854775808.0) {
        nativeRaiseKelas(vm.galatLuapan, "atap() melebihi jangkauan bilangan bulat");
        return NUMBER_VAL(0);
    }
    return INT_VAL((int64_t)hasil);
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
// ANGKA & TIPE (padanan round/divmod/bin/oct/hex/int/bool/isinstance/...)
// ============================================================================

// Baca satu elemen dari urutan yang bisa diiterasi (daftar, rentang, teks).
// Mengembalikan false bila tipenya bukan urutan.
static bool seqLength(Value seq, int* length) {
    if (sequenceItems(seq) != NULL) *length = sequenceItems(seq)->count;
    else if (IS_SET(seq)) *length = AS_SET(seq)->keys.count;
    else if (IS_RANGE(seq)) *length = (int)rangeLength(AS_RANGE(seq));
    else if (IS_STRING(seq)) *length = AS_STRING(seq)->charCount;
    else return false;
    return true;
}

// Alokasi: untuk teks ini membuat ObjString baru, jadi pemanggil wajib
// menjaga nilai lain yang dipegangnya tetap tereferensi dari stack VM.
static Value seqGet(Value seq, int i) {
    if (sequenceItems(seq) != NULL) return sequenceItems(seq)->values[i];
    if (IS_SET(seq)) return AS_SET(seq)->keys.values[i];
    if (IS_RANGE(seq)) return rangeElement(AS_RANGE(seq), i);
    ObjString* teks = AS_STRING(seq);
    int offset = utf8ByteOffset(teks, i);
    return OBJ_VAL(copyString(teks->chars + offset, utf8CharBytes(teks, offset)));
}

// Perbandingan '<' seperti operator bahasa: angka dengan angka, teks dengan
// teks (strcmp). Campuran tipe adalah GalatTipe.
static bool valueLess(Value a, Value b, const char* nama, bool* less) {
    if (IS_NUMBER(a) && IS_NUMBER(b)) {
        *less = AS_NUMBER(a) < AS_NUMBER(b);
        return true;
    }
    if (IS_STRING(a) && IS_STRING(b)) {
        *less = strcmp(AS_CSTRING(a), AS_CSTRING(b)) < 0;
        return true;
    }
    nativeRaiseKelas(vm.galatTipe, "%s() tidak dapat membandingkan tipe yang berbeda", nama);
    return false;
}

// Inti max()/min(): bentuk varargs ATAU satu argumen urutan (daftar, rentang,
// teks), seperti Python.
static Value minMax(int argCount, Value* args, bool wantMax, const char* nama) {
    Value seq;
    int count;
    bool fromSeq;

    // Pesan "minimal 2 argumen" dipertahankan apa adanya: golden
    // kesalahan_native menegaskan teks ini untuk min(5).
    if (argCount == 0) {
        nativeRaiseKelas(vm.galatTipe, "%s() memerlukan minimal 2 argumen", nama);
        return NUMBER_VAL(0);
    }
    if (argCount == 1) {
        if (!seqLength(args[0], &count)) {
            nativeRaiseKelas(vm.galatTipe, "%s() memerlukan minimal 2 argumen", nama);
            return NUMBER_VAL(0);
        }
        if (count == 0) {
            nativeRaiseKelas(vm.galatNilai, "%s() dari urutan kosong", nama);
            return NUMBER_VAL(0);
        }
        seq = args[0];
        fromSeq = true;
    } else {
        count = argCount;
        seq = KOSONG_VAL;
        fromSeq = false;
    }

    // Kandidat terbaik disimpan di stack VM: seqGet() dapat mengalokasi
    // (teks) dan memicu GC saat kandidat sebelumnya belum tereferensi.
    push(fromSeq ? seqGet(seq, 0) : args[0]);
    for (int i = 1; i < count; i++) {
        Value item = fromSeq ? seqGet(seq, i) : args[i];
        push(item);
        bool less;
        if (!valueLess(vm.stackTop[-1], vm.stackTop[-2], nama, &less)) {
            vm.stackTop -= 2;
            return NUMBER_VAL(0);
        }
        if (less != wantMax) {       // item lebih baik dari kandidat
            Value better = pop();
            pop();
            push(better);
        } else {
            pop();
        }
    }
    return pop();
}

// bulatkan(x[, digit]) - round() Python: pembulatan setengah ke genap.
// Dikerjakan lewat printf("%.*f") karena glibc membulatkan dari nilai biner
// yang SEBENARNYA dengan ties-to-even — persis algoritma _Py_dg_dtoa CPython.
// Karena itu bulatkan(2.675, 2) == 2.67, sama seperti Python (2.675 sejatinya
// 2.67499...): ini bukan bug, jangan "diperbaiki".
Value nativeFnBulatkan(int argCount, Value* args) {
    if (argCount < 1 || argCount > 2) {
        nativeRaiseKelas(vm.galatTipe, "bulatkan() memerlukan 1 atau 2 argumen");
        return NUMBER_VAL(0);
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaiseKelas(vm.galatTipe, "bulatkan() hanya menerima angka");
        return NUMBER_VAL(0);
    }
    double x = AS_NUMBER(args[0]);
    int digit = 0;
    if (argCount == 2) {
        if (!IS_NUMBER(args[1]) || AS_NUMBER(args[1]) != floor(AS_NUMBER(args[1]))) {
            nativeRaiseKelas(vm.galatTipe, "bulatkan() digit harus bilangan bulat");
            return NUMBER_VAL(0);
        }
        double d = AS_NUMBER(args[1]);
        if (d > 320) return NUMBER_VAL(x);      // sudah lebih presisi dari double
        if (d < -320) return NUMBER_VAL(x < 0 ? -0.0 : 0.0);
        digit = (int)d;
    }
    if (!isfinite(x)) return NUMBER_VAL(x);

    // Python's round(x) with no digit returns an int (banker's rounding).
    if (digit == 0) {
        double hasil = nearbyint(x);
        if (hasil >= 9223372036854775808.0 || hasil < -9223372036854775808.0) {
            nativeRaiseKelas(vm.galatLuapan, "bulatkan() melebihi jangkauan bilangan bulat");
            return NUMBER_VAL(0);
        }
        return INT_VAL((int64_t)hasil);
    }

    if (digit < 0) {
        // printf tidak punya presisi negatif: skala, bulatkan setengah-ke-genap
        // (mode pembulatan default FPU), lalu kembalikan skalanya.
        double scale = pow(10.0, -digit);
        return NUMBER_VAL(nearbyint(x / scale) * scale);
    }
    if (fabs(x) >= 1e17) return NUMBER_VAL(x);  // sudah bulat pada presisi itu

    char buf[512];
    snprintf(buf, sizeof(buf), "%.*f", digit, x);
    return NUMBER_VAL(strtod(buf, NULL));
}

// bagi_sisa(a, b) - divmod(): [a // b, a % b], konsisten dengan
// OP_FLOOR_DIVIDE dan OP_MODULO di vm.c (lantai + sisa bertanda pembagi).
Value nativeFnBagiSisa(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaiseKelas(vm.galatTipe, "bagi_sisa() memerlukan 2 argumen");
        return KOSONG_VAL;
    }
    if (!IS_NUMBER(args[0]) || !IS_NUMBER(args[1])) {
        nativeRaiseKelas(vm.galatTipe, "bagi_sisa() hanya menerima angka");
        return KOSONG_VAL;
    }
    if (IS_INT(args[0]) && IS_INT(args[1])) {
        int64_t a = AS_INT(args[0]);
        int64_t b = AS_INT(args[1]);
        if (b == 0) {
            nativeRaiseKelas(vm.galatBagiNol, "Pembagian dengan nol.");
            return KOSONG_VAL;
        }
        int64_t hasilBagi, hasilSisa;
        if (!intFloorDiv(a, b, &hasilBagi) || !intModulo(a, b, &hasilSisa)) {
            nativeRaiseKelas(vm.galatLuapan, "bagi_sisa() melebihi jangkauan bilangan bulat");
            return KOSONG_VAL;
        }
        ObjList* hasil = newList();
        push(OBJ_VAL(hasil));
        writeValueArray(&hasil->items, INT_VAL(hasilBagi));
        writeValueArray(&hasil->items, INT_VAL(hasilSisa));
        pop();
        return OBJ_VAL(hasil);
    }

    double a = AS_NUMBER(args[0]);
    double b = AS_NUMBER(args[1]);
    if (b == 0) {
        nativeRaiseKelas(vm.galatBagiNol, "Pembagian dengan nol.");
        return KOSONG_VAL;
    }
    double sisa = fmod(a, b);
    if (sisa == 0) {
        sisa = 0;                                // hindari "-0"
    } else if ((sisa < 0) != (b < 0)) {
        sisa += b;
    }

    ObjList* hasil = newList();
    push(OBJ_VAL(hasil));                        // jaga dari GC saat tumbuh
    writeValueArray(&hasil->items, NUMBER_VAL(floor(a / b)));
    writeValueArray(&hasil->items, NUMBER_VAL(sisa));
    pop();
    return OBJ_VAL(hasil);
}

// Inti biner()/oktal()/heksa(): teks berawalan, tanda minus di depan awalan
// seperti Python ('-0b1010').
static Value toBase(int argCount, Value* args, int base, const char* prefix,
                    const char* nama) {
    if (argCount != 1) {
        nativeRaiseKelas(vm.galatTipe, "%s() memerlukan 1 argumen", nama);
        return KOSONG_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaiseKelas(vm.galatTipe, "%s() hanya menerima angka", nama);
        return KOSONG_VAL;
    }
    int64_t bulat;
    if (IS_INT(args[0])) {
        bulat = AS_INT(args[0]);
    } else {
        double x = AS_NUMBER(args[0]);
        if (!isfinite(x) || x != floor(x)) {
            nativeRaiseKelas(vm.galatTipe, "%s() memerlukan bilangan bulat", nama);
            return KOSONG_VAL;
        }
        if (x >= 9223372036854775808.0 || x < -9223372036854775808.0) {
            nativeRaiseKelas(vm.galatNilai, "%s() angka terlalu besar", nama);
            return KOSONG_VAL;
        }
        bulat = (int64_t)x;
    }

    bool negatif = bulat < 0;
    // Two's-complement magnitude, so INT64_MIN converts without UB.
    unsigned long long n = negatif ? (unsigned long long)(-(bulat + 1)) + 1
                                   : (unsigned long long)bulat;
    char digits[72];
    int len = 0;
    do {
        digits[len++] = "0123456789abcdef"[n % (unsigned)base];
        n /= (unsigned)base;
    } while (n > 0);

    char buf[80];
    int pos = 0;
    if (negatif) buf[pos++] = '-';
    buf[pos++] = prefix[0];
    buf[pos++] = prefix[1];
    while (len > 0) buf[pos++] = digits[--len];
    return OBJ_VAL(copyString(buf, pos));
}

Value nativeFnBiner(int argCount, Value* args)  { return toBase(argCount, args, 2,  "0b", "biner"); }
Value nativeFnOktal(int argCount, Value* args)  { return toBase(argCount, args, 8,  "0o", "oktal"); }
Value nativeFnHeksa(int argCount, Value* args)  { return toBase(argCount, args, 16, "0x", "heksa"); }

// ke_bulat(x) - int(): memotong KE ARAH NOL (bukan lantai), dan menerima teks.
Value nativeFnKeBulat(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaiseKelas(vm.galatTipe, "ke_bulat() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    if (IS_BOOL(args[0])) return INT_VAL(AS_BOOL(args[0]) ? 1 : 0);
    if (IS_INT(args[0])) return args[0];
    if (IS_NUMBER(args[0])) {
        double x = AS_NUMBER(args[0]);
        if (!isfinite(x)) {
            nativeRaiseKelas(vm.galatNilai, "ke_bulat() tidak dapat mengubah %s",
                             isnan(x) ? "nan" : "takhingga");
            return NUMBER_VAL(0);
        }
        // Truncates toward zero, like Python's int().
        if (trunc(x) > 9223372036854775807.0 || trunc(x) < -9223372036854775808.0) {
            nativeRaiseKelas(vm.galatLuapan, "ke_bulat() melebihi jangkauan bilangan bulat");
            return NUMBER_VAL(0);
        }
        return INT_VAL((int64_t)trunc(x));
    }
    if (!IS_STRING(args[0])) {
        nativeRaiseKelas(vm.galatTipe, "ke_bulat() hanya menerima angka, teks, atau logika");
        return NUMBER_VAL(0);
    }

    // Seperti int("..."): spasi di tepi boleh, pecahan tidak.
    const char* s = AS_CSTRING(args[0]);
    const char* p = s;
    while (isspace((unsigned char)*p)) p++;
    const char* mulai = p;
    if (*p == '+' || *p == '-') p++;
    if (!isdigit((unsigned char)*p)) {
        nativeRaiseKelas(vm.galatNilai, "ke_bulat() gagal: '%s' bukan bilangan bulat", s);
        return NUMBER_VAL(0);
    }
    while (isdigit((unsigned char)*p)) p++;
    const char* selesai = p;
    while (isspace((unsigned char)*p)) p++;
    if (*p != '\0') {
        nativeRaiseKelas(vm.galatNilai, "ke_bulat() gagal: '%s' bukan bilangan bulat", s);
        return NUMBER_VAL(0);
    }
    (void)selesai;
    errno = 0;
    long long nilai = strtoll(mulai, NULL, 10);
    if (errno == ERANGE) {
        nativeRaiseKelas(vm.galatLuapan, "ke_bulat() melebihi jangkauan bilangan bulat");
        return NUMBER_VAL(0);
    }
    return INT_VAL(nilai);
}

// ke_logika(x) - bool(): memakai truthiness K1 yang sama dengan jika/tidak.
Value nativeFnKeLogika(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaiseKelas(vm.galatTipe, "ke_logika() memerlukan 1 argumen");
        return BOOL_VAL(false);
    }
    return BOOL_VAL(!isFalsey(args[0]));
}

// adalah_instansi(objek, kelas) - isinstance(). Memakai valueIsInstanceOf()
// di vm.c, helper yang sama yang dipakai OP_IS_INSTANCE untuk `kecuali`.
Value nativeFnAdalahInstansi(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaiseKelas(vm.galatTipe, "adalah_instansi() memerlukan 2 argumen");
        return BOOL_VAL(false);
    }
    if (!IS_CLASS(args[1])) {
        nativeRaiseKelas(vm.galatTipe, "adalah_instansi() argumen kedua harus kelas");
        return BOOL_VAL(false);
    }
    return BOOL_VAL(valueIsInstanceOf(args[0], args[1]));
}

// bisa_dipanggil(x) - callable().
Value nativeFnBisaDipanggil(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaiseKelas(vm.galatTipe, "bisa_dipanggil() memerlukan 1 argumen");
        return BOOL_VAL(false);
    }
    Value v = args[0];
    return BOOL_VAL(IS_FUNCTION(v) || IS_CLOSURE(v) || IS_NATIVE(v) ||
                    IS_CLASS(v) || IS_BOUND_METHOD(v));
}

// repr(x) - repr(): teks diberi tanda kutip, sisanya seperti ke_teks().
// Defined further down, next to the other text-building helpers.
static void appendStringRepr(ObjString* str, char* buf, size_t size, size_t* pos);

Value nativeFnRepr(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaiseKelas(vm.galatTipe, "repr() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    if (IS_STRING(args[0])) {
        // Quote and escape via the shared rule, so repr() agrees with the
        // way collections print their elements.
        char buf[512];
        size_t pos = 0;
        buf[0] = '\0';
        appendStringRepr(AS_STRING(args[0]), buf, sizeof(buf), &pos);
        return OBJ_VAL(copyString(buf, (int)pos));
    }
    return nativeFnKeTeks(argCount, args);
}

// id(x) - identitas objek, konsisten dengan operator `adalah`
// (valuesIdentical): objek memakai alamat heap-nya, primitif memakai hash
// 53-bit dari nilainya sehingga dua primitif yang identik selalu seid.
// ponytail: hash primitif bisa (sangat kecil kemungkinannya) bertabrakan
// dengan sebuah alamat; cukup sampai ada tipe id yang bukan double.
Value nativeFnId(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaiseKelas(vm.galatTipe, "id() memerlukan 1 argumen");
        return NUMBER_VAL(0);
    }
    Value v = args[0];
    if (IS_OBJ(v)) return INT_VAL((int64_t)(uintptr_t)AS_OBJ(v));

    uint64_t hash = 1469598103934665603ULL;      // FNV-1a
    unsigned char bytes[9];
    bytes[0] = (unsigned char)v.type;
    double d = 0;
    if (IS_BOOL(v)) d = AS_BOOL(v) ? 1 : 0;
    else if (IS_NUMBER(v)) d = AS_NUMBER(v) + 0.0;   // -0.0 dan 0.0 identik
    memcpy(bytes + 1, &d, sizeof(double));
    for (size_t i = 0; i < sizeof(bytes); i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL;
    }
    return INT_VAL((int64_t)(hash & 0x1FFFFFFFFFFFFFULL));   // 53 bit: eksak di double
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
        return INT_VAL(AS_STRING(args[0])->charCount);
    }
    if (IS_SET(args[0])) {
        return INT_VAL(AS_SET(args[0])->keys.count);
    }
    if (sequenceItems(args[0]) != NULL) {          // daftar or tupel
        return INT_VAL(sequenceItems(args[0])->count);
    }
    if (IS_DICT(args[0])) {
        return INT_VAL(dictLength(AS_DICT(args[0])));
    }
    if (IS_RANGE(args[0])) {
        return INT_VAL((int64_t)rangeLength(AS_RANGE(args[0])));
    }
    nativeRaise("panjang() hanya menerima string, list, kamus, atau rentang");
    return NUMBER_VAL(0);
}

// tupel is immutable: the daftar-mutating builtins refuse it with the same
// message as t[i] = x in the VM, instead of a confusing "harus list".
static bool rejectTuple(Value value) {
    if (!IS_TUPLE(value)) return false;
    nativeRaiseKelas(vm.galatTipe, "Tupel tidak dapat diubah.");
    return true;
}

// Every himpunan write path routes through here: tambah() and hapus() are the
// only two, so one guard covers them both.
static bool rejectFrozenSet(Value value) {
    if (!IS_SET(value) || !AS_SET(value)->frozen) return false;
    nativeRaiseKelas(vm.galatTipe, "Himpunan beku tidak dapat diubah.");
    return true;
}

Value nativeFnTambah(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("tambah() memerlukan 2 argumen (list, nilai)");
        return KOSONG_VAL;
    }
    if (rejectTuple(args[0])) return KOSONG_VAL;
    if (rejectFrozenSet(args[0])) return KOSONG_VAL;
    if (IS_SET(args[0])) {   // tambah(himpunan, x): already a member is fine
        if (!setAdd(AS_SET(args[0]), args[1])) {
            nativeRaiseKelas(vm.galatTipe, "Anggota himpunan harus nilai yang dapat di-hash.");
        }
        return args[0];
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
    if (rejectTuple(args[0])) return KOSONG_VAL;
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
    if (!IS_DICT(args[0])) {
        nativeRaise("punya() memerlukan (kamus, kunci)");
        return BOOL_VAL(false);
    }
    Value tmp;
    return BOOL_VAL(valueTableGet(&AS_DICT(args[0])->table, args[1], &tmp));
}

Value nativeFnHapusKunci(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("hapus_kunci() memerlukan 2 argumen (kamus, kunci)");
        return BOOL_VAL(false);
    }
    if (!IS_DICT(args[0])) {
        nativeRaise("hapus_kunci() memerlukan (kamus, kunci)");
        return BOOL_VAL(false);
    }
    return BOOL_VAL(dictDelete(AS_DICT(args[0]), args[1]));
}

// enumerasi(urutan[, awal]) - pasangan [indeks, nilai] untuk iterable dasar
Value nativeFnEnumerasi(int argCount, Value* args) {
    if (argCount < 1 || argCount > 2) {
        nativeRaise("enumerasi() memerlukan 1 atau 2 argumen");
        return KOSONG_VAL;
    }
    if (argCount == 2 && !IS_NUMBER(args[1])) {
        nativeRaise("enumerasi() argumen awal harus angka");
        return KOSONG_VAL;
    }

    Value urutan = args[0];
    int count;
    if (sequenceItems(urutan) != NULL) count = sequenceItems(urutan)->count;
    else if (IS_SET(urutan)) count = AS_SET(urutan)->keys.count;
    else if (IS_STRING(urutan)) count = AS_STRING(urutan)->charCount;
    else if (IS_RANGE(urutan)) count = (int)rangeLength(AS_RANGE(urutan));
    else if (IS_DICT(urutan)) count = dictLength(AS_DICT(urutan));
    else {
        nativeRaise("enumerasi() hanya menerima daftar, tupel, himpunan, teks, rentang, atau kamus");
        return KOSONG_VAL;
    }

    if (argCount == 2 && AS_NUMBER(args[1]) != floor(AS_NUMBER(args[1]))) {
        nativeRaise("enumerasi() argumen awal harus bilangan bulat");
        return KOSONG_VAL;
    }
    int64_t awal = argCount == 2 ? (int64_t)AS_NUMBER(args[1]) : 0;
    ObjList* hasil = newList();
    push(OBJ_VAL(hasil));
    for (int i = 0; i < count; i++) {
        ObjList* pasangan = newList();
        push(OBJ_VAL(pasangan));
        Value nilai;
        if (sequenceItems(urutan) != NULL) nilai = sequenceItems(urutan)->values[i];
        else if (IS_SET(urutan)) nilai = AS_SET(urutan)->keys.values[i];
        else if (IS_STRING(urutan)) {
            ObjString* teks = AS_STRING(urutan);
            int offset = utf8ByteOffset(teks, i);
            nilai = OBJ_VAL(copyString(teks->chars + offset, utf8CharBytes(teks, offset)));
        } else if (IS_RANGE(urutan)) {
            nilai = rangeElement(AS_RANGE(urutan), i);
        } else nilai = dictKeyAt(AS_DICT(urutan), i);

        push(nilai);
        writeValueArray(&pasangan->items, INT_VAL(awal + i));
        writeValueArray(&pasangan->items, nilai);
        pop();
        writeValueArray(&hasil->items, OBJ_VAL(pasangan));
        pop();
    }
    pop();
    return OBJ_VAL(hasil);
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
    for (int i = 0; i < dictLength(dict); i++) {
        writeValueArray(&keys->items, dictKeyAt(dict, i));
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
    } else if (IS_TUPLE(args[0])) {
        return OBJ_VAL(copyString("tupel", 5));
    } else if (IS_SET(args[0])) {
        return AS_SET(args[0])->frozen ? OBJ_VAL(copyString("himpunan_beku", 13))
                                       : OBJ_VAL(copyString("himpunan", 8));
    } else if (IS_DICT(args[0])) {
        return OBJ_VAL(copyString("kamus", 5));
    } else if (IS_INSTANCE(args[0])) {
        return OBJ_VAL(copyString("objek", 5));
    } else if (IS_CLASS(args[0])) {
        return OBJ_VAL(copyString("kelas", 5));
    } else if (IS_FILE(args[0])) {
        return OBJ_VAL(copyString("berkas", 6));
    } else if (IS_DB(args[0])) {
        return OBJ_VAL(copyString("db", 2));
    } else if (IS_SOCKET(args[0])) {
        return OBJ_VAL(copyString("soket", 5));
    } else if (IS_RANGE(args[0])) {
        return OBJ_VAL(copyString("rentang", 7));
    } else if (IS_ITERATOR(args[0])) {
        return OBJ_VAL(copyString("iterator", 8));
    } else if (IS_MODULE(args[0])) {
        return OBJ_VAL(copyString("modul", 5));
    } else if (IS_FUNCTION(args[0]) || IS_CLOSURE(args[0]) ||
               IS_NATIVE(args[0]) || IS_BOUND_METHOD(args[0])) {
        return OBJ_VAL(copyString("fungsi", 6));
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

static void appendValueText(Value v, char* buf, size_t size, size_t* pos);
// `repr`: quote and escape strings, for values printed inside a collection.
static void appendValueRepr(Value v, char* buf, size_t size, size_t* pos);

// Append a literal string, respecting the buffer bound.
static void appendRaw(const char* s, char* buf, size_t size, size_t* pos) {
    for (int i = 0; s[i] != '\0' && *pos + 1 < size; i++) {
        buf[(*pos)++] = s[i];
    }
    buf[*pos] = '\0';
}

// Append a value's text representation into buf at *pos (bounded by size).
// Lists, dicts and ranges recurse so ke_teks()/format()/repr() agree with
// cetak() instead of printing "<objek>". Kept self-contained (no
// open_memstream / stdout redirection) because this must build on Windows.
// Append a string in quoted/escaped form. Uses the shared stringEscape() so
// this matches printObject() and repr() exactly.
static void appendStringRepr(ObjString* str, char* buf, size_t size, size_t* pos) {
    appendRaw("\"", buf, size, pos);
    for (int i = 0; i < str->length; i++) {
        const char* esc = stringEscape(str->chars[i]);
        if (esc != NULL) {
            appendRaw(esc, buf, size, pos);
        } else if (*pos + 1 < size) {
            buf[(*pos)++] = str->chars[i];
            buf[*pos] = '\0';
        }
    }
    appendRaw("\"", buf, size, pos);
}

static void appendValueRepr(Value v, char* buf, size_t size, size_t* pos) {
    if (IS_STRING(v)) {
        appendStringRepr(AS_STRING(v), buf, size, pos);
    } else {
        appendValueText(v, buf, size, pos);
    }
}

static void appendValueText(Value v, char* buf, size_t size, size_t* pos) {
    char tmp[64];
    const char* s = NULL;
    int len = 0;

    if (IS_BOOL(v)) {
        s = AS_BOOL(v) ? "benar" : "salah";
    } else if (IS_KOSONG(v)) {
        s = "kosong";
    } else if (IS_INT(v)) {
        snprintf(tmp, sizeof(tmp), "%lld", (long long)AS_INT(v));
        s = tmp;
    } else if (IS_NUMBER(v)) {
        numberToString(AS_NUMBER(v), tmp, sizeof(tmp));
        s = tmp;
    } else if (IS_STRING(v)) {
        ObjString* str = AS_STRING(v);
        s = str->chars;
        len = str->length;
    } else if (IS_LIST(v)) {
        ObjList* list = AS_LIST(v);
        appendRaw("[", buf, size, pos);
        for (int i = 0; i < list->items.count; i++) {
            if (i > 0) appendRaw(", ", buf, size, pos);
            appendValueRepr(list->items.values[i], buf, size, pos);
        }
        appendRaw("]", buf, size, pos);
        return;
    } else if (IS_SET(v)) {
        // himpunan([1, 2]) / himpunan() -- same as printObject (K17: no literal).
        ObjSet* set = AS_SET(v);
        const char* nama = set->frozen ? "himpunan_beku" : "himpunan";
        if (set->keys.count == 0) {
            appendRaw(nama, buf, size, pos);
            appendRaw("()", buf, size, pos);
            return;
        }
        appendRaw(nama, buf, size, pos);
        appendRaw("([", buf, size, pos);
        for (int i = 0; i < set->keys.count; i++) {
            if (i > 0) appendRaw(", ", buf, size, pos);
            appendValueRepr(set->keys.values[i], buf, size, pos);
        }
        appendRaw("])", buf, size, pos);
        return;
    } else if (IS_TUPLE(v)) {
        // (1, 2), (1,), () -- same as printObject and Python.
        ObjTuple* tuple = AS_TUPLE(v);
        appendRaw("(", buf, size, pos);
        for (int i = 0; i < tuple->items.count; i++) {
            if (i > 0) appendRaw(", ", buf, size, pos);
            appendValueRepr(tuple->items.values[i], buf, size, pos);
        }
        if (tuple->items.count == 1) appendRaw(",", buf, size, pos);
        appendRaw(")", buf, size, pos);
        return;
    } else if (IS_DICT(v)) {
        ObjDict* dict = AS_DICT(v);
        appendRaw("{", buf, size, pos);
        bool first = true;
        // Insertion order (matching printObject and kunci()), any hashable
        // key type (K8); keys and values as repr.
        for (int i = 0; i < dict->keys.count; i++) {
            Value key = dict->keys.values[i];
            Value entryValue;
            if (!valueTableGet(&dict->table, key, &entryValue)) continue;
            if (!first) appendRaw(", ", buf, size, pos);
            first = false;
            appendValueRepr(key, buf, size, pos);
            appendRaw(": ", buf, size, pos);
            appendValueRepr(entryValue, buf, size, pos);
        }
        appendRaw("}", buf, size, pos);
        return;
    } else if (IS_RANGE(v)) {
        // Mirrors printObject: rentang(0, 5) / rentang(0, 10, 2).
        ObjRange* range = AS_RANGE(v);
        char start[32], stop[32], step[32];
        rangeNumberToString(range->start, start, sizeof(start));
        rangeNumberToString(range->stop, stop, sizeof(stop));
        appendRaw("rentang(", buf, size, pos);
        appendRaw(start, buf, size, pos);
        appendRaw(", ", buf, size, pos);
        appendRaw(stop, buf, size, pos);
        if (range->step != 1) {
            rangeNumberToString(range->step, step, sizeof(step));
            appendRaw(", ", buf, size, pos);
            appendRaw(step, buf, size, pos);
        }
        appendRaw(")", buf, size, pos);
        return;
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
    // A string of digits is a bulat, like Python's int(); anything else goes
    // through strtod and becomes a desimal.
    bool hanyaBulat = true;
    for (int i = 0; i < str->length; i++) {
        char c = str->chars[i];
        if (i == 0 && (c == '+' || c == '-')) continue;
        if (c < '0' || c > '9') { hanyaBulat = false; break; }
    }
    if (hanyaBulat && str->length > 0) {
        errno = 0;
        long long bulat = strtoll(str->chars, NULL, 10);
        if (errno == ERANGE) {
            nativeRaiseKelas(vm.galatLuapan, "ke_angka() melebihi jangkauan bilangan bulat");
            return NUMBER_VAL(0);
        }
        return INT_VAL(bulat);
    }

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
    long long lo = (long long)AS_NUMBER(args[0]);
    long long hi = (long long)AS_NUMBER(args[1]);
    if (lo > hi) {
        nativeRaise("acak_bulat(): min tidak boleh lebih besar dari maks");
        return NUMBER_VAL(0);
    }
    long long range = hi - lo + 1;
    return INT_VAL(lo + (rand() % range));
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
    return INT_VAL((int64_t)written);
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
    return INT_VAL((int64_t)written);
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
    push(OBJ_VAL(k));  // keep reachable during dictionary growth
    dictSet(dict, OBJ_VAL(k), NUMBER_VAL(num));
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
    if (i < 0 || i >= s->charCount) {
        nativeRaise("karakter() indeks di luar jangkauan (indeks %d, panjang %d)", i, s->charCount);
        return KOSONG_VAL;
    }
    int offset = utf8ByteOffset(s, i);
    return OBJ_VAL(copyString(s->chars + offset, utf8CharBytes(s, offset)));
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
    if (start < 0 || start > s->charCount) {
        nativeRaise("potong() 'mulai' di luar jangkauan (%d, panjang %d)", start, s->charCount);
        return KOSONG_VAL;
    }
    if (count < 0) count = 0;
    if (start + count > s->charCount) count = s->charCount - start;
    int from = utf8ByteOffset(s, start);
    int to = utf8ByteOffset(s, start + count);
    return OBJ_VAL(copyString(s->chars + from, to - from));
}

// ============================================================================
// PRIMITIF KARAKTER & LIST TAMBAHAN
// ============================================================================

// Decode the first UTF-8 character of `chars`. Malformed input falls back to
// the value of the lead byte.
static uint32_t decodeUtf8(const char* chars, int length) {
    const unsigned char* s = (const unsigned char*)chars;
    if (s[0] < 0x80 || length < 2) return s[0];
    if ((s[0] & 0xE0) == 0xC0) return ((s[0] & 0x1Fu) << 6) | (s[1] & 0x3Fu);
    if ((s[0] & 0xF0) == 0xE0 && length >= 3) {
        return ((s[0] & 0x0Fu) << 12) | ((s[1] & 0x3Fu) << 6) | (s[2] & 0x3Fu);
    }
    if ((s[0] & 0xF8) == 0xF0 && length >= 4) {
        return ((s[0] & 0x07u) << 18) | ((s[1] & 0x3Fu) << 12) |
               ((s[2] & 0x3Fu) << 6) | (s[3] & 0x3Fu);
    }
    return s[0];
}

// Encode a code point as UTF-8 into `out` (4 bytes max); returns the length.
static int encodeUtf8(uint32_t code, char* out) {
    if (code < 0x80) {
        out[0] = (char)code;
        return 1;
    }
    if (code < 0x800) {
        out[0] = (char)(0xC0 | (code >> 6));
        out[1] = (char)(0x80 | (code & 0x3F));
        return 2;
    }
    if (code < 0x10000) {
        out[0] = (char)(0xE0 | (code >> 12));
        out[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        out[2] = (char)(0x80 | (code & 0x3F));
        return 3;
    }
    out[0] = (char)(0xF0 | (code >> 18));
    out[1] = (char)(0x80 | ((code >> 12) & 0x3F));
    out[2] = (char)(0x80 | ((code >> 6) & 0x3F));
    out[3] = (char)(0x80 | (code & 0x3F));
    return 4;
}

// ord(karakter) - Kode Unicode (code point) dari string 1 karakter.
Value nativeFnOrd(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("ord() memerlukan 1 argumen (karakter)");
        return NUMBER_VAL(0);
    }
    if (!IS_STRING(args[0])) {
        nativeRaise("ord() hanya menerima string");
        return NUMBER_VAL(0);
    }
    ObjString* s = AS_STRING(args[0]);
    if (s->charCount != 1) {
        nativeRaise("ord() memerlukan string tepat 1 karakter (panjang %d)", s->charCount);
        return NUMBER_VAL(0);
    }
    return INT_VAL((int64_t)decodeUtf8(s->chars, s->length));
}

// chr(kode) - Karakter dari kode Unicode 0-0x10FFFF (dikodekan sebagai UTF-8).
Value nativeFnChr(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("chr() memerlukan 1 argumen (kode)");
        return KOSONG_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("chr() hanya menerima angka");
        return KOSONG_VAL;
    }
    int code = (int)AS_NUMBER(args[0]);
    if (code < 0 || code > 0x10FFFF || (code >= 0xD800 && code <= 0xDFFF)) {
        nativeRaise("chr() kode di luar jangkauan Unicode (%d)", code);
        return KOSONG_VAL;
    }
    char bytes[4];
    int length = encodeUtf8((uint32_t)code, bytes);
    return OBJ_VAL(copyString(bytes, length));
}

// Largest |value| for which rentang() arithmetic stays exact in a double.
#define RENTANG_MAKS 9007199254740992.0  // 2^53

// rentang(akhir) / rentang(awal, akhir) / rentang(awal, akhir, langkah) -
// Deret bilangan bulat yang malas (seperti range di Python 3).
Value nativeFnRentang(int argCount, Value* args) {
    if (argCount < 1 || argCount > 3) {
        nativeRaise("rentang() memerlukan 1 sampai 3 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    for (int i = 0; i < argCount; i++) {
        if (!IS_NUMBER(args[i]) || AS_NUMBER(args[i]) != floor(AS_NUMBER(args[i])) ||
            fabs(AS_NUMBER(args[i])) > RENTANG_MAKS) {
            nativeRaise("rentang() hanya menerima bilangan bulat");
            return KOSONG_VAL;
        }
    }
    double start = argCount == 1 ? 0 : AS_NUMBER(args[0]);
    double stop = argCount == 1 ? AS_NUMBER(args[0]) : AS_NUMBER(args[1]);
    double step = argCount == 3 ? AS_NUMBER(args[2]) : 1;
    if (step == 0) {
        nativeRaise("rentang() langkah tidak boleh nol");
        return KOSONG_VAL;
    }
    return OBJ_VAL(newRange(start, stop, step));
}

// hapus_pada(list, i) - Hapus & kembalikan elemen pada indeks i, geser sisanya.
Value nativeFnHapusPada(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("hapus_pada() memerlukan 2 argumen (list, indeks)");
        return KOSONG_VAL;
    }
    if (rejectTuple(args[0])) return KOSONG_VAL;
    if (!IS_LIST(args[0]) || !IS_NUMBER(args[1])) {
        nativeRaise("hapus_pada() memerlukan (list, angka)");
        return KOSONG_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    int i = (int)AS_NUMBER(args[1]);
    if (i < 0 || i >= list->items.count) {
        nativeRaise("hapus_pada() indeks di luar jangkauan (indeks %d, panjang %d)", i, list->items.count);
        return KOSONG_VAL;
    }
    Value removed = list->items.values[i];
    for (int j = i; j < list->items.count - 1; j++) {
        list->items.values[j] = list->items.values[j + 1];
    }
    list->items.count--;
    return removed;
}

// sisip(list, i, nilai) - Sisipkan nilai pada indeks i (i == panjang -> append).
Value nativeFnSisip(int argCount, Value* args) {
    if (argCount != 3) {
        nativeRaise("sisip() memerlukan 3 argumen (list, indeks, nilai)");
        return KOSONG_VAL;
    }
    if (rejectTuple(args[0])) return KOSONG_VAL;
    if (!IS_LIST(args[0]) || !IS_NUMBER(args[1])) {
        nativeRaise("sisip() memerlukan (list, angka, nilai)");
        return KOSONG_VAL;
    }
    ObjList* list = AS_LIST(args[0]);
    int i = (int)AS_NUMBER(args[1]);
    if (i < 0 || i > list->items.count) {
        nativeRaise("sisip() indeks di luar jangkauan (indeks %d, panjang %d)", i, list->items.count);
        return KOSONG_VAL;
    }
    // Grow by appending a placeholder (writeValueArray handles capacity + GC),
    // then shift elements right to open a slot at i.
    writeValueArray(&list->items, KOSONG_VAL);
    for (int j = list->items.count - 1; j > i; j--) {
        list->items.values[j] = list->items.values[j - 1];
    }
    list->items.values[i] = args[2];
    return args[0];
}

// ============================================================================
// SOKET (TCP) - IMPLEMENTASI
// Server HTTP sederhana. Listen di 127.0.0.1 (localhost) demi keamanan.
// Semua kegagalan memakai nativeRaise (dapat ditangkap coba/kecuali).
// ============================================================================

void indkCloseSocket(long long fd) {
    INDK_CLOSESOCKET((indk_socket_t)fd);
}

#ifdef _WIN32
static int indkWinsockReady = 0;
static void indkEnsureWinsock() {
    if (!indkWinsockReady) {
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        indkWinsockReady = 1;
    }
}
#else
static void indkEnsureWinsock() { }
#endif

// soket_dengar(port) -> soket server yang mendengarkan di 127.0.0.1:port.
Value nativeFnSoketDengar(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("soket_dengar() memerlukan 1 argumen (port)");
        return KOSONG_VAL;
    }
    if (!IS_NUMBER(args[0])) {
        nativeRaise("soket_dengar() hanya menerima angka (port)");
        return KOSONG_VAL;
    }
    int port = (int)AS_NUMBER(args[0]);
    if (port < 1 || port > 65535) {
        nativeRaise("soket_dengar() port di luar jangkauan (1-65535)");
        return KOSONG_VAL;
    }

    indkEnsureWinsock();

    indk_socket_t s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INDK_INVALID_SOCKET) {
        nativeRaise("soket_dengar() gagal membuat soket");
        return KOSONG_VAL;
    }

    // Izinkan reuse alamat agar restart cepat tidak "address in use".
    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        INDK_CLOSESOCKET(s);
        nativeRaise("soket_dengar() gagal bind ke port %d", port);
        return KOSONG_VAL;
    }
    if (listen(s, 16) != 0) {
        INDK_CLOSESOCKET(s);
        nativeRaise("soket_dengar() gagal listen di port %d", port);
        return KOSONG_VAL;
    }

    return OBJ_VAL(newSocket((long long)s));
}

// soket_terima(server) -> soket klien (blocking sampai ada koneksi).
Value nativeFnSoketTerima(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("soket_terima() memerlukan 1 argumen (soket server)");
        return KOSONG_VAL;
    }
    if (!IS_SOCKET(args[0])) {
        nativeRaise("soket_terima() hanya menerima soket");
        return KOSONG_VAL;
    }
    ObjSocket* server = AS_SOCKET(args[0]);
    if (!server->isOpen) {
        nativeRaise("soket_terima() pada soket yang sudah tertutup");
        return KOSONG_VAL;
    }
    indk_socket_t c = accept((indk_socket_t)server->fd, NULL, NULL);
    if (c == INDK_INVALID_SOCKET) {
        nativeRaise("soket_terima() gagal menerima koneksi");
        return KOSONG_VAL;
    }
    return OBJ_VAL(newSocket((long long)c));
}

// soket_baca(klien) -> teks yang diterima (satu recv, maks ~64KB), atau
// string kosong jika koneksi ditutup pihak lain.
Value nativeFnSoketBaca(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("soket_baca() memerlukan 1 argumen (soket klien)");
        return KOSONG_VAL;
    }
    if (!IS_SOCKET(args[0])) {
        nativeRaise("soket_baca() hanya menerima soket");
        return KOSONG_VAL;
    }
    ObjSocket* client = AS_SOCKET(args[0]);
    if (!client->isOpen) {
        nativeRaise("soket_baca() pada soket yang sudah tertutup");
        return KOSONG_VAL;
    }

    int cap = 65536;
    char* buffer = ALLOCATE(char, cap);
    int received = (int)recv((indk_socket_t)client->fd, buffer, cap - 1, 0);
    if (received < 0) {
        FREE_ARRAY(char, buffer, cap);
        nativeRaise("soket_baca() gagal membaca dari soket");
        return KOSONG_VAL;
    }
    buffer[received] = '\0';
    ObjString* result = takeString(buffer, received);
    return OBJ_VAL(result);
}

// soket_tulis(klien, teks) -> jumlah byte terkirim.
Value nativeFnSoketTulis(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("soket_tulis() memerlukan 2 argumen (soket, teks)");
        return NUMBER_VAL(0);
    }
    if (!IS_SOCKET(args[0]) || !IS_STRING(args[1])) {
        nativeRaise("soket_tulis() memerlukan (soket, string)");
        return NUMBER_VAL(0);
    }
    ObjSocket* client = AS_SOCKET(args[0]);
    if (!client->isOpen) {
        nativeRaise("soket_tulis() pada soket yang sudah tertutup");
        return NUMBER_VAL(0);
    }
    ObjString* text = AS_STRING(args[1]);
    int total = 0;
    while (total < text->length) {
        int sent = (int)send((indk_socket_t)client->fd,
                             text->chars + total, text->length - total, 0);
        if (sent <= 0) {
            nativeRaise("soket_tulis() gagal mengirim data");
            return NUMBER_VAL(total);
        }
        total += sent;
    }
    return NUMBER_VAL((double)total);
}

// soket_tutup(soket) -> tutup soket (server atau klien).
Value nativeFnSoketTutup(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("soket_tutup() memerlukan 1 argumen (soket)");
        return KOSONG_VAL;
    }
    if (!IS_SOCKET(args[0])) {
        nativeRaise("soket_tutup() hanya menerima soket");
        return KOSONG_VAL;
    }
    ObjSocket* sock = AS_SOCKET(args[0]);
    if (sock->isOpen) {
        INDK_CLOSESOCKET((indk_socket_t)sock->fd);
        sock->isOpen = false;
    }
    return KOSONG_VAL;
}

// ============================================================================
// AGREGASI & KOLEKSI
// ============================================================================
//
// Semua fungsi di bawah menerima tiga iterable dasar (daftar, rentang, teks)
// lewat iterCount()/iterAt(), meniru pola nativeFnEnumerasi.
//
// GC-SAFETY: iterAt() mengalokasi untuk elemen teks dan writeValueArray dapat
// menumbuhkan array; daftar hasil WAJIB berada di stack VM selama seluruh
// loop pengisian. peta()/saring() memanggil balik ke VM lewat callFromNative,
// yang bisa memicu GC kapan saja.

// Jumlah elemen iterable; -1 bila tipenya tidak iterable.
static int iterCount(Value v) {
    if (sequenceItems(v) != NULL) return sequenceItems(v)->count;
    if (IS_SET(v)) return AS_SET(v)->keys.count;
    if (IS_STRING(v)) return AS_STRING(v)->charCount;
    if (IS_RANGE(v)) return (int)rangeLength(AS_RANGE(v));
    return -1;
}

// Elemen ke-i. Untuk teks ini MENGALOKASI sebuah ObjString baru.
static Value iterAt(Value v, int i) {
    if (sequenceItems(v) != NULL) return sequenceItems(v)->values[i];
    if (IS_SET(v)) return AS_SET(v)->keys.values[i];
    if (IS_RANGE(v)) return rangeElement(AS_RANGE(v), i);
    ObjString* teks = AS_STRING(v);
    int offset = utf8ByteOffset(teks, i);
    return OBJ_VAL(copyString(teks->chars + offset, utf8CharBytes(teks, offset)));
}

// tupel() / tupel(urutan) - tupel baru dari daftar, tupel, teks, rentang, atau
// kamus (kuncinya), seperti tuple() di Python.
Value nativeFnTupel(int argCount, Value* args) {
    if (argCount > 1) {
        nativeRaise("tupel() memerlukan 0 atau 1 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    if (argCount == 0) return OBJ_VAL(newTuple());
    Value src = args[0];
    if (IS_TUPLE(src)) return src;   // immutable, so sharing it is safe (Python does too)

    int count = IS_DICT(src) ? dictLength(AS_DICT(src)) : iterCount(src);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe,
                         "tupel() hanya menerima daftar, tupel, teks, rentang, atau kamus");
        return KOSONG_VAL;
    }
    // GC: the tupel and each element stay rooted while the array grows --
    // iterAt allocates a fresh string per character for teks.
    ObjTuple* tuple = newTuple();
    push(OBJ_VAL(tuple));
    for (int i = 0; i < count; i++) {
        Value element = IS_DICT(src) ? dictKeyAt(AS_DICT(src), i) : iterAt(src, i);
        push(element);
        writeValueArray(&tuple->items, element);
        pop();
    }
    pop();
    return OBJ_VAL(tuple);
}

// himpunan() / himpunan(urutan) - koleksi anggota unik (K17: satu-satunya cara
// membuat himpunan; tidak ada literal {1, 2}). Seperti set() di Python.
Value nativeFnHimpunan(int argCount, Value* args) {
    if (argCount > 1) {
        nativeRaise("himpunan() memerlukan 0 atau 1 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    ObjSet* set = newSet();
    if (argCount == 0) return OBJ_VAL(set);

    Value src = args[0];
    int count = IS_DICT(src) ? dictLength(AS_DICT(src)) : iterCount(src);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe,
                         "himpunan() hanya menerima daftar, tupel, himpunan, teks, rentang, atau kamus");
        return KOSONG_VAL;
    }
    // GC: the himpunan and each member stay rooted while the table grows --
    // iterAt allocates a fresh string per character for teks.
    push(OBJ_VAL(set));
    for (int i = 0; i < count; i++) {
        Value member = IS_DICT(src) ? dictKeyAt(AS_DICT(src), i) : iterAt(src, i);
        push(member);
        bool added = setAdd(set, member);
        pop();
        if (!added) {
            pop();
            nativeRaiseKelas(vm.galatTipe, "Anggota himpunan harus nilai yang dapat di-hash.");
            return KOSONG_VAL;
        }
    }
    pop();
    return OBJ_VAL(set);
}

// s.hapus(x) - buang anggota x; GalatKunci bila x bukan anggota (Python's
// set.remove raises KeyError; 'buang' is a keyword, so the method is 'hapus').
Value nativeFnHimpunanHapus(int argCount, Value* args) {
    if (argCount != 2 || !IS_SET(args[0])) {
        nativeRaise("hapus() pada himpunan memerlukan 1 argumen (anggota)");
        return KOSONG_VAL;
    }
    if (rejectFrozenSet(args[0])) return KOSONG_VAL;
    if (!dictDelete(setAsDict(AS_SET(args[0])), args[1])) {
        nativeRaiseKelas(vm.galatKunci, "Nilai itu bukan anggota himpunan.");
    }
    return KOSONG_VAL;
}

// jumlah(d[, awal]) - seperti sum(); hanya angka.
Value nativeFnJumlah(int argCount, Value* args) {
    if (argCount < 1 || argCount > 2) {
        nativeRaise("jumlah() memerlukan 1 atau 2 argumen");
        return NUMBER_VAL(0);
    }
    int count = iterCount(args[0]);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe, "jumlah() hanya menerima daftar, rentang, atau teks");
        return NUMBER_VAL(0);
    }
    if (argCount == 2 && !IS_NUMBER(args[1])) {
        nativeRaiseKelas(vm.galatTipe, "jumlah() nilai awal harus angka");
        return NUMBER_VAL(0);
    }

    // sum() of bulat stays bulat (and raises GalatLuapan instead of silently
    // losing precision); one desimal turns the whole total into a desimal.
    int64_t totalBulat = 0;
    double total = 0;
    bool semuaBulat = true;
    if (argCount == 2) {
        if (IS_INT(args[1])) totalBulat = AS_INT(args[1]);
        else { semuaBulat = false; total = AS_NUMBER(args[1]); }
    }
    for (int i = 0; i < count; i++) {
        Value item = iterAt(args[0], i);
        if (!IS_NUMBER(item)) {
            nativeRaiseKelas(vm.galatTipe, "jumlah() hanya menerima angka");
            return NUMBER_VAL(0);
        }
        if (semuaBulat && IS_INT(item)) {
            int64_t baru;
            if (intAddOverflow(totalBulat, AS_INT(item), &baru)) {
                nativeRaiseKelas(vm.galatLuapan, "jumlah() melebihi jangkauan bilangan bulat");
                return NUMBER_VAL(0);
            }
            totalBulat = baru;
        } else {
            if (semuaBulat) { total = (double)totalBulat; semuaBulat = false; }
            total += AS_NUMBER(item);
        }
    }
    return semuaBulat ? INT_VAL(totalBulat) : NUMBER_VAL(total);
}

// semua(d) - seperti all(); daftar kosong -> benar.
Value nativeFnSemua(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("semua() memerlukan 1 argumen");
        return BOOL_VAL(false);
    }
    int count = iterCount(args[0]);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe, "semua() hanya menerima daftar, rentang, atau teks");
        return BOOL_VAL(false);
    }
    for (int i = 0; i < count; i++) {
        if (isFalsey(iterAt(args[0], i))) return BOOL_VAL(false);
    }
    return BOOL_VAL(true);
}

// ada_yang(d) - seperti any(); daftar kosong -> salah.
Value nativeFnAdaYang(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("ada_yang() memerlukan 1 argumen");
        return BOOL_VAL(false);
    }
    int count = iterCount(args[0]);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe, "ada_yang() hanya menerima daftar, rentang, atau teks");
        return BOOL_VAL(false);
    }
    for (int i = 0; i < count; i++) {
        if (!isFalsey(iterAt(args[0], i))) return BOOL_VAL(true);
    }
    return BOOL_VAL(false);
}

// qsort tidak punya parameter konteks di C11; arah urutan dititipkan di sini.
// Aman karena VM ini single-threaded dan tidak ada alokasi (apalagi callback)
// selama qsort berjalan.
static bool terurutMenurun = false;

static int bandingNilai(const void* a, const void* b) {
    const Value* x = (const Value*)a;
    const Value* y = (const Value*)b;
    int cmp;
    if (IS_NUMBER(*x)) {
        // numberCompare keeps int64 exact when both sides are bulat.
        numberCompare(*x, *y, &cmp);
    } else {
        cmp = strcmp(AS_CSTRING(*x), AS_CSTRING(*y));
        if (cmp < 0) cmp = -1; else if (cmp > 0) cmp = 1;
    }
    return terurutMenurun ? -cmp : cmp;
}

// terurut(d[, menurun]) - seperti sorted(); selalu daftar BARU.
Value nativeFnTerurut(int argCount, Value* args) {
    if (argCount < 1 || argCount > 2) {
        nativeRaise("terurut() memerlukan 1 atau 2 argumen");
        return KOSONG_VAL;
    }
    int count = iterCount(args[0]);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe, "terurut() hanya menerima daftar, rentang, atau teks");
        return KOSONG_VAL;
    }

    ObjList* hasil = newList();
    push(OBJ_VAL(hasil));   // GC: tetap reachable selama diisi
    bool pakaiAngka = false;
    for (int i = 0; i < count; i++) {
        Value item = iterAt(args[0], i);
        if (i == 0) {
            pakaiAngka = IS_NUMBER(item);
            if (!pakaiAngka && !IS_STRING(item)) {
                pop();
                nativeRaiseKelas(vm.galatTipe, "terurut() hanya menerima angka atau teks");
                return KOSONG_VAL;
            }
        } else if (IS_NUMBER(item) != pakaiAngka || (!pakaiAngka && !IS_STRING(item))) {
            pop();
            nativeRaiseKelas(vm.galatTipe, "terurut() tidak dapat membandingkan angka dengan teks");
            return KOSONG_VAL;
        }
        push(item);                                   // GC: item teks baru
        writeValueArray(&hasil->items, item);
        pop();
    }
    terurutMenurun = argCount == 2 && !isFalsey(args[1]);
    if (count > 1) {
        qsort(hasil->items.values, (size_t)count, sizeof(Value), bandingNilai);
    }
    pop();
    return OBJ_VAL(hasil);
}

// terbalik(d) - seperti reversed(), tetapi mengembalikan daftar baru.
Value nativeFnTerbalik(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("terbalik() memerlukan 1 argumen");
        return KOSONG_VAL;
    }
    int count = iterCount(args[0]);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe, "terbalik() hanya menerima daftar, rentang, atau teks");
        return KOSONG_VAL;
    }
    ObjList* hasil = newList();
    push(OBJ_VAL(hasil));
    for (int i = count - 1; i >= 0; i--) {
        Value item = iterAt(args[0], i);
        push(item);
        writeValueArray(&hasil->items, item);
        pop();
    }
    pop();
    return OBJ_VAL(hasil);
}

// pasangkan(a, b, ...) - seperti zip(); panjang = iterable terpendek.
Value nativeFnPasangkan(int argCount, Value* args) {
    if (argCount < 2) {
        nativeRaise("pasangkan() memerlukan minimal 2 argumen");
        return KOSONG_VAL;
    }
    int shortest = -1;
    for (int a = 0; a < argCount; a++) {
        int count = iterCount(args[a]);
        if (count < 0) {
            nativeRaiseKelas(vm.galatTipe, "pasangkan() hanya menerima daftar, rentang, atau teks");
            return KOSONG_VAL;
        }
        if (shortest < 0 || count < shortest) shortest = count;
    }

    ObjList* hasil = newList();
    push(OBJ_VAL(hasil));
    for (int i = 0; i < shortest; i++) {
        ObjList* pasangan = newList();
        push(OBJ_VAL(pasangan));
        for (int a = 0; a < argCount; a++) {
            Value item = iterAt(args[a], i);
            push(item);
            writeValueArray(&pasangan->items, item);
            pop();
        }
        writeValueArray(&hasil->items, OBJ_VAL(pasangan));
        pop();
    }
    pop();
    return OBJ_VAL(hasil);
}

// peta(f, d) - seperti map(), tetapi eager: hasilnya daftar.
Value nativeFnPeta(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("peta() memerlukan 2 argumen (fungsi, iterable)");
        return KOSONG_VAL;
    }
    int count = iterCount(args[1]);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe, "peta() argumen kedua harus daftar, rentang, atau teks");
        return KOSONG_VAL;
    }
    Value fungsi = args[0];
    ObjList* hasil = newList();
    push(OBJ_VAL(hasil));   // GC: harus di stack selama SELURUH loop callback
    for (int i = 0; i < count; i++) {
        push(fungsi);
        push(iterAt(args[1], i));
        Value nilai;
        if (!callFromNative(1, &nilai)) {
            pop();   // hasil
            return KOSONG_VAL;
        }
        push(nilai);
        writeValueArray(&hasil->items, nilai);
        pop();
    }
    pop();
    return OBJ_VAL(hasil);
}

// saring(f, d) - seperti filter(), tetapi eager: hasilnya daftar.
Value nativeFnSaring(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("saring() memerlukan 2 argumen (fungsi, iterable)");
        return KOSONG_VAL;
    }
    int count = iterCount(args[1]);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe, "saring() argumen kedua harus daftar, rentang, atau teks");
        return KOSONG_VAL;
    }
    Value fungsi = args[0];
    ObjList* hasil = newList();
    push(OBJ_VAL(hasil));
    for (int i = 0; i < count; i++) {
        Value item = iterAt(args[1], i);
        push(item);          // GC: item teks baru harus selamat dari callback
        push(fungsi);
        push(item);
        Value keputusan;
        if (!callFromNative(1, &keputusan)) {
            pop();   // item
            pop();   // hasil
            return KOSONG_VAL;
        }
        if (!isFalsey(keputusan)) {
            writeValueArray(&hasil->items, item);
        }
        pop();       // item
    }
    pop();
    return OBJ_VAL(hasil);
}

// cetak(...) — K12: a normal function, not a statement.
// Positional arguments are printed separated by `pisah` (default " ") and
// followed by `akhir` (default "\n"). Keyword names arrive via
// vm.nativeArgNames; callValue only populates it for this native.
Value nativeFnCetak(int argCount, Value* args) {
    const char* pisah = " ";
    int pisahLen = 1;
    const char* akhir = "\n";
    int akhirLen = 1;

    int namedCount = vm.nativeArgNames == NULL
                         ? 0 : vm.nativeArgNames->items.count;
    int positional = argCount - namedCount;

    for (int i = 0; i < namedCount; i++) {
        ObjString* name = AS_STRING(vm.nativeArgNames->items.values[i]);
        Value value = args[positional + i];
        bool isPisah = strcmp(name->chars, "pisah") == 0;
        bool isAkhir = strcmp(name->chars, "akhir") == 0;
        if (!isPisah && !isAkhir) {
            nativeRaise("cetak() tidak punya parameter bernama '%s'", name->chars);
            return KOSONG_VAL;
        }
        if (!IS_STRING(value)) {
            nativeRaise("cetak(): '%s' harus teks", name->chars);
            return KOSONG_VAL;
        }
        if (isPisah) {
            pisah = AS_CSTRING(value);
            pisahLen = AS_STRING(value)->length;
        } else {
            akhir = AS_CSTRING(value);
            akhirLen = AS_STRING(value)->length;
        }
    }

    for (int i = 0; i < positional; i++) {
        printValue(args[i]);
        if (i < positional - 1) fwrite(pisah, 1, (size_t)pisahLen, stdout);
    }
    fwrite(akhir, 1, (size_t)akhirLen, stdout);
    return KOSONG_VAL;
}

// === Sisa Fase 2 ============================================================

// hash(x) - bulat hash, exposing the dict-key rule to the language.
Value nativeFnHash(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("hash() memerlukan 1 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    uint32_t h;
    if (!hashValue(args[0], &h)) {
        nativeRaiseKelas(vm.galatTipe, "Nilai itu tidak dapat di-hash.");
        return KOSONG_VAL;
    }
    return INT_VAL((int64_t)h);
}

// turunan_dari(kelas, induk) - issubclass; a class is its own subclass.
Value nativeFnTurunanDari(int argCount, Value* args) {
    if (argCount != 2) {
        nativeRaise("turunan_dari() memerlukan 2 argumen, bukan %d", argCount);
        return BOOL_VAL(false);
    }
    if (!IS_CLASS(args[0]) || !IS_CLASS(args[1])) {
        nativeRaiseKelas(vm.galatTipe, "turunan_dari() hanya menerima kelas.");
        return BOOL_VAL(false);
    }
    for (ObjClass* k = AS_CLASS(args[0]); k != NULL; k = k->superclass) {
        if (k == AS_CLASS(args[1])) return BOOL_VAL(true);
    }
    return BOOL_VAL(false);
}

// ke_desimal(x) - float(). Needed since K2 made ke_angka("3") a bulat.
Value nativeFnKeDesimal(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("ke_desimal() memerlukan 1 argumen, bukan %d", argCount);
        return NUMBER_VAL(0);
    }
    Value v = args[0];
    if (IS_INT(v)) return NUMBER_VAL((double)AS_INT(v));
    if (IS_NUMBER(v)) return NUMBER_VAL(AS_NUMBER(v));
    if (IS_BOOL(v)) return NUMBER_VAL(AS_BOOL(v) ? 1.0 : 0.0);
    if (!IS_STRING(v)) {
        nativeRaiseKelas(vm.galatTipe, "ke_desimal() hanya menerima angka, logika, atau teks.");
        return NUMBER_VAL(0);
    }
    ObjString* teks = AS_STRING(v);
    char* end = NULL;
    errno = 0;
    double d = strtod(teks->chars, &end);
    // Skip trailing spaces so " 3.5 " works like Python's float().
    while (end != NULL && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r')) end++;
    if (end == teks->chars || *end != '\0') {
        nativeRaiseKelas(vm.galatNilai, "ke_desimal() gagal: '%s' bukan angka valid", teks->chars);
        return NUMBER_VAL(0);
    }
    if (errno == ERANGE) {
        nativeRaiseKelas(vm.galatLuapan, "ke_desimal() melebihi jangkauan desimal");
        return NUMBER_VAL(0);
    }
    return NUMBER_VAL(d);
}

// daftar() / daftar(urutan) - list(). Always a fresh copy, even from a daftar.
Value nativeFnDaftar(int argCount, Value* args) {
    if (argCount > 1) {
        nativeRaise("daftar() memerlukan 0 atau 1 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    ObjList* list = newList();
    if (argCount == 0) return OBJ_VAL(list);

    Value src = args[0];
    int count = IS_DICT(src) ? dictLength(AS_DICT(src)) : iterCount(src);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe,
                         "daftar() hanya menerima daftar, tupel, himpunan, teks, rentang, atau kamus");
        return KOSONG_VAL;
    }
    // GC: iterAt allocates a fresh string per character, so the new daftar and
    // each element must stay rooted while the array grows.
    push(OBJ_VAL(list));
    for (int i = 0; i < count; i++) {
        Value element = IS_DICT(src) ? dictKeyAt(AS_DICT(src), i) : iterAt(src, i);
        push(element);
        writeValueArray(&list->items, element);
        pop();
    }
    pop();
    return OBJ_VAL(list);
}

// himpunan_beku() / himpunan_beku(urutan) - a hashable himpunan.
Value nativeFnHimpunanBeku(int argCount, Value* args) {
    if (argCount > 1) {
        nativeRaise("himpunan_beku() memerlukan 0 atau 1 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    ObjSet* set = newSet();
    if (argCount == 0) {
        set->frozen = true;
        return OBJ_VAL(set);
    }

    Value src = args[0];
    int count = IS_DICT(src) ? dictLength(AS_DICT(src)) : iterCount(src);
    if (count < 0) {
        nativeRaiseKelas(vm.galatTipe,
                         "himpunan_beku() hanya menerima daftar, tupel, himpunan, teks, rentang, atau kamus");
        return KOSONG_VAL;
    }
    push(OBJ_VAL(set));
    for (int i = 0; i < count; i++) {
        Value member = IS_DICT(src) ? dictKeyAt(AS_DICT(src), i) : iterAt(src, i);
        push(member);
        bool added = setAdd(set, member);
        pop();
        if (!added) {
            pop();
            nativeRaiseKelas(vm.galatTipe, "Anggota himpunan harus nilai yang dapat di-hash.");
            return KOSONG_VAL;
        }
    }
    pop();
    // Freeze only after filling: setAdd refuses a frozen himpunan.
    set->frozen = true;
    return OBJ_VAL(set);
}

// iter(urutan) - an explicit cursor, pairing with lanjutkan().
Value nativeFnIter(int argCount, Value* args) {
    if (argCount != 1) {
        nativeRaise("iter() memerlukan 1 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    Value src = args[0];
    if (IS_ITERATOR(src)) return src;   // iter(iter(x)) is iter(x), as in Python
    if (!IS_DICT(src) && iterCount(src) < 0) {
        nativeRaiseKelas(vm.galatTipe,
                         "iter() hanya menerima daftar, tupel, himpunan, teks, rentang, atau kamus");
        return KOSONG_VAL;
    }
    return OBJ_VAL(newIterator(src));
}

// lanjutkan(it) / lanjutkan(it, bawaan) - next(). Named lanjutkan because
// `lanjut` is already the continue keyword.
Value nativeFnLanjutkan(int argCount, Value* args) {
    if (argCount < 1 || argCount > 2) {
        nativeRaise("lanjutkan() memerlukan 1 atau 2 argumen, bukan %d", argCount);
        return KOSONG_VAL;
    }
    if (!IS_ITERATOR(args[0])) {
        nativeRaiseKelas(vm.galatTipe, "lanjutkan() memerlukan iterator dari iter().");
        return KOSONG_VAL;
    }
    ObjIterator* it = AS_ITERATOR(args[0]);
    // Re-read the length every call: mutating the source mid-iteration ends it
    // early instead of reading out of bounds.
    Value src = it->source;
    int count = IS_DICT(src) ? dictLength(AS_DICT(src)) : iterCount(src);
    if (count < 0 || it->index >= count) {
        if (argCount == 2) return args[1];
        nativeRaiseKelas(vm.hentiIterasi, "Iterator sudah habis.");
        return KOSONG_VAL;
    }
    int i = it->index++;
    return IS_DICT(src) ? dictKeyAt(AS_DICT(src), i) : iterAt(src, i);
}
