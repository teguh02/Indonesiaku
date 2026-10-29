# Rencana Fitur: mengikuti pengalihan (redirect) HTTP

- **Status:** selesai
- **Target versi:** 1.4.0
- **Pemilik:** Teguh Rijanandi

## 1. Masalah

`http_ambil()` mengembalikan respons pengalihan apa adanya, bukan halaman
yang dituju:

```indonesiaku
variabel r = http_ambil("http://contoh.id/artikel")
cetak(r.status)   # 301 — bukan 200
cetak(r.badan)    # kosong atau "Moved Permanently"
```

Sebagian besar server mengalihkan: `/artikel` → `/artikel/`, domain tanpa
`www` → dengan `www`, dan HTTP → HTTPS. Tanpa dukungan ini pengguna harus
membaca header `Location` lalu memanggil ulang `http_ambil()` sendiri —
dan menulis sendiri perlindungan terhadap loop pengalihan.

Roadmap Fase 11 menyebut "mengikuti redirect" sebagai bagian klien HTTP.

## 2. Sintaks usulan

Tidak ada kata kunci atau builtin baru — satu parameter opsional:

```indonesiaku
http_ambil(url)                             # ikuti pengalihan (bawaan)
http_ambil(url, "GET", kosong, "", 0)       # jangan ikuti, kembalikan 301
http_ambil(url, "GET", kosong, "", 3)       # maksimal 3 lompatan
```

Bawaan `5`, sama seperti pustaka HTTP pada umumnya.

**Perubahan perilaku:** program lama yang memeriksa `r.status == 301` akan
melihat `200`. Ini dicatat di CHANGELOG. Mengikuti pengalihan adalah
perilaku yang diharapkan pengguna; yang lama praktis selalu bug.

## 3. Semantik

- Status yang diikuti: **301, 302, 303, 307, 308**.
- `303` → metode menjadi `GET` dan badan dibuang (aturan HTTP: respons
  "lihat di sini" selalu diambil dengan GET).
- `301`/`302` → **tetap mempertahankan metode**. Peramban mengubah POST
  menjadi GET di sini, tetapi itu kekeliruan historis yang dibekukan demi
  kompatibilitas; `307`/`308` ada justru untuk menghindarinya. Pustaka ini
  tidak menanggung beban sejarah peramban.
- `307`/`308` → metode dan badan dipertahankan.
- Tanpa header `Location` → respons pengalihan dikembalikan apa adanya,
  bukan galat: servernya yang salah, dan pemanggil berhak melihatnya.
- `Location` relatif (`/halaman`) diselesaikan terhadap URL sebelumnya.
- Melebihi batas lompatan → `GalatJaringan` yang menyebut rantai URL-nya.
- **Loop** (`/a` → `/b` → `/a`) terdeteksi lewat URL yang sudah dikunjungi →
  `GalatJaringan`, tidak menunggu batas lompatan habis.
- `r.url` berisi URL **terakhir**, bukan URL awal — supaya `Location`
  relatif berikutnya diselesaikan dengan benar.
- Hanya skema `http://` yang diikuti. `Location` ke `https://` → galat
  jelas, karena TLS belum ada (lihat §7).

## 4. Rencana implementasi

Seluruhnya di `pustaka/http.idk`. **Tidak ada perubahan C.**

| Bagian | Perubahan |
|---|---|
| `_http_satu_permintaan()` | isi `http_ambil()` lama, dipindah apa adanya |
| `http_ambil()` | gelung pengalihan + parameter `maks_pengalihan` |
| `_http_gabung_url()` | menyelesaikan `Location` relatif → absolut |

## 5. Pengujian

Butuh server HTTP sungguhan. `soket_dengar`/`soket_terima` sudah ada, jadi
server uji ditulis dengan Indonesiaku sendiri — tanpa dependensi baru.

- Golden: `examples/http_pengalihan.idk` + `.out` — server lokal yang
  melayani rantai pengalihan di proses yang sama.
- Kasus wajib: satu lompatan; rantai dua lompatan; `Location` relatif;
  `303` mengubah metode jadi GET; `307` mempertahankan POST; batas terlampaui;
  loop; `maks_pengalihan=0`; pengalihan tanpa `Location`.
- Tes galat untuk batas terlampaui.
- Jalankan dengan `-DDEBUG_STRESS_GC`.

**Risiko tes:** server di port tetap bisa bentrok di CI. Port diambil dari
rentang tinggi dan server dijalankan di proses yang sama dengan klien.

## 6. Dampak dokumentasi & tooling

- [x] `website/panduan/07_PUSTAKA_STANDAR.md` — bagian `http`
- [x] `CHANGELOG.md` (termasuk catatan perubahan perilaku) + `docs/ROADMAP.md`
- [ ] editor — tidak perlu, `http_ambil` fungsi pustaka, bukan builtin VM
- [ ] `03_PANDUAN_SINTAKS.md` — bukan sintaks
- [x] Berkas rencana ini → `selesai/`

## 7. Yang TIDAK dikerjakan di sini

Fase 11 juga menyebut **HTTPS/TLS**. Itu **terblokir dan tidak boleh
diam-diam dilewati**: Indonesiaku tidak menautkan OpenSSL (`LDFLAGS = -lm`,
`ldd` hanya libc/libm) dan SQLite pun di-*vendor* sebagai amalgamation agar
biner rilis tetap mandiri.

Menautkan OpenSSL akan mengakhiri properti itu — biner rilis mulai menuntut
`libssl` versi tertentu di mesin pengguna. Itu keputusan arsitektur, bukan
detail implementasi, dan pantas diputuskan tersendiri. Menulis TLS sendiri
bukan pilihan: kriptografi buatan sendiri di jalur jaringan adalah cara
klasik menciptakan lubang keamanan.

Tanpa TLS, klien ini hanya berguna untuk HTTP polos — jaringan internal dan
pengembangan lokal. Itu harus dikatakan terang-terangan di dokumentasi,
bukan disembunyikan.
