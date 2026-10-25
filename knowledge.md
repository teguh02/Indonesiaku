# knowledge.md — Indonesiaku

## What this is

**Indonesiaku** is a Python-inspired programming language with Indonesian keywords,
implemented as a **bytecode virtual machine in C11** (architecture follows
*Crafting Interpreters* / clox):

```
.idk source → scanner → single-pass Pratt compiler → chunk (bytecode) → stack VM (+ mark-sweep GC)
```

No AST is built; the compiler emits bytecode directly. Version constant lives in
`src/common.h` (`INDK_VERSION`); currently `1.0.0`.

## Where the code lives

| Path | Contents |
|---|---|
| `src/main.c` | CLI entry point + REPL (multiline buffering via `braceDelta`) |
| `src/scanner.c` | Lexer (no INDENT/DEDENT — blocks use `{ }`) |
| `src/compiler.c` | Pratt parser + bytecode codegen, string escape handling |
| `src/chunk.c` | Bytecode container |
| `src/vm.c` | Interpreter loop, native registration (`initVM`), `impor` |
| `src/native.c` | All built-in functions |
| `src/value.c`, `src/object.c` | Value system and heap objects |
| `src/table.c` | Hash table (FNV-1a) |
| `src/memory.c` | Allocation + mark-sweep GC |
| `src/debug.c` | Bytecode disassembler |
| `pustaka/*.idk` | Standard library **written in Indonesiaku itself** |
| `examples/*.idk` | Example programs; most double as golden tests |
| `tests/` | Golden-output harness + CRUD/HTTP integration tests + benchmark + fuzzer |
| `website/` | Static site (`*.html` + `assets/`), user guides in `website/panduan/*.md`, and the playground client in `website/playground/` |
| `website/assets/` | `tailwind-config.js` (tokens), `app.js` (Alpine: theme/lang/copy), `i18n.json` (ID/EN strings), `site.css` (prose, code tokens, editor) |
| `docs/` | Language **planning only** (roadmap, per-feature plans) — not user docs |
| `compiler/` | Windows installer (Inno Setup) — **not** the language compiler |
| `docker/images/` | Multi-stage Alpine Dockerfile |

## Commands

```bash
# Website preview (needs HTTP: fetch, CDN, Senna SPA)
python -m http.server 8000 -d website    # → http://localhost:8000/

# Playground with real execution (dev: site + /api/jalankan in one process)
mingw32-make                              # build indk first
INDK_BIN=$PWD/indk.exe PUSTAKA_DIR=$PWD/pustaka \
  python docker/playground/server.py --port 8099 --static website

# Production bundle: nginx + runner in one image, isolated networks
docker compose -f docker/playground/docker-compose.yml up -d --build   # :8080

# Build (all 11 src/*.c; -Isrc, links -lm)
make                     # → indk (POSIX) / indk.exe (Windows), adds -lws2_32 on Windows
make clean

# On this Windows dev box: no `make` on PATH — use
mingw32-make             # MSYS2 UCRT64: C:\msys64\ucrt64\bin

# Raw gcc (matches README)
gcc -Wall -Wextra -std=c11 -O2 -Isrc -o indk.exe \
    src/main.c src/chunk.c src/compiler.c src/debug.c src/memory.c \
    src/native.c src/object.c src/scanner.c src/table.c src/value.c src/vm.c -lm

# Run
./indk file.idk          # script; args after path → global list `argumen`
./indk                   # REPL
./indk -v | -h           # version / help

# Test (golden-output: stdout must match, stderr must be empty, exit 0)
powershell -ExecutionPolicy Bypass -File tests/test_all.ps1     # Windows
bash tests/test_all.sh                                          # Linux/macOS (builds first)

# Integration tests
bash tests/test_todo_cli.sh
bash tests/test_todo_server.sh        # drives the REST server with curl

# Website (static, publish dir = website/, self-hosted in Docker; no Netlify)
python -m http.server 8000 -d website   # local preview (fetch() needs http, not file://)
```

## Conventions

- C: 4-space indent, PascalCase types, `UPPER_CASE` constants, `snake_case`
  variables, same-line braces. `.editorconfig`: LF + 4 spaces (tabs in Makefile).
- Commit messages use Indonesian prefixes: `Tambah:`, `Perbaiki:`, `Ubah:`,
  `Docs:`, `Refactor:`, `Test:`, `Chore:`.
- Language keywords are Indonesian: `jika`/`jika_lain`/`selain`, `selagi`,
  `untuk ... dalam`, `fungsi`, `kembalikan`, `kelas`/`diri`/`super`, `coba`/
  `kecuali`, `naikkan`, `impor`, `cetak`, `benar`/`salah`/`kosong`.
