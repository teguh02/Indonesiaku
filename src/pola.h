#ifndef indonesiaku_pola_h
#define indonesiaku_pola_h

// Regular expressions for the `pola` module (Fase 6 gelombang C, D6).
//
// A Pike VM in the style of Russ Cox's re1: the pattern compiles to a small
// instruction program that runs all alternatives in lock-step, one input
// character at a time. Threads keep their priority order, so the match found
// is the same leftmost-first match Python's backtracking `re` reports -- but
// in time linear in the input, with no pathological (ReDoS) patterns.
// Backreferences and lookaround would need backtracking, so they are rejected.
//
// The engine works on code points (K13: indexes count characters, not bytes)
// and knows nothing about the VM; the natives at the bottom of pola.c wrap it.

#include <stddef.h>
#include <stdint.h>

#include "object.h"

// Flags, the values `pola.ABAIKAN_KAPITAL` etc. carry.
#define POLA_ABAIKAN_KAPITAL 1   // re.IGNORECASE (ASCII letters, like re.ASCII)
#define POLA_MULTIBARIS      2   // re.MULTILINE: ^ $ at every line
#define POLA_TITIK_SEMUA     4   // re.DOTALL: . also matches "\n"

// How much of the subject must match.
typedef enum {
    POLA_CARI,    // re.search: anywhere from `start`
    POLA_COCOK,   // re.match: anchored at `start`
    POLA_PENUH    // re.fullmatch: from `start` to the end
} PolaMode;

typedef struct PolaProgram PolaProgram;

// Compile `pattern` (UTF-8, `length` bytes). On error returns NULL and writes
// a message (in Indonesian) into `error`.
PolaProgram* polaCompile(const char* pattern, int length, int flags,
                         char* error, size_t errorSize);
void polaFree(PolaProgram* program);

// Number of capture groups, not counting group 0 (the whole match).
int polaGroupCount(const PolaProgram* program);
// Name of group `index` (1-based) when it was written (?P<name>...), else NULL.
const char* polaGroupName(const PolaProgram* program, int index);

// Run `program` over code points text[0..length). `caps` receives
// 2 * (groupCount + 1) character positions, -1 for a group that did not
// take part. `forbidEmptyAt` (or -1) rejects an EMPTY match starting there,
// which is how iteration steps past an empty match exactly like CPython.
// Returns 1 on a match, 0 on none, -1 when out of memory.
int polaExec(const PolaProgram* program, const uint32_t* text, int length,
             int start, PolaMode mode, int forbidEmptyAt, int* caps);

#ifndef POLA_TANPA_VM
// Natives used by pustaka/pola.idk (registered in initVM).
Value nativeFnPolaKompilasi(int argCount, Value* args);
Value nativeFnPolaCari(int argCount, Value* args);
Value nativeFnPolaSemua(int argCount, Value* args);
#endif

#endif
