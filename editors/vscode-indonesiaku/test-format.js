// Uji rapikanLewatIndk() tanpa menjalankan VS Code.
// Jalankan: node editors/vscode-indonesiaku/test-format.js
//
// require("vscode") hanya ada di dalam editor, jadi modul itu di-stub lebih
// dulu lewat Module._load — tanpa ini require("./extension.js") gagal di CLI.
const assert = require("assert");
const path = require("path");
const fs = require("fs");
const os = require("os");
const Module = require("module");

const _load = Module._load;
Module._load = function (permintaan, induk, utama) {
  if (permintaan === "vscode") return {};          // stub: tidak dipakai helper
  return _load(permintaan, induk, utama);
};

const { rapikanLewatIndk } = require("./extension.js");

const AKAR = path.resolve(__dirname, "..", "..");
const BIN = path.join(AKAR, process.platform === "win32" ? "indk.exe" : "indk");
if (!fs.existsSync(BIN)) {
  console.error(`LEWATI: ${BIN} belum dibangun — jalankan 'make' lebih dulu.`);
  process.exit(0);
}

const tmp = fs.mkdtempSync(path.join(os.tmpdir(), "indk-fmt-"));
let lulus = 0;

function uji(nama, fn) {
  fn();
  lulus++;
  console.log(`  ok   ${nama}`);
}

// 1. Kode berantakan dirapikan sesuai aturan formatter.
uji("merapikan spasi operator dan pemanggilan", () => {
  const f = path.join(tmp, "berantakan.idk");
  fs.writeFileSync(f, "variabel a=1\ncetak( a )\n");
  assert.strictEqual(rapikanLewatIndk(require("child_process").spawnSync, BIN, f),
    "variabel a = 1\ncetak(a)\n");
});

// 2. Berkas asli TIDAK berubah — tanpa --tulis, rapikan hanya menulis ke stdout.
//    Ini yang membuat provider aman: VS Code sendiri yang menerapkan edit.
uji("tidak menyentuh berkas di disk", () => {
  const f = path.join(tmp, "utuh.idk");
  const asli = "variabel b=2\n";
  fs.writeFileSync(f, asli);
  rapikanLewatIndk(require("child_process").spawnSync, BIN, f);
  assert.strictEqual(fs.readFileSync(f, "utf8"), asli);
});

// 3. Kode yang sudah rapi keluar apa adanya (idempoten).
uji("kode rapi tidak berubah", () => {
  const f = path.join(tmp, "rapi.idk");
  const rapi = "variabel c = 3\ncetak(c)\n";
  fs.writeFileSync(f, rapi);
  assert.strictEqual(rapikanLewatIndk(require("child_process").spawnSync, BIN, f), rapi);
});

// 4. Komentar bertahan — inti dari TOKEN_KOMENTAR.
uji("komentar bertahan", () => {
  const f = path.join(tmp, "komentar.idk");
  fs.writeFileSync(f, "# catatan penting\nvariabel d=4\n");
  assert.ok(rapikanLewatIndk(require("child_process").spawnSync, BIN, f)
    .includes("# catatan penting"));
});

// 5. Galat interpreter menjadi exception, bukan keluaran rusak yang diam-diam
//    ditulis ke buffer pengguna.
uji("berkas tidak ada -> melempar galat", () => {
  assert.throws(() => rapikanLewatIndk(require("child_process").spawnSync, BIN,
    path.join(tmp, "tidak_ada.idk")));
});

// 6. Binari yang tidak ada juga melempar, bukan mengembalikan string kosong
//    (string kosong akan MENGHAPUS isi berkas pengguna).
uji("binari salah -> melempar galat", () => {
  assert.throws(() => rapikanLewatIndk(require("child_process").spawnSync,
    path.join(tmp, "indk-palsu"), path.join(tmp, "rapi.idk")));
});

fs.rmSync(tmp, { recursive: true, force: true });
console.log(`\nformatter VS Code: ${lulus}/6 lulus`);