- Stdlib functions are prefixed (`teks_`, `daftar_`, `mat_`, `kamus_`, `json_`,
  `http_`) because all globals share one namespace.
- Adding a golden test: drop `examples/foo.idk`, then capture **stdout only**:
  `indk.exe examples/foo.idk 1>tests/expected/foo.out 2>NUL`. `.gitignore`
  ignores `*.out` globally but re-includes `tests/expected/*.out` — keep that
  negation.

## Gotchas

- **Blocks use braces, not indentation.** There is no INDENT/DEDENT token.
  Older prose describing Python-style blocks is wrong.
- **Header changes must trigger full rebuild.** `%.o: %.c $(HEADERS)` depends on
  every header on purpose; stale partial builds silently miscompile. If
  behavior looks impossible, `make clean && make` first.
- **GC safety:** any C code that allocates while holding an unrooted object must
  keep it reachable — push it on the VM stack or use `peek` instead of `pop`
  before `tableSet`/`writeValueArray` (which can grow and trigger GC). See
  `makeConstant`, `concatenate`, `kunci`, `OP_INDEX_SET`, `sisip`.
- **GC verification:** build with `-DDEBUG_STRESS_GC` and run all examples —
  output must be byte-identical. Most valuable check for VM changes.
- **Implicit assignment** `x = 10` uses define-semantics (create-or-update) at
  any scope depth; only compound assignment (`+=`) requires the global to exist.
  Use `variabel x = ...` for function-locals that closures must capture.
- **Catchable vs fatal:** only user `naikkan` and native errors (`nativeRaise`)
  are caught by `kecuali`. Internal `runtimeError` prints and exits 70.
- **Adding string escapes touches two places:** `compiler.c string()` and the
  scanner's escape skipping.
- **`src/native.c` must stay in the Makefile `SOURCES`** — it was once missing
  and produced `undefined reference to nativeFn*`.
- **Some `examples/` files are libraries** (e.g. `pustaka_matematika.idk`) —
  they define only, so their golden output file is empty. Don't add top-level
  `cetak` to them.
- **Sockets:** Winsock on Windows (Makefile adds `-lws2_32`, `native.c` lazy
  `WSAStartup`), BSD on POSIX; servers bind `127.0.0.1` only. A blocking server
  can't be a golden test — it's driven by `curl` in CI and stopped via
  `/berhenti`.
- **`impor` shares one global namespace** and caches loaded files.
- **Docs site is HTTP-only:** `docs.html` `fetch()`es the `.md` files, so it
  won't work over `file://`. `{{embed:examples/foo.idk}}` markers pull code from
  `raw.githubusercontent.com` — the example must be pushed to `main` to appear.
- **`website/` is both the site and the user guides**; adding a doc to the
  reader's sidebar means editing the `DOCS` array in `website/docs.html` and
  putting the `.md` in `website/panduan/`.
- **Frontend has no build step**: Tailwind Play CDN + `assets/tailwind-config.js`,
  Alpine bootstrapped by `assets/app.js` after `assets/i18n.json` loads, and
  Senna.js 2.7.3 for SPA navigation over the `#konten` surface. All UI text goes
  in `i18n.json` (same keys under `id` and `en`).
- **Page inline scripts must be IIFEs** — Senna re-evaluates them on every SPA
  navigation, so top-level `const` would collide. Declare `window.__indkApp` /
  avoid new globals.
- **New pages need `marked` + `highlight.js` in `<head>`** (Senna does not swap
  `<head>`), otherwise the docs reader breaks when reached by SPA navigation.
- **Never `history.replaceState(null, …)`** on the docs page — it wipes Senna's
  state and breaks the back button. Preserve `history.state`.
- Dark mode is class-based (`darkMode: 'class'`); the inline pre-paint script in
  each `<head>` sets `dark` from `localStorage['indk-theme']` to avoid a flash.
- **`docs/` is planning, not documentation.** Do not put user-facing guides
  there; `docs/ROADMAP.md` is the single source of truth for the roadmap.
- **Playground backend lives in `docker/playground/`.** The page posts
  `{ kode, input }` to `/api/jalankan` and expects
  `{ stdout, stderr, kode_keluar, ms }`; `input` becomes stdin (one line per
  `input()` call). Dev mode: `python docker/playground/server.py --port 8099
  --static website`. Without the service the page shows "backend tidak
  tersedia".
- Root-level `test_*.idk` files are git-ignored scratch scripts, not the suite.
