#!/usr/bin/env python3
"""
API playground Indonesiaku — menjalankan `indk` untuk kode dari browser.

Kontrak:
    POST /api/jalankan  {"kode": "...", "input": "..."}   (input opsional)
    → 200 {"stdout": "...", "stderr": "...", "kode_keluar": 0, "ms": 7}

`input` disalurkan sebagai stdin, sehingga `input("Nama: ")` di program
membaca satu baris per nilai (dipisah newline).

Batas keamanan (lapisan kedua setelah container):
  * ukuran body 64 KB, kode maks 64 KB
  * waktu eksekusi 3 s (wall clock) + 3 s CPU
  * memori 128 MB (RLIMIT_AS di POSIX)
  * ukuran berkas 1 MB (RLIMIT_FSIZE), 32 file descriptor
  * stdout/stderr dipotong 64 KB
  * rate limit 60 request/menit per IP
  * setiap eksekusi berjalan di direktori sementara sendiri, tanpa stdin,
    dan `pustaka/` hanya ditautkan (read-only).

Jalankan langsung untuk pengembangan:
    python3 server.py --port 8099 --static ../website
"""

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
import threading
import time
from collections import defaultdict, deque
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from urllib.parse import unquote, urlparse

try:  # hanya tersedia di POSIX
    import resource
except ImportError:  # pragma: no cover - Windows
    resource = None

MAX_BODY = 128 * 1024
MAX_KODE = 64 * 1024
MAX_INPUT = 64 * 1024
MAX_OUTPUT = 64 * 1024
TIMEOUT_DETIK = 3.0
BATAS_CPU_DETIK = 3
BATAS_MEMORI_BYTE = 128 * 1024 * 1024
BATAS_BERKAS_BYTE = 1024 * 1024
RATE_LIMIT = 60
RATE_JENDELA = 60.0

INDK = os.environ.get("INDK_BIN", "/usr/local/bin/indk")
PUSTAKA = os.environ.get("PUSTAKA_DIR", "/opt/indonesiaku/pustaka")
STATIC_DIR = None

_hits = defaultdict(deque)
_hits_lock = threading.Lock()

TIPE = {
    ".html": "text/html; charset=utf-8",
    ".css": "text/css; charset=utf-8",
    ".js": "application/javascript; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".md": "text/markdown; charset=utf-8",
    ".idk": "text/plain; charset=utf-8",
    ".svg": "image/svg+xml",
    ".png": "image/png",
    ".ico": "image/x-icon",
    ".woff2": "font/woff2",
}


def batasi_rate(ip):
    """True kalau masih di bawah batas."""
    sekarang = time.monotonic()
    with _hits_lock:
        q = _hits[ip]
        while q and sekarang - q[0] > RATE_JENDELA:
            q.popleft()
        if len(q) >= RATE_LIMIT:
            return False
        q.append(sekarang)
        return True


def batas_proses():
    """Dijalankan di proses anak sebelum exec (POSIX saja)."""
    if resource is None:
        return
    resource.setrlimit(resource.RLIMIT_CPU, (BATAS_CPU_DETIK, BATAS_CPU_DETIK))
    resource.setrlimit(resource.RLIMIT_AS, (BATAS_MEMORI_BYTE, BATAS_MEMORI_BYTE))
    resource.setrlimit(resource.RLIMIT_FSIZE, (BATAS_BERKAS_BYTE, BATAS_BERKAS_BYTE))
    resource.setrlimit(resource.RLIMIT_NOFILE, (32, 32))
    try:
        resource.setrlimit(resource.RLIMIT_NPROC, (16, 16))
    except (ValueError, OSError):
        pass


def jalankan_kode(kode, masukan=""):
    mulai = time.monotonic()
    kerja = tempfile.mkdtemp(prefix="indk-", dir="/tmp" if os.path.isdir("/tmp") else None)
    try:
        with open(os.path.join(kerja, "main.idk"), "w", encoding="utf-8") as f:
            f.write(kode)

        # stdin playground: satu baris per pembacaan input().
        stdin_path = os.path.join(kerja, "stdin.txt")
        with open(stdin_path, "w", encoding="utf-8", newline="\n") as f:
            f.write(masukan)

        # `impor "pustaka/..."` mencari relatif terhadap cwd.
        if os.path.isdir(PUSTAKA):
            tujuan = os.path.join(kerja, "pustaka")
            try:
                os.symlink(PUSTAKA, tujuan)
            except (OSError, NotImplementedError):
                # Windows tanpa hak symlink: salin (pustaka kecil, read-only).
                shutil.copytree(PUSTAKA, tujuan)

        env = {
            "PATH": "/usr/local/bin:/usr/bin:/bin",
            "HOME": kerja,
            "LANG": "C.UTF-8",
        }

        try:
            with open(stdin_path, "rb") as masuk:
                hasil = subprocess.run(
                    # --sandbox: tolak lingkungan, isi direktori, dan
                    # menjalankan program lain (src/sistem.c). Lapisan
                    # tambahan di atas kontainer read-only tanpa jaringan.
                    [INDK, "--sandbox", "main.idk"],
                    cwd=kerja,
                    env=env,
                    stdin=masuk,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    timeout=TIMEOUT_DETIK,
                    preexec_fn=batas_proses if resource is not None else None,
                )
            keluar = hasil.returncode
            stdout = hasil.stdout
            stderr = hasil.stderr
        except subprocess.TimeoutExpired as exc:
            keluar = 124
            stdout = exc.stdout or b""
            stderr = (exc.stderr or b"") + (
                b"\n[playground] waktu eksekusi habis (3 detik)\n"
            )

        def rapikan(data):
            teks = data.decode("utf-8", "replace")
            if len(teks) > MAX_OUTPUT:
                teks = teks[:MAX_OUTPUT] + "\n[... keluaran dipotong ...]\n"
            return teks

        return {
            "stdout": rapikan(stdout),
            "stderr": rapikan(stderr),
            "kode_keluar": keluar,
            "ms": int((time.monotonic() - mulai) * 1000),
        }
    finally:
        shutil.rmtree(kerja, ignore_errors=True)


