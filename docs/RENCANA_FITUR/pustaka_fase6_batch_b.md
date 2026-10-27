# Rencana Fitur: Fase 6 — Pustaka standar, gelombang B (data & format)

- **Status:** disetujui
- **Target versi:** 1.2.0 (lanjutan Fase 6)
- **Pemilik:** kontributor Fase 6
- **Butir ROADMAP:** Fase 6 — `bita`, `base64`, `hash`, `csv`.

## 1. Masalah

Tidak ada cara bekerja dengan data biner: tidak ada modul byte, base64,
hash, maupun CSV. `base64`/`hash` sangat dibutuhkan aplikasi nyata (token,
sidik jari berkas, autentikasi) dan `csv` untuk pertukaran data tabular.

## 2. Keputusan desain (disepakati dengan pemilik)

1. **Byte direpresentasikan sebagai DAFTAR bulat 0..255**, bukan tipe objek
   baru. Nol perubahan bahasa: tidak ada `OBJ_BYTES`, tidak ada sintaks
   literal `b"..."`. Alasan: mempercepat Fase 6 dan menghindari keputusan
   tipe besar di tengah jalan; tipe `bita` tersendiri tetap mungkin nanti
   (dan dibutuhkan untuk `BLOB` SQLite) tanpa merusak modul ini.
   Helper `ke_bita(teks)`/`ke_teks(bita)` menghitung encoding UTF-8 di idk,
   memakai `ord`/`chr` + bitwise, sehingga aman untuk teks non-ASCII.
2. **`hash` ditulis di native C** (vendored seperti SQLite), bukan idk:
   md5/sha1/sha256/sha512 harus akurat bit-per-bit dan cepat; menulisnya di
   idk lambat dan rawan salah. Ini mengikuti prinsip katalog "C bila perlu
   cepat".

## 3. Sintaks usulan

```indonesiaku
impor bita, base64, hash
dari bita impor ke_bita, ke_teks

cetak(ke_bita("Halo"))            # [72, 97, 108, 111]
cetak(ke_teks([72, 97, 108, 111]))# Halo

cetak(base64.enkode("Halo"))      # SGFsbw==
cetak(base64.dekode("SGFsbw=="))  # Halo

cetak(hash.sha256("abc").heksa())
# ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
```

## 4. Semantik

- **`bita`** — `ke_bita(teks)` mengembalikan daftar bulat 0..255 (UTF-8);
  `ke_teks(bita)` mengembalikan teks (galat `GalatBita` bila urutannya bukan
  UTF-8 sah); `ke_heksa(bita)`/`dari_heksa(teks)` untuk representasi heksa.
- **`base64`** — `enkode(data)`/`dekode(teks)` menerima teks **atau** daftar
  byte; ada varian `_url` (alfabet `-`/`_`). Padding `=` dipertahankan
  (sama seperti Python). Panjang data yang tidak kelipatan 4 menaikkan
  `GalatBase64`.
- **`csv`** — `urai(teks)` → daftar baris berisi daftar kolom; `bentuk(baris)`
  → teks. Kutip ganda `"` melindungi pemisah/baris baru; `""` berarti satu
  kutip literal (RFC 4180). Ada `urai_kamus`/`bentuk_kamus` (baris pertama =
  header).
- **`hash`** — `md5`, `sha1`, `sha256`, `sha512` menerima teks (UTF-8) atau
  daftar byte, mengembalikan **objek hash** dengan metode `heksa()` (string
  heksa huruf kecil) dan `bita()` (daftar byte). Objek hash tidak dapat
  diubah dan dapat dibandingkan.

## 5. Rencana implementasi

