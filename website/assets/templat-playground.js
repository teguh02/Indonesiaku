/* =====================================================================
   Template kode untuk playground.

   Setiap `kode` di sini WAJIB benar-benar berjalan di indk (exit 0).
   Jangan menulis template dari ingatan — jalankan dulu:
       node -e '...' | indk /dev/stdin
   atau pakai tests/test_templat_playground.sh yang memeriksa semuanya.

   `label` memakai kunci i18n (lihat assets/i18n.json, awalan pg_tpl_).
   ===================================================================== */
(function () {
  "use strict";

  window.INDK_TEMPLAT = [
    {
      id: "halo",
      label: "pg_tpl_halo",
      kode:
`# Program pertama: menyapa dunia
cetak("Halo, dunia!")
cetak("Selamat datang di Indonesiaku 🇮🇩")

nama = "Teguh"
cetak(f"Apa kabar, {nama}?")
`
    },
    {
      id: "percabangan",
      label: "pg_tpl_percabangan",
      kode:
`# Percabangan: jika / jika_lain / selain
fungsi nilai_huruf(angka) {
    jika angka >= 85 {
        kembalikan "A"
    } jika_lain angka >= 70 {
        kembalikan "B"
    } jika_lain angka >= 55 {
        kembalikan "C"
    } selain {
        kembalikan "D"
    }
}

untuk n dalam [92, 78, 60, 40] {
    cetak(n, "->", nilai_huruf(n))
}

# Ekspresi kondisional (satu baris)
umur = 20
cetak("lulus" jika umur >= 17 selain "belum cukup umur")
`
    },
    {
      id: "perulangan",
      label: "pg_tpl_perulangan",
      kode:
`# Perulangan: untuk ... dalam, selagi, hentikan, lanjut
cetak("-- untuk ... dalam --")
untuk buah dalam ["mangga", "jeruk", "pisang"] {
    cetak("suka", buah)
}

cetak("-- rentang --")
untuk i dalam rentang(1, 6) {
    cetak(i, "kuadratnya", i * i)
}

cetak("-- selagi --")
sisa = 3
selagi sisa > 0 {
    cetak("hitung mundur:", sisa)
    sisa = sisa - 1
}

cetak("-- lanjut & hentikan --")
untuk n dalam rentang(1, 10) {
    jika n % 2 == 0 { lanjut }
    jika n > 7 { hentikan }
    cetak("ganjil:", n)
}
`
    },
    {
      id: "fungsi",
      label: "pg_tpl_fungsi",
      kode:
`# Fungsi: parameter bawaan, argumen bernama, rekursi, closure
fungsi sapa(nama, salam="Halo") {
    kembalikan salam + ", " + nama + "!"
}

cetak(sapa("Budi"))
cetak(sapa("Ani", salam="Hai"))

# Rekursi
fungsi faktorial(n) {
    jika n <= 1 { kembalikan 1 }
    kembalikan n * faktorial(n - 1)
}
cetak("5! =", faktorial(5))

# Closure: fungsi yang mengingat keadaannya
fungsi buat_pencacah() {
    variabel n = 0
    fungsi naik() {
        nonlokal n
        n = n + 1
        kembalikan n
    }
    kembalikan naik
}

hitung = buat_pencacah()
cetak(hitung(), hitung(), hitung())

# Fungsi anonim sebagai nilai
kuadrat = fungsi(x) { kembalikan x * x }
cetak("kuadrat 7 =", kuadrat(7))
`
    },
    {
      id: "kelas",
      label: "pg_tpl_kelas",
      kode:
`# Kelas, pewarisan, dan super
kelas Hewan {
    init(nama) {
        diri.nama = nama
    }

    bersuara() {
        cetak(diri.nama, "mengeluarkan suara")
    }

    perkenalan() {
        cetak("Namaku", diri.nama)
    }
}

kelas Kucing < Hewan {
    init(nama, warna) {
        super.init(nama)
        diri.warna = warna
    }

    bersuara() {
        cetak(diri.nama, "berkata: meong!")
    }
}

hewan = Hewan("Hewan Umum")
hewan.bersuara()

kucing = Kucing("Oyen", "oranye")
kucing.perkenalan()
kucing.bersuara()
cetak("warnanya", kucing.warna)
`
    },
    {
      id: "koleksi",
      label: "pg_tpl_koleksi",
      kode:
`# Daftar, kamus, tupel, dan himpunan
angka = [5, 3, 9, 1, 7]
cetak("daftar     :", angka)
cetak("terurut    :", terurut(angka))
cetak("terbesar   :", max(5, 3, 9, 1, 7))
cetak("panjang    :", panjang(angka))
cetak("irisan 1:4 :", angka[1:4])
cetak("dibalik    :", angka[::-1])

# List comprehension
cetak("kuadrat    :", [x * x untuk x dalam angka])

# Kamus
siswa = {"nama": "Siti", "umur": 21, "jurusan": "Informatika"}
untuk k dalam kunci(siswa) {
    cetak(" ", k, "=", siswa[k])
}

# Tupel (tidak bisa diubah) dan himpunan (anggota unik)
titik = (3, 4)
cetak("tupel      :", titik)
cetak("himpunan   :", terurut(himpunan([1, 2, 2, 3, 3, 3])))
`
    },
    {
      id: "galat",
      label: "pg_tpl_galat",
      kode:
`# Menangani galat: coba / kecuali / akhirnya
fungsi bagi(a, b) {
    coba {
        kembalikan a / b
    } kecuali e {
        cetak("gagal membagi:", e)
        kembalikan kosong
    } akhirnya {
        cetak("(selesai mencoba", a, "/", b, ")")
    }
}

cetak("hasil:", bagi(10, 2))
cetak("hasil:", bagi(10, 0))

# Memunculkan galat sendiri
kelas GalatUmur < Galat {
    init(pesan) {
        diri.pesan = pesan
    }
}

fungsi periksa_umur(umur) {
    jika umur < 0 {
        naikkan GalatUmur("umur tidak boleh negatif")
    }
    cetak("umur sah:", umur)
}

coba {
    periksa_umur(17)
    periksa_umur(-5)
} kecuali GalatUmur sebagai e {
    cetak("tertangkap:", e.pesan)
}
`
    },
    {
      id: "masukan",
      label: "pg_tpl_masukan",
      kode:
`# Membaca masukan pengguna.
# Isi kotak "Input (stdin)" di bawah — satu baris untuk tiap input().
nama = input("Siapa namamu? ")
umur = ke_bulat(input("Berapa umurmu? "))

cetak("Halo,", nama)
cetak("Tahun depan umurmu", umur + 1)

jika umur >= 17 {
    cetak("Kamu sudah boleh punya KTP.")
} selain {
    cetak("Masih", 17 - umur, "tahun lagi menuju 17.")
}
`
    }
  ];
})();
