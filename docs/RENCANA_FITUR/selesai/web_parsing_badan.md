# Rencana Fitur: parsing badan request (WEB-02)

- **Status:** selesai
- **Target versi:** 1.4.0
- **Roadmap:** Fase 11 langkah 4 (**WEB-02**)

## 1. Masalah

`pustaka/web.idk` (langkah 3) memberi `req["badan"]` sebagai **teks mentah**.
Setiap penangan harus mengurai sendiri. Akibatnya:

- Form HTML (`application/x-www-form-urlencoded`) harus diurai manual, padahal
  `_web_urai_query` sudah melakukannya persis untuk query string.
- JSON harus dipanggil `json_urai()` manual, dan galat JSON dari klien
  menjadi **500** (galat server) padahal seharusnya **400** (salah klien).
- `multipart/form-data` — upload berkas — tidak bisa diurai sama sekali.
- **Tidak ada batas ukuran.** Klien mengirim 2 GB, server menampungnya.

## 2. Temuan yang mengubah rencana

Laporan awal saya menyebut streaming **terblokir** karena `soket_baca()` adalah
"satu recv, maks ~64KB". Itu **salah** — yang benar: satu *panggilan* membaca
maks ~64KB, tetapi memanggilnya berulang sampai `Content-Length` terpenuhi
mengumpulkan badan utuh.

Dibuktikan sebelum menulis rencana ini: POST 200.000 byte diterima **utuh**
dalam **4 putaran**. Jadi tidak perlu mengubah C.

Yang tetap benar: gelung soket yang **sudah ada** di contoh hanya memanggil
`soket_baca()` **sekali**, jadi badan >64KB terpotong diam-diam. Itu bug nyata
di sisi pemanggil, dan perbaikannya milik rencana ini.

## 3. Sintaks

```indonesiaku
impor web

variabel app = web.Aplikasi()

app.rute("POST", "/simpan", fungsi(req) {
    variabel f = req["form"]          # kamus, dari urlencoded ATAU multipart
    kembalikan web.teks(201, f["nama"])
})

app.rute("POST", "/api", fungsi(req) {
    variabel data = req["json"]       # hasil json_urai, atau kosong
    kembalikan web.json(200, "{}")
})

app.rute("POST", "/unggah", fungsi(req) {
    variabel b = req["berkas"]["foto"]
    kembalikan web.teks(200, b["nama_berkas"] + " " + ke_teks(panjang(b["isi"])))
})
```

Membaca badan utuh dari soket:

```indonesiaku
variabel mentah = web.baca_permintaan(klien)     # gelung sampai lengkap
```

Batas:

```indonesiaku
variabel app = web.Aplikasi(1048576)   # maks 1 MB; bawaan 1 MB
```

## 4. Semantik

**Pemilihan parser** berdasarkan `Content-Type` (bagian sebelum `;`,
huruf kecil):

| Content-Type | Mengisi |
|---|---|
| `application/x-www-form-urlencoded` | `req["form"]` |
| `application/json` | `req["json"]` |
| `multipart/form-data` | `req["form"]` + `req["berkas"]` |
| lain / tidak ada | tidak satu pun; `req["badan"]` tetap ada |

Ketiga kunci **selalu ada** (`kamus()` kosong / `kosong`), supaya penangan
tidak perlu mengecek keberadaan kunci.

**Keputusan yang perlu ditegaskan:**

- Badan rusak dari klien → **400**, bukan 500. JSON tidak sah, boundary
  multipart hilang, `Content-Length` bukan angka: semuanya salah klien.
- Badan melebihi batas → **413**, dan **sebelum** badan dikumpulkan di memori
  (cek `Content-Length` dulu). Menolak setelah menelan 2 GB tidak ada gunanya.
- `Content-Length` tidak ada pada POST/PUT → badan dianggap kosong. `chunked`
  **tidak** didukung; ditolak **411**. Mengklaim dukungan chunked tanpa
  mengimplementasikannya lebih berbahaya daripada menolak terang-terangan.
- Nama field berulang (`a=1&a=2`) → **yang terakhir menang**, sama dengan
  perilaku query string yang sudah ada. Konsisten mengalahkan "benar".
- Berkas disimpan di **memori** sebagai teks, bukan ke disk. Dengan batas 1 MB
  bawaan ini aman; untuk upload besar butuh penulisan bertahap ke disk —
  di luar lingkup, dicatat sebagai batas.

**Multipart** diurai dengan memisah pada `--boundary`, lalu tiap bagian
dipisah header/isi pada `\r\n\r\n`, dan `Content-Disposition` dibaca untuk
`name=` dan `filename=`. Bagian dengan `filename=` masuk `req["berkas"]`,
sisanya `req["form"]`.

## 5. Berkas tersentuh

| Berkas | Perubahan |
|---|---|
| `pustaka/web.idk` | `baca_permintaan()`, parser badan, batas ukuran |
| `examples/web_badan.idk` + `tests/expected/web_badan.out` | golden |
| `tests/bantu/server_badan.idk`, `tests/web_badan_server.sh` | tes HTTP sungguhan |
| `tests/test_all.sh` | daftarkan skrip |
| `website/panduan/07_PUSTAKA_STANDAR.md` | dokumentasi |
| `CHANGELOG.md`, `docs/ROADMAP.md` | entri + centang langkah 4 |

## 6. Rencana uji

Golden (tanpa jaringan): urlencoded, JSON, multipart satu/dua field,
multipart dengan berkas, Content-Type tak dikenal, JSON rusak → 400,
badan melebihi batas → 413, chunked → 411, dekode persen di form.

HTTP sungguhan (curl, proses terpisah):
- `curl --data` → form terbaca
- `curl -F` → multipart + berkas terbaca
- `curl -H 'Content-Type: application/json' -d` → JSON terbaca
- **upload > 64KB** → terbaca utuh (membuktikan gelung baca, bukan satu recv)
- upload melebihi batas → 413

## 7. Yang TIDAK dikerjakan

- `Transfer-Encoding: chunked` → ditolak 411.
- Streaming ke disk; berkas ditahan di memori, dibatasi ukuran.
- `multipart/mixed` bersarang.