| Bagian | Perubahan |
|---|---|
| `pustaka/bita.idk` | `ke_bita`, `ke_teks`, `ke_heksa`, `dari_heksa` (idk, UTF-8 manual) |
| `pustaka/base64.idk` | `enkode`, `dekode`, `enkode_url`, `dekode_url` (idk) |
| `pustaka/csv.idk` | `urai`, `bentuk`, `urai_kamus`, `bentuk_kamus` (idk) |
| `src/object.h`/`.c` | `OBJ_HASH`, `ObjHash` (digest 64 byte + panjang), `newHash`, format |
| `src/memory.c` | tandai/bebaskan `ObjHash` |
| `src/native.c` | implementasi md5/sha1/sha256/sha512 + 4 native + tabel metode `heksa`/`bita` |
| `src/vm.c` | `defineNative` keempatnya, `methodsForValue` untuk hash, tabel `hashMethods` |
| `src/value.c` | `hashValue`/`valuesEqual` untuk `ObjHash` |

**GC-safety:** `newHash` mengalokasi; digest disimpan inline (bukan pointer)
sehingga tidak ada pointer mentah yang perlu di-root. Native yang menerima
`Value` byte harus memakai `peek` bila mengalokasi daftar hasil.

## 6. Pengujian

- Golden: `examples/pustaka_bita.idk`, `pustaka_base64.idk`,
  `pustaka_csv.idk`, `pustaka_hash.idk`.
- Nilai uji hash diambil dari vektor standar: `sha256("")` =
  `e3b0c442...b855`, `sha256("abc")` = `ba7816bf...15ad`, `md5("abc")` =
  `900150983cd24fb0d6963f7d28e17f72`, `sha1("abc")` =
  `a9993e364706816aba3e25717850c26c9cd0d89d`, `sha512("abc")` = `ddaf35a1...`.
  Dibandingkan dengan `hashlib` Python saat menulis fixture.
- `base64`: vektor `"Halo"`, `""`, panjang 1/2/3 mod 4, varian URL, dan
  round-trip dengan `ke_bita`/`ke_teks` untuk teks non-ASCII (`"café 😀"`).
- `bita`: round-trip UTF-8 untuk ASCII, 2-byte, 3-byte, 4-byte (emoji);
  heksa bolak-balik; UTF-8 tak sah → `GalatBita`.
- `csv`: kutip berisi pemisah, kutip, dan baris baru; `urai`/`bentuk`
  round-trip; header kamus.
- Tes galat: `tests/galat/base64_panjang_salah`, `tests/galat/bita_utf8_salah`.
- **Sanitizer wajib** untuk `hash` (tipe objek baru): `-DDEBUG_STRESS_GC`
  `-fsanitize=address,undefined` atas seluruh golden **dan** `tests/galat/`.
- Editor VS Code: builtin baru `md5`/`sha1`/`sha256`/`sha512` masuk daftar
  BUILTINS yang dibangkitkan dari `defineNative()`.
- Blok `indonesiaku` baru di `website/panduan/07_PUSTAKA_STANDAR.md` ikut
  dijalankan `test_panduan.sh`; komentar hasilnya diverifikasi.

## 7. Dampak dokumentasi & tooling

- [ ] `website/panduan/07_PUSTAKA_STANDAR.md` (bagian 2.13–2.16 + builtin hash)
- [ ] `editors/vscode-indonesiaku/` — BUILTINS dibangkitkan ulang (4 builtin)
- [ ] `README.md`, `CHANGELOG.md`, `docs/ROADMAP.md`,
      `docs/ANALISIS/02_PETA_FITUR_PYTHON.md`
- [ ] `AGENTS.md` bila ada jebakan baru
- [ ] Berkas rencana ini → `selesai/`

## 8. Risiko & alternatif

- **Risiko: hash C salah bit.** Diredam dengan vektor uji standar + sanitizer
  (UBSan menangkap pergeseran tak terdefinisi pada implementasi 64-bit).
- **Risiko: base64 idk lambat** untuk berkas besar. Diterima: orientasi
  bahasa pendidikan; bila perlu, pindah ke C nanti tanpa mengubah API.
- **Alternatif ditolak:** membuat tipe `bita` sekarang. Dicatat sebagai
  pekerjaan lanjutan (dibutuhkan juga untuk `BLOB` SQLite), bukan dikerjakan
  diam-diam di sini.
- **Alternatif ditolak:** hash di idk. Terlalu lambat dan rawan; katalog
  sendiri mengizinkan C bila perlu cepat.
