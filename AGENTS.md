# AGENTS.md

Indonesiaku: a Python-inspired language with Indonesian keywords, implemented as a bytecode VM in C11. Architecture follows *Crafting Interpreters* (clox): scanner → single-pass Pratt compiler → chunk (bytecode) → stack VM, with a mark-sweep garbage collector. Version constant: `INDK_VERSION` in `src/common.h` (currently `0.4.0`).

## Build

- `make` → produces `indk` (Linux/macOS) or `indk.exe` (Windows). C11, `-Isrc`, links `-lm`.
- `make clean` removes objects + binary (Windows and POSIX both handled; the recipe uses backslash paths on Windows because `del` mis-parses forward slashes).
- All 11 `src/*.c` are in the Makefile `SOURCES` (including `src/native.c` — it was historically missing, causing `undefined reference to nativeFn*` at link; do not remove it).
- Objects depend on all headers (`%.o: %.c $(HEADERS)`). Editing a `.h` triggers a full recompile — important because this codebase is tightly header-coupled and stale partial builds silently miscompile. If behavior seems impossible, do a clean rebuild before debugging.
- Local dev here: no `make` on PATH; use `mingw32-make` (MSYS2 UCRT64 at `C:\msys64\ucrt64\bin`). Raw gcc one-liner also works — see README.

## Run

- `./indk file.idk` — run a script; `./indk` — REPL; `./indk -v` — version; `./indk -h` — help.

## Test

- Windows: `powershell -ExecutionPolicy Bypass -File tests/test_all.ps1`
- Linux/macOS: `bash tests/test_all.sh` (also builds first)
- **Golden-output** harness: each `tests/expected/<name>.out` is compared against the stdout of the matching `examples/**/<name>.idk`. A test fails on stdout mismatch, non-zero exit, OR any stderr. Runners discover tests by iterating `tests/expected/*.out` and finding the source recursively under `examples/`.
- Add a test by dropping `examples/foo.idk` and generating its golden: run the binary and redirect stdout only (stderr must stay empty for a passing example), e.g. `indk.exe examples/foo.idk 1>tests/expected/foo.out 2>NUL`.
- `.gitignore` ignores `*.out` globally but re-includes `tests/expected/*.out` — keep that negation or goldens vanish.
- **Error tests** (`tests/galat/`): for behavior that must FAIL (compile errors, uncatchable runtime errors), add `tests/galat/foo.idk` + `tests/galat/foo.err`. Passes when the exit code is non-zero and stderr equals the `.err` file exactly; stdout is ignored. Generate with `indk.exe tests/galat/foo.idk 2>tests/galat/foo.err 1>NUL`. Both runners run these after the goldens.
- A golden must never contain a raw `\r` (e.g. printing an HTTP response): Windows output then has mixed `\r\r\n`, git classifies the file as binary (`git ls-files --eol` shows `i/-text`) and stores the Windows bytes, so Linux/macOS CI fails. Print such text with `ganti(t, "\r", "\\r")` (see `examples/http_pakai.idk`). Check new fixtures with `git ls-files --eol tests/expected/<name>.out` → must be `i/lf`.
- `src/native.c` defines `_DEFAULT_SOURCE` before its includes: `usleep` is POSIX, hidden by glibc under `-std=c11`, and GCC 14 rejects implicit declarations. Any new POSIX call needs the same care.
- Line endings: on Windows `indk.exe` writes CRLF and git (`autocrlf=true`) checks fixtures out as CRLF, so locally generated `.out`/`.err` files are CRLF; git normalizes them on commit and Linux CI compares LF to LF.
- Non-obvious: some `examples/` files are libraries meant to be `impor`-ed (e.g. `pustaka_matematika.idk`) — they define only, so their golden is an empty file. Don't add top-level `cetak` to them.

## GC / debugging the VM

- Mark-sweep GC is always on. To verify GC correctness after touching allocation/stack code, build with `-DDEBUG_STRESS_GC` (collects on every allocation) and run all examples — output must be unchanged. This is the single most valuable check for VM changes.
- Other flags in `src/common.h`: `DEBUG_LOG_GC`, `DEBUG_PRINT_CODE`, `DEBUG_TRACE_EXECUTION`.
- GC-safety rule: any C code that allocates while holding an unrooted object must keep it reachable (push it on the VM stack, or use `peek` instead of `pop`). See `makeConstant`, `concatenate`, `allocateString`, and the `kunci`/`OP_BUILD_LIST` handlers for the pattern.

