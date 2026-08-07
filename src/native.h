#ifndef indk_native_h
#define indk_native_h

#include "common.h"
#include "value.h"
#include "object.h"

typedef struct {
    const char* name;
    NativeFn function;
} NativeFunction;

// ============================================================================
// FUNGSI MATEMATIKA
// ============================================================================

// min(a, b, ...) - Mengembalikan nilai terkecil
Value nativeFnMin(int argCount, Value* args);

// max(a, b, ...) - Mengembalikan nilai terbesar
Value nativeFnMax(int argCount, Value* args);

// abs(x) - Nilai absolut dari x
Value nativeFnAbs(int argCount, Value* args);

// akar(x) - Akar kuadrat dari x
Value nativeFnAkar(int argCount, Value* args);

// pangkat(a, b) - a pangkat b
Value nativeFnPangkat(int argCount, Value* args);

// bulat(x) - Membulatkan bilangan
Value nativeFnBulat(int argCount, Value* args);

// lantai(x) - Membulatkan ke bawah
Value nativeFnLantai(int argCount, Value* args);

// atap(x) - Membulatkan ke atas
Value nativeFnAtap(int argCount, Value* args);

// acak() - Bilangan acak 0-1
Value nativeFnAcak(int argCount, Value* args);

// ============================================================================
// FUNGSI STRING
// ============================================================================

// panjang(teks) - Mengembalikan panjang string
Value nativeFnPanjang(int argCount, Value* args);

// huruf_besar(teks) - Ubah ke huruf besar
Value nativeFnHurofBesar(int argCount, Value* args);

// huruf_kecil(teks) - Ubah ke huruf kecil
Value nativeFnHurofKecil(int argCount, Value* args);

// ganti(teks, cari, ganti) - Ganti substring
Value nativeFnGanti(int argCount, Value* args);

// ============================================================================
// FUNGSI LIST
// ============================================================================

// tambah(list, nilai) - Menambahkan nilai ke akhir list
Value nativeFnTambah(int argCount, Value* args);

// hapus(list) - Menghapus dan mengembalikan elemen terakhir list
Value nativeFnHapus(int argCount, Value* args);

// ============================================================================
// FUNGSI KAMUS (DICTIONARY)
// ============================================================================

// kamus() - Membuat kamus kosong
Value nativeFnKamus(int argCount, Value* args);

// punya(kamus, kunci) - Apakah kamus memiliki kunci tersebut
Value nativeFnPunya(int argCount, Value* args);

// hapus_kunci(kamus, kunci) - Menghapus pasangan kunci-nilai
Value nativeFnHapusKunci(int argCount, Value* args);

// kunci(kamus) - Mengembalikan list berisi semua kunci
Value nativeFnKunci(int argCount, Value* args);

// ============================================================================
// FUNGSI UTILITAS
// ============================================================================

// jenis(obj) - Mengembalikan tipe data dari objek
Value nativeFnJenis(int argCount, Value* args);

// input(prompt) - Membaca input dari pengguna
Value nativeFnInput(int argCount, Value* args);

// henti(sek) - Menunda eksekusi dalam detik
Value nativeFnHenti(int argCount, Value* args);

#endif
