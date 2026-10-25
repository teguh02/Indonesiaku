# Rencana Fitur: `cetak` menjadi fungsi biasa

- **Status:** selesai
- **Target versi:** 0.7.0 (Fase 1)
- **Pemilik:** Teguh
- **Keputusan terkait:** K12

## 1. Masalah

`cetak` hari ini adalah **pernyataan**, bukan fungsi: `TOKEN_CETAK` →
`printStatement()` → `OP_PRINT`. Akibatnya ia bukan nilai:

```indonesiaku
variabel f = cetak           # Kesalahan di 'cetak': Harapkan ekspresi
peta(cetak, [1, 2, 3])       # tidak bisa dioper
bisa_dipanggil(cetak)        # parse error
```

Dan pemisah/akhiran barisnya tidak bisa diatur:

```indonesiaku
cetak("a", "b")              # selalu "a b" lalu newline
```

## 2. Sasaran (K12)

```indonesiaku
cetak(1, 2)                      # 1 2
cetak()                          # baris kosong
cetak("a", "b", pisah="-")       # a-b
cetak("a", akhir="")             # tanpa newline
variabel f = cetak               # nilai biasa
peta(cetak, [1, 2])              # dapat dioper
```

## 3. Penghalang: native belum menerima argumen bernama

`callValue()` menolak mentah-mentah:

```c
case OBJ_NATIVE:
    if (names != NULL) {
        runtimeError("Fungsi bawaan belum menerima argumen bernama.");
```

Sementara `call()` (jalur closure) memetakan nama → slot memakai
`fn->paramNames` dan arity tetap. Model itu **tidak cocok** untuk `cetak`:
jumlah argumen posisionalnya bebas, dan `pisah=`/`akhir=` bukan parameter
pada posisi tertentu.

Pilihan yang dipertimbangkan:

1. **Beri setiap native daftar `paramNames`.** Ditolak: 82 native harus
   didata ulang padahal hanya satu yang butuh, dan `cetak` tetap tidak muat
   karena variadic.
2. **Buat `cetak` closure di `pustaka/`.** Ditolak: menulis ke stdout tetap
   butuh native, jadi hanya memindahkan masalah sambil menambah lapisan.
3. **Teruskan nama apa adanya ke native** dan biarkan native yang peduli
   menafsirkannya. Dipilih: perubahan di satu tempat, native lain tidak
   tersentuh dan tetap menolak keyword seperti sebelumnya.

Bentuk (3): `callValue` menaruh `names` di `vm.nativeArgNames` sebelum
memanggil native, lalu membersihkannya. Native yang tidak membaca field itu
berperilaku persis seperti sekarang — tetapi karena mereka tidak lagi menolak
otomatis, penolakan dipindah ke pemeriksaan eksplisit: native yang tidak
mendukung keyword tetap harus melaporkan galat, bukan diam-diam mengabaikan
`pisah=`. Dipakai helper `nativeTolakArgumenBernama(nama)` yang dipanggil di
awal native non-`cetak` yang mungkin kena. **Default aman: menolak.**

Agar tidak perlu menyunting 82 native, penolakan dilakukan terpusat di
`callValue`: bila `names != NULL` dan native yang dipanggil **bukan** salah
satu yang terdaftar menerima keyword, galat lama tetap dikeluarkan. Daftar
penerima keyword saat ini: `cetak` saja.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/vm.h` | `Value* nativeArgNames` + `int nativeArgNameCount` |
| `src/vm.c` | `callValue` meneruskan nama hanya untuk native ber-keyword |
| `src/native.c` | `nativeFnCetak` baru |
| `src/vm.c` | `defineNative("cetak", nativeFnCetak)` |
| `src/compiler.c` | hapus `printStatement`, `TOKEN_CETAK` dari `statement()` |
| `src/scanner.c` | `cetak` bukan lagi kata kunci → identifier biasa |
| `src/scanner.h` | buang `TOKEN_CETAK` |
| `src/chunk.h`, `src/vm.c`, `src/debug.c` | buang `OP_PRINT` |

`OP_PRINT` **dihapus**, bukan dibiarkan menganggur: tidak ada lagi yang
memancarkannya, dan opcode mati adalah jebakan untuk pembaca berikutnya.

Pencetakan memakai `printValue()` yang sudah ada, jadi format nilai tidak
berubah sedikit pun.

## 5. Risiko

- **`cetak` jadi nama biasa** sehingga bisa ditimpa: `variabel cetak = 5`.
  Itu memang konsekuensi K12 dan sama seperti Python. Diuji agar tidak crash.
- **Regresi terluas di repo.** Hampir semua contoh dan tes memakai `cetak`;
  seluruh suite harus hijau tanpa mengubah satu pun berkas `.idk` yang ada.
- **Perbedaan `cetak` di dalam koleksi** yang disebut K12 (`cetak(["a"])`
  seharusnya `["a"]`, bukan `[a]`) **tidak** dikerjakan di sini: itu soal
  `repr` untuk elemen koleksi, butir terpisah dan akan mengubah banyak
  golden. Dicatat sebagai sisa K12.

## 6. Pengujian

Golden `examples/cetak_fungsi.idk`:
- `cetak()` kosong, satu argumen, banyak argumen
- `pisah=`, `akhir=`, keduanya sekaligus, `pisah=""`
- `cetak` sebagai nilai: disimpan di variabel, dioper ke fungsi,
  `bisa_dipanggil(cetak)`
- dipakai di dalam fungsi, closure, dan perulangan

Tes galat: `pisah=` bukan teks, nama keyword tak dikenal, keyword pada native
lain tetap ditolak.

Sanitizer ASan+UBSan+`DEBUG_STRESS_GC`.

## 7. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi Mencetak (4 blok diuji) + `cetak` dikeluarkan dari tabel Ringkasan Keywords
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — K12 → ✅, sisa `repr` didaftarkan sebagai butir ⬜ tersendiri
- [x] `docs/ANALISIS/03_KEPUTUSAN_DESAIN.md` — K12 terlaksana sebagian + sisa dicatat
- [x] `editors/vscode-indonesiaku/` — `cetak` pindah dari KEYWORDS ke BUILTINS (tmLanguage sudah benar sejak awal)
- [ ] `AGENTS.md` — tertunda: berkas diproteksi Hermes, perlu persetujuan interaktif pemilik repo

## 8. Hasil (2026-10-25)

```text
tests/test_all.sh                  120/120 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (300 iterasi, termasuk pisah= f-string)
```

Catatan dari pengerjaan:

- Tidak satu pun berkas `.idk` yang sudah ada perlu diubah: `cetak(...)` yang
  dulu pernyataan kini terbaca sebagai pemanggilan fungsi dengan hasil sama.
- `OP_PRINT` benar-benar dihapus dari `chunk.h`, `debug.c`, dan `vm.c`, bukan
  disisakan menganggur.
- `vm.nativeArgNames` ikut ditandai di `markRoots()`. Secara praktis ia sudah
  ter-root lewat tabel konstanta chunk, tetapi bergantung pada indirection itu
  rapuh terhadap perubahan di kemudian hari.
- Efek samping yang memang diinginkan K12: `variabel cetak = 5` kini sah, dan
  pemanggilan sesudahnya menghasilkan "Hanya dapat memanggil fungsi dan
  kelas." — sama seperti Python, diuji di `tests/galat/cetak_ditimpa.idk`.
