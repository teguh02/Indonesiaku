# Rencana Fitur: `pastikan` (assert)

- **Status:** selesai
- **Target versi:** 0.7.0 (Fase 1)
- **Pemilik:** Teguh
- **Keputusan terkait:** K5 (hierarki galat)

## 1. Masalah

Tidak ada cara ringkas menyatakan "kondisi ini harus benar". Hari ini:

```indonesiaku
pastikan 1 == 1
# Variabel tidak terdefinisi 'pastikan'.
```

Penggantinya bertele-tele dan tidak punya tipe galat sendiri:

```indonesiaku
jika tidak (saldo >= 0) {
    naikkan "saldo negatif"        # teks biasa, bukan galat bertipe
}
```

Padahal kelas **`GalatPenegasan` sudah terdaftar** di `src/vm.c`
(`defineErrorClass("GalatPenegasan", vm.galat)`) sejak hierarki galat dibuat,
tetapi tidak pernah dipakai satu kali pun. Fiturnya setengah jadi.

Pemakaian utamanya mengajar: menunjukkan prakondisi pada mahasiswa, dan
menulis tes kecil di dalam contoh tanpa kerangka uji.

## 2. Sintaks usulan

```indonesiaku
pastikan saldo >= 0
pastikan saldo >= 0, "saldo tidak boleh negatif"
```

Mengikuti Python: `pastikan <kondisi>` dengan pesan opsional sesudah koma.
Tidak ada kurung wajib — `pastikan(x, y)` akan terbaca sebagai satu tupel
seperti kekeliruan klasik di Python, jadi panduan menyebutkan ini.

Kata kunci baru: `pastikan` (satu-satunya; tidak ada token lain).

## 3. Semantik

- Kondisi dinilai dengan aturan kebenaran Python yang sudah ada (`isFalsey`,
  K1): `0`, `""`, daftar/kamus kosong, `kosong`, `salah` → gagal.
- **Pesan hanya dinilai bila penegasan gagal** — persis Python. Ini sebabnya
  pesan dikompilasi di dalam cabang gagal, bukan dievaluasi lebih dulu.
- Gagal → menaikkan instance **`GalatPenegasan`**, sehingga bisa ditangkap:

  ```indonesiaku
  coba { pastikan salah, "x" } kecuali GalatPenegasan sebagai e { cetak(e.pesan) }
  ```

- Tanpa pesan, `.pesan` berisi `"Penegasan gagal."`. Dengan pesan, isinya
  hasil `ke_teks` dari ekspresi pesan (jadi `pastikan x, 42` sah).
- Berhasil → tidak meninggalkan apa pun di stack, tidak ada efek samping.
- **Tidak ada mode `-O` yang mematikan penegasan.** Python punya karena ada
  infrastruktur flag; di sini belum ada satu pun flag runtime, dan menambah
  satu demi fitur ini melanggar YAGNI. Dicatat sebagai keputusan sadar.

## 4. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `src/scanner.h` | `TOKEN_PASTIKAN` |
| `src/scanner.c` | `case 'p'` → `pastikan` (belum ada cabang `'p'` sama sekali) |
| `src/chunk.h` | `OP_PASTIKAN` |
| `src/compiler.c` | `assertStatement()` + cabang di `statement()` |
| `src/vm.c` | handler `OP_PASTIKAN`: bangun `GalatPenegasan`, lempar |
| `src/debug.c` | nama opcode |

Codegen (cabang gagal yang memegang pesan, agar pesan malas dinilai):

```text
  <kondisi>
  OP_JUMP_IF_FALSE -> gagal
  OP_POP                      # kondisi benar, buang
  OP_JUMP -> selesai
gagal:
  OP_POP                      # buang kondisi
  <pesan>  |  OP_KOSONG       # kosong = pakai pesan bawaan
  OP_PASTIKAN                 # pop pesan, lempar GalatPenegasan
selesai:
```

**Kenapa opcode baru, bukan desugaring.** Alternatif tanpa opcode adalah
memancarkan `OP_GET_GLOBAL "GalatPenegasan"` + `OP_CALL` + `OP_SET_PROPERTY`.
Ditolak: hasilnya bergantung pada variabel global yang bisa ditimpa pengguna
(`GalatPenegasan = 5` akan mengubah arti `pastikan`), dan kelas galat belum
punya `init` yang menerima pesan. Satu opcode lebih kecil sekaligus lebih
tahan banting.

**GC-safety:** handler membuat `ObjString` pesan lalu `ObjInstance` galat.
Alokasi kedua dapat memicu GC, jadi string pesan harus sudah ter-root (di
stack VM) sebelum instance dibuat — pola `newError` yang sudah ada di `vm.c`
diikuti apa adanya, bukan ditulis ulang.

## 5. Pengujian

- Golden `examples/pastikan.idk` + `tests/expected/pastikan.out`: penegasan
  lolos (tanpa dan dengan pesan), penegasan gagal yang **ditangkap**
  `kecuali GalatPenegasan`, tertangkap sebagai `Galat` induk, pesan non-teks,
  dan bukti bahwa ekspresi pesan **tidak dinilai** saat kondisi benar
  (pakai fungsi yang mencetak sesuatu sebagai pesan).
- Tes galat: `pastikan_gagal` (tanpa pesan) dan `pastikan_gagal_pesan`
  (dengan pesan) — keduanya harus exit non-nol dengan stderr tetap.
- `-DDEBUG_STRESS_GC` + ASan: alokasi string+instance di jalur gagal.

### Hasil (2026-10-25)

```text
tests/test_all.sh                  107/107 PASSED
ASan+UBSan+DEBUG_STRESS_GC         bersih (300 iterasi penegasan gagal)
```

Koreksi terhadap §7: dugaan bahwa `pastikan(kondisi, pesan)` akan diam-diam
selalu lolos (kekeliruan klasik Python) **tidak berlaku di sini** — diuji dan
hasilnya galat kompilasi `Harapkan ')' setelah ekspresi`, karena `(a, b)`
bukan tupel selama tipe tupel belum ada. Panduan menyebutkan perilaku yang
sebenarnya, bukan analogi Python.

Sebagai efek samping, pencarian `.pesan` pada instance galat yang sebelumnya
ditulis inline di `throwValue` diekstrak menjadi `errorMessage()` dan kini
dipakai dua pemanggil.

## 6. Dampak dokumentasi

- [x] `website/panduan/03_PANDUAN_SINTAKS.md` — seksi Penegasan (`pastikan`)
- [x] `README.md` + `CHANGELOG.md`
- [x] `docs/ROADMAP.md` — butir `pastikan` Fase 1 → ✅
- [x] `docs/ANALISIS/02_PETA_FITUR_PYTHON.md` — `assert` → ✅
- [x] `editors/vscode-indonesiaku/` — KEYWORDS, tmLanguage, snippet `pastikan`
- [ ] `AGENTS.md` — kata kunci baru (tertunda: berkas terproteksi, perlu persetujuan pemilik repo)

## 7. Risiko & alternatif

**Kompatibilitas:** `pastikan` menjadi kata kunci, sehingga program lama yang
memakainya sebagai nama variabel/fungsi akan berhenti kompilasi. Dicek dengan
grep ke seluruh `examples/`, `pustaka/`, dan `tests/` sebelum implementasi —
bila ada pemakaian, nama itu harus diganti dalam perubahan yang sama.

**Alternatif ditolak:** menjadikan `pastikan` fungsi bawaan biasa. Itu akan
selalu menilai argumen pesan (Python tidak), dan tidak bisa dimatikan di masa
depan bila mode optimasi ditambahkan.
