const vscode = require("vscode");

const KEYWORDS = [
  "akhirnya", "atau", "benar", "cetak", "coba", "dalam", "dan", "dari", "dengan", "diri",
  "fungsi", "hentikan", "impor", "jika", "jika_lain", "kecuali", "kelas", "kembalikan", "kosong",
  "lanjut", "lewati", "naikkan", "salah", "sebagai", "selagi", "selain", "super", "tidak", "untuk", "variabel"
];

const BUILTINS = [
  "abs", "acak", "acak_bulat", "ada_berkas", "akar", "atap", "baca", "baca_baris", "baca_semua",
  "buka_berkas", "bulat", "cetak", "chr", "format", "format_tanggal", "ganti", "hapus", "hapus_berkas",
  "hapus_kunci", "hapus_pada", "huruf_besar", "huruf_kecil", "input", "jam", "jenis", "kamus", "karakter",
  "ke_angka", "ke_teks", "kunci", "lantai", "max", "min", "ord", "pangkat", "panjang", "potong", "punya",
  "sisip", "soket_baca", "soket_dengar", "soket_terima", "soket_tulis", "soket_tutup", "tambah", "tambah_berkas",
  "tanggal", "tulis", "tulis_semua", "tutup", "waktu"
];

function item(label, kind, detail, insertText) {
  const result = new vscode.CompletionItem(label, kind);
  result.detail = detail;
  result.insertText = insertText || label;
  return result;
}

function activate(context) {
  const provider = vscode.languages.registerCompletionItemProvider("indonesiaku", {
    provideCompletionItems() {
      return [
        ...KEYWORDS.map((word) => item(word, vscode.CompletionItemKind.Keyword, "kata kunci Indonesiaku")),
        ...BUILTINS.map((name) => item(name, vscode.CompletionItemKind.Function, "fungsi bawaan Indonesiaku", `${name}($0)`))
      ];
    }
  });

  context.subscriptions.push(provider);
}

function deactivate() {}

module.exports = { activate, deactivate };