class Handler(BaseHTTPRequestHandler):
    server_version = "IndonesiakuPlayground/1.0"
    sys_version = ""

    def log_message(self, fmt, *args):  # log ringkas ke stderr
        sys.stderr.write("%s - %s\n" % (self.address_string(), fmt % args))

    def _kirim_json(self, status, payload):
        body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.send_header("X-Content-Type-Options", "nosniff")
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path.startswith("/api/"):
            self._kirim_json(405, {"pesan": "Gunakan POST /api/jalankan."})
            return
        if not STATIC_DIR:
            self._kirim_json(404, {"pesan": "Hanya /api/jalankan yang tersedia."})
            return
        path = unquote(urlparse(self.path).path)
        if path.endswith("/"):
            path += "index.html"
        target = os.path.normpath(os.path.join(STATIC_DIR, path.lstrip("/")))
        if not target.startswith(os.path.abspath(STATIC_DIR)):
            self.send_error(403)
            return
        if not os.path.isfile(target):
            self.send_error(404)
            return
        ext = os.path.splitext(target)[1].lower()
        with open(target, "rb") as f:
            body = f.read()
        self.send_response(200)
        self.send_header("Content-Type", TIPE.get(ext, "application/octet-stream"))
        self.send_header("Content-Length", str(len(body)))
        self.send_header("X-Content-Type-Options", "nosniff")
        self.end_headers()
        self.wfile.write(body)

    def do_POST(self):
        if urlparse(self.path).path != "/api/jalankan":
            self._kirim_json(404, {"pesan": "Tidak ditemukan."})
            return

        ip = self.client_address[0]
        if not batasi_rate(ip):
            self._kirim_json(429, {"pesan": "Terlalu banyak permintaan. Coba lagi sebentar."})
            return

        try:
            panjang = int(self.headers.get("Content-Length", "0"))
        except ValueError:
            panjang = 0
        if panjang <= 0 or panjang > MAX_BODY:
            self._kirim_json(413, {"pesan": "Ukuran permintaan tidak valid (maks 64 KB)."})
            return

        try:
            data = json.loads(self.rfile.read(panjang).decode("utf-8"))
        except (UnicodeDecodeError, json.JSONDecodeError):
            self._kirim_json(400, {"pesan": "Body harus JSON: {\"kode\": \"...\"}."})
            return

        kode = data.get("kode") if isinstance(data, dict) else None
        if not isinstance(kode, str):
            self._kirim_json(400, {"pesan": "Field 'kode' harus berupa string."})
            return
        if len(kode.encode("utf-8")) > MAX_KODE:
            self._kirim_json(413, {"pesan": "Kode terlalu panjang (maks 64 KB)."})
            return

        masukan = data.get("input", "") if isinstance(data, dict) else ""
        if masukan is None:
            masukan = ""
        if not isinstance(masukan, str):
            self._kirim_json(400, {"pesan": "Field 'input' harus berupa string."})
            return
        if len(masukan.encode("utf-8")) > MAX_INPUT:
            self._kirim_json(413, {"pesan": "Input terlalu panjang (maks 64 KB)."})
            return

        try:
            hasil = jalankan_kode(kode, masukan)
        except FileNotFoundError:
            self._kirim_json(503, {"pesan": "Interpreter indk tidak ditemukan di server."})
            return
        except Exception as exc:  # noqa: BLE001 - laporkan, jangan matikan layanan
            self._kirim_json(500, {"pesan": "Gagal menjalankan kode: %s" % exc})
            return

        self._kirim_json(200, hasil)


def main():
    global STATIC_DIR
    ap = argparse.ArgumentParser(description="API playground Indonesiaku")
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=8099)
    ap.add_argument("--static", default=None, help="folder situs untuk mode pengembangan")
    args = ap.parse_args()

    if args.static:
        STATIC_DIR = os.path.abspath(args.static)
    if not os.path.isfile(INDK):
        sys.stderr.write("[playground] peringatan: %s tidak ditemukan\n" % INDK)

    httpd = ThreadingHTTPServer((args.host, args.port), Handler)
    sys.stderr.write(
        "[playground] mendengarkan di http://%s:%d (indk=%s, statis=%s)\n"
        % (args.host, args.port, INDK, STATIC_DIR or "-")
    )
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
