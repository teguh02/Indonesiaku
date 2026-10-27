# Rencana Fitur: cerminan aritmetika (metode ajaib tahap 2B)

- **Status:** selesai
- **Target versi:** 1.1.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

Metode aritmetika K19 hanya dicari di operand **kiri**. Begitu objek ada di
sebelah kanan, operatornya langsung `GalatTipe` walaupun kelasnya jelas tahu
cara berhitung:

```python
kelas Rupiah {
    init(jumlah) { diri.jumlah = jumlah }
    _tambah_(lain) { kembalikan Rupiah(diri.jumlah + lain) }
}

cetak(Rupiah(5000) + 2500)   # Rupiah — jalan
cetak(2500 + Rupiah(5000))   # GalatTipe: Operator '+' tidak didukung antara 'bulat' dan 'Rupiah'
```

Perbandingan sudah tidak punya masalah ini: `b > a` dijawab
`a._kurang_dari_(b)` lewat cerminan yang sudah ada di `binaryMagic`.
Aritmetika memanggil `binaryMagic` yang sama tetapi menyerahkan `MAGIC_NONE`
sebagai cerminannya, jadi jalur itu mati.

Akibatnya `jumlah()` atas daftar objek, akumulator yang dimulai dari `0`, dan
rumus campuran seperti `2 * vektor` semuanya gagal.

## 2. Sintaks usulan

Tujuh metode baru, berawalan `k` (kanan):

```python
kelas Rupiah {
    init(jumlah) { diri.jumlah = jumlah }

    _tambah_(lain)  { kembalikan Rupiah(diri.jumlah + lain) }
    _ktambah_(lain) { kembalikan Rupiah(lain + diri.jumlah) }

    _kali_(n)  { kembalikan Rupiah(diri.jumlah * n) }
    _kkali_(n) { kembalikan Rupiah(n * diri.jumlah) }

    _teks_() { kembalikan f"Rp{diri.jumlah}" }
}

cetak(2500 + Rupiah(5000))   # Rp7500
cetak(3 * Rupiah(1000))      # Rp3000
```

| Metode cerminan | Dipanggil oleh |
|---|---|
| `_ktambah_` | `nilai + objek` |
| `_kkurangi_` | `nilai - objek` |
| `_kkali_` | `nilai * objek` |
| `_kbagi_` | `nilai / objek` |
| `_kbagi_bulat_` | `nilai // objek` |
| `_ksisa_` | `nilai % objek` |
| `_kpangkat_` | `nilai ** objek` |

Tidak ada kata kunci atau builtin baru.

## 3. Semantik

Urutan pencarian untuk `kiri OP kanan`, persis seperti cerminan perbandingan
yang sudah berjalan:

1. `kiri` instance dan punya metode langsung → `kiri._tambah_(kanan)`.
2. Kalau tidak, `kanan` instance dan punya metode cerminan →
   `kanan._ktambah_(kiri)`.
3. Kalau tidak dan salah satu operand instance → `GalatTipe` dengan pesan yang
   menyebut kedua tipe (perilaku sekarang, tidak berubah).

Metode cerminan menerima operand **kiri** sebagai argumennya, dan `diri` tetap
objek di kanan. Urutan ini penting untuk operator yang tidak komutatif:
`10 - Rupiah(3)` memanggil `Rupiah(3)._kkurangi_(10)`, jadi tubuh metodenya
yang menulis `lain - diri.jumlah`, bukan sebaliknya.

Operator gabungan (`+=`, `-=`, ...) memakai opcode yang sama, jadi ikut
otomatis tanpa perubahan.

