# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

The canonical agent guide is `AGENTS.md` (architecture, language semantics, GC-safety rules, CI, docs layout, conventions). It is imported here so there is one source of truth — update `AGENTS.md`, not this file, when those facts change.

@AGENTS.md

## Quick command reference (Windows dev box)

`make` is not on PATH here; use MSYS2 `mingw32-make` (`C:\msys64\ucrt64\bin`).

```bash
mingw32-make clean && mingw32-make          # build indk.exe (clean rebuild after any .h edit)
powershell -ExecutionPolicy Bypass -File tests/test_all.ps1   # full golden suite
./indk.exe examples/hello.idk               # run one script
```

**Run a single golden test** — there is no per-test flag; run the example and diff against its golden (stderr must be empty and exit code 0 for it to count as passing):

```bash
./indk.exe examples/<path>/<name>.idk 2>err.txt | diff --strip-trailing-cr - tests/expected/<name>.out; cat err.txt
```

**GC stress check** after touching allocation, stack, or collection-mutation code — rebuild with stress GC and rerun the suite; output must be unchanged:

```bash
gcc -std=c11 -O1 -Isrc -DDEBUG_STRESS_GC src/*.c -o indk.exe -lm -lws2_32
```

(Rebuild normally with `mingw32-make clean && mingw32-make` afterwards.)

Integration tests that need a running binary: `tests/test_todo_cli.ps1` (CLI CRUD over JSON) and `tests/test_todo_server.ps1` (REST server, driven by `curl`).