## Language reality (differs from older docs)

- **Blocks use braces `{ }`, not Python indentation.** The scanner has no INDENT/DEDENT. `jika c { ... } jika_lain c2 { ... } selain { ... }`, `selagi c { ... }`, `fungsi f() { ... }`.
- Implicit assignment `x = 10` at top level defines a global. Inside a function, assignment to an unresolved name still targets a global — use `variabel x = ...` to create a function-local (required for closures to capture).
- Truthiness is Python's (K1, `isFalsey` in `vm.c`): `salah`, `kosong`, `0`, `""`, empty list/kamus/rentang are false; every instance is true. Collection emptiness uses live counts (`dictLength` skips tombstones).
- `rentang()` returns an `OBJ_RANGE` (start/stop/step doubles, no list). `untuk` is desugared to `OP_LIST_LEN` + `OP_INDEX_GET`, so any sequence those two opcodes accept is iterable — currently list, rentang, and teks (per character). Add a type there to make it iterable.
- `x dalam y` / `x tidak dalam y` compile to `OP_IN` (+ `OP_NOT`) at `PREC_COMPARISON` (`containsValue` in `vm.c`). `TOKEN_TIDAK` now has an infix rule (`notIn`), so because of B16 a line starting with `tidak` right after an expression is a compile error ("Harapkan 'dalam'"). Comparison chaining is not implemented yet.
- Implemented: closures/upvalues, lists `[..]` + indexing, dicts via `kamus()` + string indexing, `untuk x dalam list { }`, `hentikan`/`lanjut`, classes (`kelas`, `init`, `diri`, `<` inheritance, `super`), `naikkan` + `coba/kecuali`, `impor "file.idk"` (shared global namespace, cached).
- Catchable vs fatal: **every** `runtimeError` is catchable by `kecuali` (the caught value is the message string). When a handler is active, `runtimeError` only records the message (`vm.runtimeErrorPending`, no allocation) and `run()` returns; `interpret()` then unwinds via `throwValue`, pushes the message, and calls `run()` again. Only `fatalError` (stack underflow, bad jump offset — VM bugs) is never catchable. Natives still use `nativeRaise`. Uncaught errors print and exit 70.
- `coba` handlers: `OP_RETURN` drops handlers registered by the returning frame; `hentikan`/`lanjut` emit `OP_TRY_END` for each `coba` they leave (`Compiler.tryDepth` vs `LoopCtx.tryDepth`).
- `akhirnya` (finally): the single-pass compiler peeks ahead with `saveScanner`/`restoreScanner` (`scanTryClauses`) to learn whether `kecuali`/`akhirnya` follow, because a finally needs an OUTER handler registered before the body. The finally block runs with two hidden locals (raised value, rethrow flag) and rethrows via `OP_THROW`. `kembalikan`/`hentikan`/`lanjut` that leave such a `coba` re-compile the finally block inline at the jump (`exitTryBlocks` → `inlineFinallyBlock`, rewinding the scanner to the position saved in `Compiler.finallies[]`), like CPython's duplicated finally code. Errors during that re-compile are silenced (`inlineFinallyDepth`) because the in-place copy reports them. A returned value is evaluated first and parked in a hidden local. `semanticError` reports well-formed-but-forbidden code without panic-mode cascades.
- Statements are NOT newline-terminated: the scanner drops `\n` (there is no `TOKEN_NEWLINE`), so a line starting with `(`, `[` or `-` continues the previous expression (B16, planned for Fase 1). `;` is an optional statement separator. `TOKEN_COLON` is scanned but reserved for slicing/dict literals.
- Strings are UTF-8 bytes but the language counts **characters**: `ObjString.charCount` is computed at allocation; `panjang`, `teks[i]`, `karakter`, `potong` go through `utf8ByteOffset` (O(1) when `charCount == length`, i.e. ASCII). `ord`/`chr` use code points.
- `cetak(a, b)` compiles to `OP_PRINT <count>`: one line, space-separated. Numbers print via `numberToString` as the shortest round-trip repr (integral values < 1e16 without `.0`).
- String literals process escape sequences (`\n \t \r \" \\ \'`) in the compiler's `string()` (compiler.c); the scanner skips escaped chars so `\"` doesn't end a string. Adding new escapes = edit both.
- Builtins live in `src/native.c`, registered in `initVM` (`src/vm.c`). Families: math, string, list, dict, conversion (`ke_angka`/`ke_teks`/`format`), file I/O (`buka_berkas`/`baca`/`tulis`/...), date/time (`waktu`/`tanggal`), TCP sockets (`soket_dengar`/`soket_terima`/`soket_baca`/`soket_tulis`/`soket_tutup`), util. `OBJ_FILE` and `OBJ_SOCKET` handles auto-close on GC.
- Sockets: Winsock on Windows (Makefile adds `-lws2_32`; native.c lazy-inits `WSAStartup`), BSD sockets on POSIX; servers listen on `127.0.0.1` only. `examples/todo_server.idk` is a REST HTTP server built on `pustaka/http.idk` (request parse + response build) + `pustaka/json.idk`. Integration-tested in CI (`tests/test_todo_server.{sh,ps1}`, job `todo-http`) — a blocking server can't be a golden test, so it's driven by `curl` and shut down via the `/berhenti` endpoint.
- Command-line args after the script path are exposed as the global list `argumen`.
- Standard library written **in the language itself** lives in `pustaka/` (`teks.idk`, `daftar.idk`, `matematika.idk`, `kamus.idk`, `json.idk`, `http.idk`), imported via `impor "pustaka/..."`. Functions are prefixed (`teks_`, `daftar_`, `mat_`, `kamus_`, `json_`, `http_`) because globals share one namespace. They build on the C primitives `karakter`/`potong` (string char/substring), `ord`/`chr`, and first-class functions.
- Implicit assignment `x = 10` to a global-target name uses define-semantics (create-or-update) at ANY scope depth — so it works inside `jika`/`selagi`/`untuk` bodies (which are their own scopes). Only compound assignment (`+=` etc.) requires the global to already exist.
- GC-safety when mutating collections: keep operands on the VM stack (`peek`, not `pop`) until after `tableSet`/`writeValueArray`, which can grow and trigger a GC. `OP_INDEX_SET` for dicts and the `sisip` native follow this.
- `examples/todo_cli.idk` is the Todo-list JSON DB demo (CRUD over a JSON file via `pustaka/json.idk`). Integration-tested in CI by `tests/test_todo_cli.{sh,ps1}` (job `todo-crud`) plus golden tests for `json_pakai`/`todo_demo`.
- Operators follow Python: `//` floors (`OP_FLOOR_DIVIDE`), `%` takes the divisor's sign, `**` is right-associative and binds tighter than unary minus (`PREC_POWER` > `PREC_UNARY`), `tidak` binds looser than comparisons (`PREC_NOT`). Compound assignment (`+=`, `//=`, ...) is table-driven via `matchCompoundAssign` and its right side is a full `expression()`. Regression: `examples/operator_aritmetika.idk`.
- Python-style keyword arguments `f(nama=nilai)` are rejected at compile time (`callArgStart` in `compiler.c`) because an assignment-as-argument used to silently create globals. Parenthesized `f((x = 1))` is still allowed. Remove this check when real keyword arguments land.
- Max call depth `FRAMES_MAX` is 1000 (Python's default); runtime tracebacks collapse identical consecutive frames into `[baris sebelumnya diulang N kali lagi]`.
- Non-obvious VM facts: `<`/`>` compare strings via `strcmp` (not just numbers). String indexing `teks[i]` (read-only; strings immutable) shares the `OP_INDEX_GET` path with list/dict. `variabel` declared inside a `jika`/`selagi`/`untuk` body is properly scoped (each body gets `beginScope`/`endScope`); a regression once leaked such locals past the loop — see `examples/lingkup_variabel.idk`.
- The REPL (`main.c`) buffers input across lines until braces balance, so multi-line `fungsi`/`kelas`/`jika` blocks work; `braceDelta` ignores braces in strings/comments.

## CI

- `.github/workflows/test.yml` jobs: `test-linux`/`test-macos`/`test-windows` (build + golden tests, gating), **`todo-crud`** (CLI CRUD integration via `tests/test_todo_cli.sh`), **`todo-http`** (REST server integration via `tests/test_todo_server.sh`, driven by `curl`), **`sanitizers`** (ASan+UBSan with `-DDEBUG_STRESS_GC`), **`valgrind`**, **`coverage`** (lcov artifact), **`fuzz`** (short libFuzzer run on `tests/fuzz_compile.c` — compiles the front end only, does NOT execute, so infinite-loop programs can't cause timeouts). `continue-on-error` was removed from build/test steps — CI red now means a real failure.
- Non-Linux `sanitizers`/`valgrind`/`fuzz` jobs build via raw gcc/clang (no `-lws2_32`; on Linux sockets are in libc). The gating Windows job builds via `make` and DOES link `-lws2_32`.
- `.github/workflows/release.yml`: on `v*.*.*` tags; runs the test suite before publishing artifacts.

## Layout gotchas

- `compiler/` is **not** the language compiler — it's the Windows installer (Inno Setup `.iss` + assets). `dist/` holds prebuilt `Indonesiaku-Setup-*.exe`. All language implementation lives in `src/`.
- Root-level `test_*.idk` files are ad-hoc scratch scripts (git-ignored), not the test suite.
- `src/` map: `main.c` CLI/REPL · `scanner.c` lexer · `compiler.c` single-pass parser+codegen (Pratt) · `chunk.c` bytecode · `vm.c` interpreter loop + native registration + import · `native.c` builtins · `value.c`/`object.c` values & heap objects · `table.c` hash table · `memory.c` allocation + GC · `debug.c` disassembler.
- `docker/images/Dockerfile` is multi-stage (build → slim Alpine runtime, ~12.6 MB); it bundles the `indk` binary + `pustaka/` + `examples/`. Image is `teguh02/indonesiaku:<version>` and `:latest` on Docker Hub. Keep the `LABEL version` in sync with `INDK_VERSION`.

## Website (`website/`) vs language planning (`docs/`)

- **Two separate folders, two separate jobs.** `website/` = everything users see (usage docs, site, playground). `docs/` = internal language planning (roadmap, spec, per-feature plans). Never put user guides in `docs/` and never put roadmap/planning in `website/panduan/`.
- `website/` pages: `index.html` (landing), `about.html`, `author.html`, `docs.html` (markdown reader), `playground/index.html`, shared `assets/`, and the user guides in `website/panduan/*.md`. There is no Netlify anymore — the site is self-hosted.
- **Frontend stack (no build step):** Tailwind Play CDN + tokens in `assets/tailwind-config.js`, Alpine.js components in `assets/app.js`, and **Senna.js 2.7.3** (globals build) as the SPA engine. Every page is still a complete HTML document with its own URL (crawlable/SEO); Senna only swaps the `#konten` surface. Navigation must never rely on `#konten` being re-parsed by hand — Senna evaluates inline scripts inside the surface, so page scripts MUST be wrapped in an IIFE.
- **Bilingual:** all UI strings live in `assets/i18n.json` (`id` + `en`, identical keys). `app.js` fetches it **before** loading Alpine and only then calls `Alpine.data('shell', …)`; never add an Alpine `<script>` tag to a page. Long-form prose on `/about` and `/author` is duplicated as two `<article>` blocks toggled by `x-show="lang === 'id'"`.
- **Theme:** `darkMode: 'class'`; the inline pre-paint script in each `<head>` applies `dark` from `localStorage['indk-theme']` or `prefers-color-scheme`. Never move that script below the stylesheet or the page flashes light.
- **SPA gotchas:** new pages must also load `marked` + `highlight.js` in the `<head>` (Senna does not swap `<head>`, so the docs reader would break when reached by SPA navigation); docs code uses `history.replaceState(history.state, …)` because plain `null` wipes Senna's history entry and breaks the back button.
- `website/playground/index.html` is the playground client (plain HTML+JS, no build step). It `POST`s `{ kode }` to `/api/jalankan` and expects `{ stdout, stderr, kode_keluar, ms }`. The API is served by the same Docker bundle; until it exists the page shows a "backend tidak tersedia" state.
- Local preview must be over HTTP (fetch + CDN + Senna): `python -m http.server 8000 -d website`, then open `http://localhost:8000/`. `file://` will not work.

## Playground API (`docker/playground/`)

- `server.py` — stdlib HTTP service. `POST /api/jalankan {kode, input}` → `{stdout, stderr, kode_keluar, ms}`. It writes the code to a temp dir, symlinks `pustaka/` next to it (so `impor "pustaka/..."` works), writes `input` to a stdin file (one line per `input()` call), runs `indk main.idk`, and returns the result. Limits: 128 KB body, 64 KB each for code/input/output, 3 s wall + 3 s CPU, 128 MB address space, 1 MB file size, 32 fds, 60 req/min per IP. Pass `--static <dir>` to also serve the site (dev mode, no nginx needed).
- `nginx.conf` — serves `/usr/share/nginx/html` (the `website/` copy), forces `text/markdown` for `*.md` and `text/plain` for `*.idk`, and proxies `/api/` to `http://runner:8099`.
- `Dockerfile` — one image: builds `indk`, bundles `website/`, `pustaka/`, nginx, python3. `docker-compose.yml` runs it as two services: `web` (published port, attached to `publik` + `internal`) and `runner` (`internal` only, so no internet egress; read-only rootfs, tmpfs `/tmp`, non-root uid 10001, cap_drop ALL).
- Run: `docker compose -f docker/playground/docker-compose.yml up -d --build` (site on `:8080`).
- Dev without Docker: build `indk`, then
  `INDK_BIN=$PWD/indk PUSTAKA_DIR=$PWD/pustaka python3 docker/playground/server.py --port 8099 --static website`.
- The page hard-codes `const API_URL = "/api/jalankan"`; keep nginx and dev server on that path.
- `.dockerignore` at the repo root keeps build contexts small; it applies to `docker/images/` too (that image only needs `src/` + `Makefile`).
- `website/docs.html` renders the `.md` files at runtime with marked.js (fetched relative to `website/`, so entries are `panduan/<file>.md`) and syntax-highlights with highlight.js. To add a doc to the reader's sidebar, edit the `DOCS` array in `docs.html`; the `mdMatch` regex in `fixLinks()` must keep accepting `/` in paths.
- **Embedding example code in docs:** put a marker `{{embed:examples/foo.idk}}` on its own line in a `.md`. The reader fetches that file from `raw.githubusercontent.com/teguh02/Indonesiaku/main/...` so it always mirrors the repo — `examples/` is NOT copied into `docs/`. This means an embedded example must be committed+pushed to `main` before it shows on the live site.
- Deployment target is our own server, as ONE Docker bundle: the `website/` static files (docs + site) and the playground run together in a container; no Netlify, no `.netlify/`. Whatever serves the static files MUST send `text/markdown; charset=utf-8` for `*.md` and `text/plain; charset=utf-8` for `*.idk`, otherwise `fetch()` in the reader fails. The reader requires HTTP — it does not work over `file://`. For local preview: `python -m http.server 8000 -d website`.
- When language features change, keep these in sync: `README.md`, `CHANGELOG.md`, `website/panduan/*.md` (esp. `03_PANDUAN_SINTAKS.md` and `07_PUSTAKA_STANDAR.md`), `docs/ROADMAP.md`, this `AGENTS.md`, and the Dockerfile `LABEL version`.

## Conventions

- C style: 4-space indent, types PascalCase, constants UPPER_CASE, variables snake_case, braces same-line. `.editorconfig` enforces LF + 4-space (tabs for Makefile).
- Commit messages use Indonesian prefixes: `Tambah:` (add), `Perbaiki:` (fix), `Ubah:` (change), `Docs:`, `Refactor:`, `Test:`, `Chore:`.

## Docs content

- User guides (Indonesian) in `website/panduan/` (`README` + `01_PERKENALAN` … `07_PUSTAKA_STANDAR`). Syntax reference: `website/panduan/03_PANDUAN_SINTAKS.md`; builtin/stdlib reference: `website/panduan/07_PUSTAKA_STANDAR.md`. These `.md` files are what the live site renders.
- Language planning lives in `docs/`: `docs/README.md` (index + how to write a plan), `docs/ROADMAP.md` (SINGLE source of truth for roadmap — do not duplicate a roadmap in README/CHANGELOG), `docs/RENCANA_FITUR/TEMPLATE.md` (per-feature plan template).
- Repo/author facts to keep consistent: GitHub `teguh02/Indonesiaku`, email `teguhrijanandi02@gmail.com` (older docs used a wrong `teguhriyan` handle — fixed).