**Yang sengaja TIDAK termasuk: `TidakDiterapkan`.** Di Python sentinel itu
dipakai saat `kiri` punya `__add__` tetapi tidak tahu cara menangani `kanan`,
sehingga penafsir mundur ke `kanan.__radd__`. Di sini `binaryMagic` masuk
lewat `call()` yang **mendorong frame** — nilai kembaliannya baru ada setelah
frame selesai, jauh dari titik keputusan di C. Mendukungnya berarti menandai
frame dengan "kalau hasilnya TidakDiterapkan, ulangi dengan cerminan", yaitu
keadaan baru di VM untuk kasus yang jarang (dua kelas berbeda, yang kiri punya
metodenya tapi menyerah). Ditunda sampai ada pemakaian nyata; sampai saat itu
kelas yang tidak bisa menangani operand cukup melempar `GalatTipe` sendiri.

## 4. Rencana implementasi

Perubahan kecil karena mesinnya sudah ada — `binaryMagic(kind, mirror)` sudah
menukar operand dan memanggil cerminan; aritmetika hanya belum memberinya
nama cerminan.

1. `src/object.h` — tambah `MAGIC_KTAMBAH`, `MAGIC_KKURANGI`, `MAGIC_KKALI`,
   `MAGIC_KBAGI`, `MAGIC_KBAGI_BULAT`, `MAGIC_KSISA`, `MAGIC_KPANGKAT`
   (jadi 28 slot, masih di bawah 32 bit bitmask).
2. `src/vm.c` — tujuh nama ke `MAGIC_NAMES`.
3. `src/vm.c` — tujuh `binaryMagicOrError(..., MAGIC_NONE, ...)` diganti
   cerminannya masing-masing.

## 5. Rencana uji

- Golden: `examples/oop/cerminan_aritmetika.idk` +
  `tests/expected/cerminan_aritmetika.out`.
- Galat: `tests/galat/cerminan_tanpa_metode.idk` + `.err` — objek di kanan
  tanpa metode cerminan tetap `GalatTipe` dengan nama kedua tipe.
- Kasus batas wajib: operator tidak komutatif (`-`, `/`, `//`, `%`, `**`)
  memberi hasil yang benar, bukan terbalik; metode langsung **menang** atas
  cerminan ketika keduanya ada; `jumlah()` atas daftar objek (mulai dari `0`);
  `+=` ikut jalan; cerminan yang melempar galat merambat.
- Jalankan juga dengan `-DDEBUG_STRESS_GC` + `-fsanitize=address,undefined`
  terhadap seluruh golden dan `tests/galat/` (menyentuh stack dan pemanggilan
  metode). `tests/test_all.sh` membangun ulang secara normal sehingga tidak
  memakai biner instrumentasi.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/03_PANDUAN_SINTAKS.md`
- [x] `editors/vscode-indonesiaku/extension.js` — MAGIC_METHODS
- [x] `editors/vscode-indonesiaku/syntaxes/indonesiaku.tmLanguage.json`
- [x] `editors/vscode-indonesiaku/snippets/indonesiaku.code-snippets`
- [x] `README.md`
- [x] `CHANGELOG.md` + `docs/ROADMAP.md`
      + `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [x] Berkas rencana ini: status → `selesai`, lalu pindah ke
      `docs/RENCANA_FITUR/selesai/`

## 7. Risiko & alternatif

**Kompatibilitas.** Jalur cerminan hanya menyala bila operand kanan adalah
instance yang mendefinisikan metode `_k*_`. Program lama tidak mungkin
terpengaruh: sebelumnya kasus itu selalu `GalatTipe`.

**Arah operand terbalik.** Risiko nyata ada di tubuh metode pengguna, bukan di
VM: menulis `_kkurangi_` seperti `_kurangi_` memberi hasil bertanda salah.
Karena itu golden wajib menguji semua operator tidak komutatif dengan angka
yang hasilnya berbeda bila dibalik (bukan `2 - 2`).

**Alternatif yang ditolak:** memakai satu nama metode untuk kedua arah dengan
parameter penanda posisi (`_tambah_(lain, di_kanan)`). Ditolak karena mengubah
tanda tangan metode yang sudah dirilis di 1.0 — melanggar K14.
