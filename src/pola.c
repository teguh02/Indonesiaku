// Regular expressions for the `pola` module -- see pola.h for the design.
//
// Pipeline: pattern -> parser (recursive descent, builds a small node tree)
// -> code generator (Pike VM instructions) -> polaExec.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pola.h"

#define POLA_MAX_INST   20000   // compiled program size cap (counted repeats expand)
#define POLA_MAX_GROUPS 99
#define POLA_MAX_REPEAT 1000    // largest {m,n} bound, like a sane subset of sre
#define POLA_MAX_DEPTH  200     // parser nesting: ((((...)))) must not blow the C stack

// ---------------------------------------------------------------------------
// Character classes
// ---------------------------------------------------------------------------

// Shorthand classes that can appear inside [...] as well as on their own.
#define KELAS_D  1   // \d
#define KELAS_ND 2   // \D
#define KELAS_W  4   // \w
#define KELAS_NW 8   // \W
#define KELAS_S  16  // \s
#define KELAS_NS 32  // \S

typedef struct {
    uint32_t* ranges;   // pairs lo, hi (inclusive)
    int count;          // number of pairs
    int capacity;
    int shorthands;     // KELAS_* bits
    int negated;
} Kelas;

static int isDigitAscii(uint32_t c) { return c >= '0' && c <= '9'; }
static int isWordAscii(uint32_t c) {
    return isDigitAscii(c) || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}
static int isSpaceAscii(uint32_t c) {
    return c == ' ' || (c >= '\t' && c <= '\r');
}
static uint32_t foldAscii(uint32_t c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}
static uint32_t swapCaseAscii(uint32_t c) {
    if (c >= 'A' && c <= 'Z') return c + 32;
    if (c >= 'a' && c <= 'z') return c - 32;
    return c;
}

static int kelasRawContains(const Kelas* k, uint32_t c) {
    for (int i = 0; i < k->count; i++) {
        if (c >= k->ranges[2 * i] && c <= k->ranges[2 * i + 1]) return 1;
    }
    int s = k->shorthands;
    if ((s & KELAS_D) && isDigitAscii(c)) return 1;
    if ((s & KELAS_ND) && !isDigitAscii(c)) return 1;
    if ((s & KELAS_W) && isWordAscii(c)) return 1;
    if ((s & KELAS_NW) && !isWordAscii(c)) return 1;
    if ((s & KELAS_S) && isSpaceAscii(c)) return 1;
    if ((s & KELAS_NS) && !isSpaceAscii(c)) return 1;
    return 0;
}

static int kelasContains(const Kelas* k, uint32_t c, int fold) {
    int in = kelasRawContains(k, c);
    if (!in && fold && swapCaseAscii(c) != c) in = kelasRawContains(k, swapCaseAscii(c));
    return k->negated ? !in : in;
}

// ---------------------------------------------------------------------------
// Program
// ---------------------------------------------------------------------------

typedef enum {
    I_CHAR,     // c
    I_ANY,      // any character except "\n"
    I_ANYNL,    // any character
    I_CLASS,    // x = class index
    I_SPLIT,    // try x first, then y
    I_JMP,      // x
    I_SAVE,     // x = capture slot
    I_ASSERT,   // x = assertion kind
    I_PROGRESS, // no progress since caps[x] was saved -> jump to y, else pc + 1
    I_MATCH
} Op;

typedef enum {
    A_BOL,        // ^
    A_EOL,        // $
    A_START,      // \A
    A_END,        // \Z
    A_WORD,       // \b
    A_NOT_WORD    // \B
} Assertion;

typedef struct {
    uint8_t op;
    uint32_t c;
    int x;
    int y;
} Inst;

struct PolaProgram {
    Inst* code;
    int count;
    int capacity;
    Kelas* classes;
    int classCount;
    int classCapacity;
    int groups;          // capture groups, excluding 0
    char** names;        // names[1..groups], NULL when unnamed
    int flags;
    int loopSlots;       // hidden slots after the capture slots (see genLoop)
};

// ---------------------------------------------------------------------------
// Parser -> node tree
// ---------------------------------------------------------------------------

typedef enum { N_EMPTY, N_CHAR, N_ANY, N_CLASS, N_ASSERT, N_CAT, N_ALT, N_REPEAT, N_GROUP } NodeKind;

typedef struct Node {
    NodeKind kind;
    uint32_t c;          // N_CHAR
    int index;           // N_CLASS class index, N_ASSERT kind, N_GROUP group (0 = non-capturing)
    int min, max;        // N_REPEAT, max -1 = unbounded
    int greedy;
    struct Node* left;   // N_CAT/N_ALT left, N_REPEAT/N_GROUP child
    struct Node* right;
} Node;

typedef struct {
    const uint32_t* p;   // pattern code points
    int n;
    int pos;
    int depth;
    PolaProgram* prog;
    char* error;
    size_t errorSize;
    int failed;
    Node** nodes;        // every node allocated, freed together
    int nodeCount;
    int nodeCapacity;
} Parser;

static void fail(Parser* ps, const char* message) {
    if (ps->failed) return;
    ps->failed = 1;
    snprintf(ps->error, ps->errorSize, "%s (posisi %d)", message, ps->pos);
}

static Node* newNode(Parser* ps, NodeKind kind) {
    if (ps->nodeCount == ps->nodeCapacity) {
        int capacity = ps->nodeCapacity < 16 ? 16 : ps->nodeCapacity * 2;
        Node** grown = realloc(ps->nodes, sizeof(Node*) * (size_t)capacity);
        if (grown == NULL) { fail(ps, "Memori habis saat mengompilasi pola"); return NULL; }
        ps->nodes = grown;
        ps->nodeCapacity = capacity;
    }
    Node* node = calloc(1, sizeof(Node));
    if (node == NULL) { fail(ps, "Memori habis saat mengompilasi pola"); return NULL; }
    node->kind = kind;
    ps->nodes[ps->nodeCount++] = node;
    return node;
}

static int atEnd(Parser* ps) { return ps->pos >= ps->n; }
static uint32_t peekc(Parser* ps) { return ps->pos < ps->n ? ps->p[ps->pos] : 0; }

static int kelasBaru(Parser* ps) {
    PolaProgram* prog = ps->prog;
    if (prog->classCount == prog->classCapacity) {
        int capacity = prog->classCapacity < 4 ? 4 : prog->classCapacity * 2;
        Kelas* grown = realloc(prog->classes, sizeof(Kelas) * (size_t)capacity);
        if (grown == NULL) { fail(ps, "Memori habis saat mengompilasi pola"); return -1; }
        prog->classes = grown;
        prog->classCapacity = capacity;
    }
    Kelas* k = &prog->classes[prog->classCount];
    memset(k, 0, sizeof(Kelas));
    return prog->classCount++;
}

static void classAddRange(Parser* ps, int index, uint32_t lo, uint32_t hi) {
    Kelas* k = &ps->prog->classes[index];
    if (k->count == k->capacity) {
        int capacity = k->capacity < 4 ? 4 : k->capacity * 2;
        uint32_t* grown = realloc(k->ranges, sizeof(uint32_t) * 2 * (size_t)capacity);
        if (grown == NULL) { fail(ps, "Memori habis saat mengompilasi pola"); return; }
        k->ranges = grown;
        k->capacity = capacity;
    }
    k->ranges[2 * k->count] = lo;
    k->ranges[2 * k->count + 1] = hi;
    k->count++;
}

static int hexValue(uint32_t c) {
    if (c >= '0' && c <= '9') return (int)(c - '0');
    if (c >= 'a' && c <= 'f') return (int)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (int)(c - 'A' + 10);
    return -1;
}

// \xhh, \uhhhh, \Uhhhhhhhh -- the backslash and letter are already consumed.
static uint32_t parseHex(Parser* ps, int digits) {
    uint32_t value = 0;
    for (int i = 0; i < digits; i++) {
        int h = atEnd(ps) ? -1 : hexValue(peekc(ps));
        if (h < 0) { fail(ps, "Escape heksadesimal tidak lengkap"); return 0; }
        value = value * 16 + (uint32_t)h;
        ps->pos++;
    }
    if (value > 0x10FFFF) fail(ps, "Code point di luar jangkauan Unicode");
    return value;
}

// An escape that stands for ONE character (used both outside and inside
// classes). Returns 1 and sets *out, or 0 when `c` is not such an escape.
static int simpleEscape(Parser* ps, uint32_t c, uint32_t* out) {
    switch (c) {
        case 'a': *out = 7; return 1;
        case 'f': *out = 12; return 1;
        case 'n': *out = '\n'; return 1;
        case 'r': *out = '\r'; return 1;
        case 't': *out = '\t'; return 1;
        case 'v': *out = 11; return 1;
        case '0': *out = 0; return 1;
        case 'x': *out = parseHex(ps, 2); return 1;
        case 'u': *out = parseHex(ps, 4); return 1;
        case 'U': *out = parseHex(ps, 8); return 1;
        default: break;
    }
    if (c < 128 && !isWordAscii(c)) { *out = c; return 1; }   // \. \* \\ \- ...
    return 0;
}

static int shorthandBit(uint32_t c) {
    switch (c) {
        case 'd': return KELAS_D;
        case 'D': return KELAS_ND;
        case 'w': return KELAS_W;
        case 'W': return KELAS_NW;
        case 's': return KELAS_S;
        case 'S': return KELAS_NS;
        default: return 0;
    }
}

static void rejectEscape(Parser* ps, uint32_t c) {
    if (c >= '1' && c <= '9') {
        fail(ps, "Backreference (\\1) tidak didukung: pola harus bisa dicocokkan tanpa backtracking");
    } else {
        fail(ps, "Escape tidak dikenal");
    }
}

// [...]: the '[' is consumed.
static Node* parseClass(Parser* ps) {
    int index = kelasBaru(ps);
    if (index < 0) return NULL;
    if (peekc(ps) == '^' && !atEnd(ps)) {
        ps->prog->classes[index].negated = 1;
        ps->pos++;
    }
    int first = 1;
    for (;;) {
        if (atEnd(ps)) { fail(ps, "Kelas karakter '[' tidak ditutup"); return NULL; }
        uint32_t c = ps->p[ps->pos++];
        if (c == ']' && !first) break;
        first = 0;
        uint32_t lo;
        if (c == '\\') {
            if (atEnd(ps)) { fail(ps, "Pola diakhiri '\\'"); return NULL; }
            uint32_t e = ps->p[ps->pos++];
            int bit = shorthandBit(e);
            if (bit) {
                ps->prog->classes[index].shorthands |= bit;
                continue;
            }
            if (e == 'b') {
                lo = 8;                       // \b inside a class is backspace
            } else if (!simpleEscape(ps, e, &lo)) {
                rejectEscape(ps, e);
                return NULL;
            }
        } else {
            lo = c;
        }
        uint32_t hi = lo;
        // A range needs a '-' that is not the last character before ']'.
        if (peekc(ps) == '-' && ps->pos + 1 < ps->n && ps->p[ps->pos + 1] != ']') {
            ps->pos++;
            uint32_t h = ps->p[ps->pos++];
            if (h == '\\') {
                if (atEnd(ps)) { fail(ps, "Pola diakhiri '\\'"); return NULL; }
                uint32_t e = ps->p[ps->pos++];
                if (shorthandBit(e) || !simpleEscape(ps, e, &h)) {
                    fail(ps, "Rentang kelas karakter tidak sah");
                    return NULL;
                }
            }
            hi = h;
            if (hi < lo) { fail(ps, "Rentang kelas karakter terbalik"); return NULL; }
        }
        classAddRange(ps, index, lo, hi);
        if (ps->failed) return NULL;
    }
    Node* node = newNode(ps, N_CLASS);
    if (node != NULL) node->index = index;
    return node;
}

static Node* parseAlternation(Parser* ps);

static Node* parseGroup(Parser* ps) {
    // '(' consumed.
    int capture = 1;
    char* name = NULL;
    if (peekc(ps) == '?' && !atEnd(ps)) {
        ps->pos++;
        uint32_t c = peekc(ps);
        if (c == ':') {
            ps->pos++;
            capture = 0;
        } else if (c == 'P' && ps->pos + 1 < ps->n && ps->p[ps->pos + 1] == '<') {
            ps->pos += 2;
            int start = ps->pos;
            while (!atEnd(ps) && isWordAscii(peekc(ps))) ps->pos++;
            if (ps->pos == start || peekc(ps) != '>' || isDigitAscii(ps->p[start])) {
                fail(ps, "Nama grup tidak sah");
                return NULL;
            }
            int len = ps->pos - start;
            name = malloc((size_t)len + 1);
            if (name == NULL) { fail(ps, "Memori habis saat mengompilasi pola"); return NULL; }
            for (int i = 0; i < len; i++) name[i] = (char)ps->p[start + i];
            name[len] = '\0';
            ps->pos++;   // '>'
            for (int g = 1; g <= ps->prog->groups; g++) {
                if (ps->prog->names[g] != NULL && strcmp(ps->prog->names[g], name) == 0) {
                    free(name);
                    fail(ps, "Nama grup dipakai dua kali");
                    return NULL;
                }
            }
        } else if (c == '=' || c == '!' || (c == '<' && ps->pos + 1 < ps->n &&
                   (ps->p[ps->pos + 1] == '=' || ps->p[ps->pos + 1] == '!'))) {
            fail(ps, "Lookaround (?=...) tidak didukung: pola harus bisa dicocokkan tanpa backtracking");
            return NULL;
        } else if (c == 'P' && ps->pos + 1 < ps->n && ps->p[ps->pos + 1] == '=') {
            fail(ps, "Backreference (?P=nama) tidak didukung");
            return NULL;
        } else {
            fail(ps, "Bendera sebaris (?...) belum didukung; pakai argumen bendera");
            return NULL;
        }
    }
    int index = 0;
    if (capture) {
        if (ps->prog->groups >= POLA_MAX_GROUPS) {
            free(name);
            fail(ps, "Terlalu banyak grup");
            return NULL;
        }
        index = ++ps->prog->groups;
        char** grown = realloc(ps->prog->names, sizeof(char*) * (size_t)(index + 1));
        if (grown == NULL) { free(name); fail(ps, "Memori habis saat mengompilasi pola"); return NULL; }
        ps->prog->names = grown;
        if (index == 1) grown[0] = NULL;
        grown[index] = name;
    }
    Node* inner = parseAlternation(ps);
    if (ps->failed) return NULL;
    if (peekc(ps) != ')' || atEnd(ps)) { fail(ps, "Tanda kurung '(' tidak ditutup"); return NULL; }
    ps->pos++;
    Node* node = newNode(ps, N_GROUP);
    if (node == NULL) return NULL;
    node->index = index;
    node->left = inner;
    return node;
}

static Node* parseAtom(Parser* ps) {
    uint32_t c = ps->p[ps->pos++];
    Node* node;
    switch (c) {
        case '(':
            if (++ps->depth > POLA_MAX_DEPTH) { fail(ps, "Grup bersarang terlalu dalam"); return NULL; }
            node = parseGroup(ps);
            ps->depth--;
            return node;
        case '[':
            return parseClass(ps);
        case '.':
            return newNode(ps, N_ANY);
        case '^':
        case '$':
            node = newNode(ps, N_ASSERT);
            if (node != NULL) node->index = c == '^' ? A_BOL : A_EOL;
            return node;
        case '\\': {
            if (atEnd(ps)) { fail(ps, "Pola diakhiri '\\'"); return NULL; }
            uint32_t e = ps->p[ps->pos++];
            int bit = shorthandBit(e);
            if (bit) {
                int index = kelasBaru(ps);
                if (index < 0) return NULL;
                ps->prog->classes[index].shorthands = bit;
                node = newNode(ps, N_CLASS);
                if (node != NULL) node->index = index;
                return node;
            }
            int assertion = -1;
            if (e == 'b') assertion = A_WORD;
            if (e == 'B') assertion = A_NOT_WORD;
            if (e == 'A') assertion = A_START;
            if (e == 'Z') assertion = A_END;
            if (assertion >= 0) {
                node = newNode(ps, N_ASSERT);
                if (node != NULL) node->index = assertion;
                return node;
            }
            uint32_t literal;
            if (!simpleEscape(ps, e, &literal)) { rejectEscape(ps, e); return NULL; }
            node = newNode(ps, N_CHAR);
            if (node != NULL) node->c = literal;
            return node;
        }
        default:
            node = newNode(ps, N_CHAR);
            if (node != NULL) node->c = c;
            return node;
    }
}

// {m}, {m,}, {,n}, {m,n} after an atom. Returns 1 and fills min/max, or 0
// (and leaves pos untouched) when the brace is not a quantifier -- Python
// then reads '{' as a literal.
static int parseBraces(Parser* ps, int* min, int* max) {
    int save = ps->pos;
    ps->pos++;   // '{'
    int lo = -1, hi = -1;
    int value = 0, digits = 0;
    while (!atEnd(ps) && isDigitAscii(peekc(ps))) {
        value = value * 10 + (int)(peekc(ps) - '0');
        if (value > 100000) value = 100000;
        digits++;
        ps->pos++;
    }
    if (digits) lo = value;
    if (peekc(ps) == '}' && !atEnd(ps)) {
        if (!digits) { ps->pos = save; return 0; }
        ps->pos++;
        *min = lo;
        *max = lo;
        return 1;
    }
    if (peekc(ps) != ',' || atEnd(ps)) { ps->pos = save; return 0; }
    ps->pos++;
    value = 0;
    digits = 0;
    while (!atEnd(ps) && isDigitAscii(peekc(ps))) {
        value = value * 10 + (int)(peekc(ps) - '0');
        if (value > 100000) value = 100000;
        digits++;
        ps->pos++;
    }
    if (digits) hi = value;
    if (peekc(ps) != '}' || atEnd(ps)) { ps->pos = save; return 0; }
    ps->pos++;
    *min = lo < 0 ? 0 : lo;
    *max = hi;   // -1 = unbounded
    return 1;
}

static int isQuantifierStart(Parser* ps) {
    if (atEnd(ps)) return 0;
    uint32_t c = peekc(ps);
    if (c == '*' || c == '+' || c == '?') return 1;
    if (c == '{') {
        int save = ps->pos, mn, mx;
        int ok = parseBraces(ps, &mn, &mx);
        ps->pos = save;
        return ok;
    }
    return 0;
}

static Node* parseRepeat(Parser* ps) {
    if (isQuantifierStart(ps)) { fail(ps, "Tidak ada yang diulang"); return NULL; }
    Node* atom = parseAtom(ps);
    if (ps->failed) return NULL;
    if (!isQuantifierStart(ps)) return atom;
    if (atom->kind == N_ASSERT) { fail(ps, "Tidak ada yang diulang"); return NULL; }
    int min, max;
    uint32_t q = peekc(ps);
    if (q == '{') {
        parseBraces(ps, &min, &max);
        if (max >= 0 && max < min) { fail(ps, "Batas pengulangan {m,n} terbalik"); return NULL; }
        if (min > POLA_MAX_REPEAT || max > POLA_MAX_REPEAT) {
            fail(ps, "Batas pengulangan terlalu besar");
            return NULL;
        }
    } else {
        ps->pos++;
        min = q == '+' ? 1 : 0;
        max = q == '?' ? 1 : -1;
    }
    int greedy = 1;
    if (peekc(ps) == '?' && !atEnd(ps)) {
        ps->pos++;
        greedy = 0;
    }
    if (isQuantifierStart(ps)) { fail(ps, "Pengulangan ganda"); return NULL; }
    Node* node = newNode(ps, N_REPEAT);
    if (node == NULL) return NULL;
    node->left = atom;
    node->min = min;
    node->max = max;
    node->greedy = greedy;
    return node;
}

static Node* parseConcat(Parser* ps) {
    Node* result = NULL;
    while (!atEnd(ps) && peekc(ps) != '|' && peekc(ps) != ')') {
        Node* item = parseRepeat(ps);
        if (ps->failed) return NULL;
        if (result == NULL) {
            result = item;
        } else {
            Node* cat = newNode(ps, N_CAT);
            if (cat == NULL) return NULL;
            cat->left = result;
            cat->right = item;
            result = cat;
        }
    }
    return result != NULL ? result : newNode(ps, N_EMPTY);
}

static Node* parseAlternation(Parser* ps) {
    Node* left = parseConcat(ps);
    if (ps->failed) return NULL;
    while (!atEnd(ps) && peekc(ps) == '|') {
        ps->pos++;
        Node* right = parseConcat(ps);
        if (ps->failed) return NULL;
        Node* alt = newNode(ps, N_ALT);
        if (alt == NULL) return NULL;
        alt->left = left;
        alt->right = right;
        left = alt;
    }
    return left;
}

// ---------------------------------------------------------------------------
// Code generation
// ---------------------------------------------------------------------------

static int emit(Parser* ps, Op op) {
    PolaProgram* prog = ps->prog;
    if (prog->count >= POLA_MAX_INST) { fail(ps, "Pola terlalu besar"); return -1; }
    if (prog->count == prog->capacity) {
        int capacity = prog->capacity < 64 ? 64 : prog->capacity * 2;
        Inst* grown = realloc(prog->code, sizeof(Inst) * (size_t)capacity);
        if (grown == NULL) { fail(ps, "Memori habis saat mengompilasi pola"); return -1; }
        prog->code = grown;
        prog->capacity = capacity;
    }
    Inst* inst = &prog->code[prog->count];
    memset(inst, 0, sizeof(Inst));
    inst->op = (uint8_t)op;
    return prog->count++;
}

static void gen(Parser* ps, Node* node);

// Can `node` match the empty string?
static int nullable(const Node* node) {
    switch (node->kind) {
        case N_EMPTY:
        case N_ASSERT:
            return 1;
        case N_CHAR:
        case N_ANY:
        case N_CLASS:
            return 0;
        case N_CAT:
            return nullable(node->left) && nullable(node->right);
        case N_ALT:
            return nullable(node->left) || nullable(node->right);
        case N_REPEAT:
            return node->min == 0 || nullable(node->left);
        case N_GROUP:
            return nullable(node->left);
    }
    return 0;
}

// Hidden slot index for a loop's "position at iteration start" (after the
// 2 * (groups + 1) capture slots; final count known only after parsing, so
// these are numbered from the end and fixed up in polaCompile).
static int loopSlot(Parser* ps) {
    return -(++ps->prog->loopSlots);
}

// The body of an optional/repeated iteration. A NULLABLE body gets
//     SAVE hidden; body; PROGRESS hidden -> exit
// so an iteration that matched the empty string leaves the loop (keeping the
// groups it set) instead of looping -- what Python's sre does, and what makes
// (a|)* and (\S?)+ report the same groups as Python. Returns the PROGRESS
// pc whose exit target the caller patches, or -1.
static int genIteration(Parser* ps, Node* child, int slot) {
    if (slot == 0) {
        gen(ps, child);
        return -1;
    }
    int pc = emit(ps, I_SAVE);
    if (pc >= 0) ps->prog->code[pc].x = slot;
    gen(ps, child);
    int progress = emit(ps, I_PROGRESS);
    if (progress >= 0) ps->prog->code[progress].x = slot;
    return progress;
}

static void gen(Parser* ps, Node* node) {
    if (ps->failed) return;
    PolaProgram* prog = ps->prog;
    int pc;
    switch (node->kind) {
        case N_EMPTY:
            return;
        case N_CHAR:
            pc = emit(ps, I_CHAR);
            if (pc >= 0) prog->code[pc].c = node->c;
            return;
        case N_ANY:
            emit(ps, (prog->flags & POLA_TITIK_SEMUA) ? I_ANYNL : I_ANY);
            return;
        case N_CLASS:
            pc = emit(ps, I_CLASS);
            if (pc >= 0) prog->code[pc].x = node->index;
            return;
        case N_ASSERT:
            pc = emit(ps, I_ASSERT);
            if (pc >= 0) prog->code[pc].x = node->index;
            return;
        case N_CAT:
            gen(ps, node->left);
            gen(ps, node->right);
            return;
        case N_ALT: {
            int split = emit(ps, I_SPLIT);
            if (split < 0) return;
            prog->code[split].x = prog->count;
            gen(ps, node->left);
            int jmp = emit(ps, I_JMP);
            if (jmp < 0) return;
            prog->code[split].y = prog->count;
            gen(ps, node->right);
            prog->code[jmp].x = prog->count;
            return;
        }
        case N_GROUP:
            if (node->index == 0) {
                gen(ps, node->left);
                return;
            }
            pc = emit(ps, I_SAVE);
            if (pc >= 0) prog->code[pc].x = 2 * node->index;
            gen(ps, node->left);
            pc = emit(ps, I_SAVE);
            if (pc >= 0) prog->code[pc].x = 2 * node->index + 1;
            return;
        case N_REPEAT: {
            // Mandatory copies first: sre iterates `min` times even when an
            // iteration matches empty.
            for (int i = 0; i < node->min && !ps->failed; i++) gen(ps, node->left);
            int slot = nullable(node->left) ? loopSlot(ps) : 0;
            if (node->max < 0 && slot != 0) {
                // Star over a NULLABLE body: two alternating copies,
                //     LA: SPLIT A, out; A; PROGRESS -> out; JMP LB
                //     LB: SPLIT B, out; B; PROGRESS -> out; JMP LA
                // After a consuming iteration, sre tries one more iteration
                // that matches empty and then leaves the loop. With a single
                // copy that empty iteration would walk the very pcs the
                // previous iteration's tail just visited at this position,
                // and the Pike VM's dedupe would kill it. Alternating copies
                // never share pcs between consecutive iterations.
                int splits[2], bodies[2], progresses[2], jumps[2];
                for (int k = 0; k < 2 && !ps->failed; k++) {
                    splits[k] = emit(ps, I_SPLIT);
                    if (splits[k] < 0) return;
                    bodies[k] = prog->count;
                    progresses[k] = genIteration(ps, node->left, slot);
                    jumps[k] = emit(ps, I_JMP);
                    if (jumps[k] < 0) return;
                }
                if (ps->failed) return;
                int out = prog->count;
                for (int k = 0; k < 2; k++) {
                    prog->code[splits[k]].x = node->greedy ? bodies[k] : out;
                    prog->code[splits[k]].y = node->greedy ? out : bodies[k];
                    prog->code[progresses[k]].y = out;
                    prog->code[jumps[k]].x = splits[1 - k];
                }
            } else if (node->max < 0) {
                // Star: L: SPLIT body, out; body; JMP L
                int split = emit(ps, I_SPLIT);
                if (split < 0) return;
                int body = prog->count;
                int progress = genIteration(ps, node->left, slot);
                int jmp = emit(ps, I_JMP);
                if (jmp < 0) return;
                prog->code[jmp].x = split;
                int out = prog->count;
                prog->code[split].x = node->greedy ? body : out;
                prog->code[split].y = node->greedy ? out : body;
                if (progress >= 0) prog->code[progress].y = out;
            } else {
                // Optional copies: SPLIT body, next ... ; an empty iteration
                // skips every remaining copy.
                int optional = node->max - node->min;
                int* exits = optional > 0 ? malloc(sizeof(int) * (size_t)optional) : NULL;
                if (optional > 0 && exits == NULL) { fail(ps, "Memori habis saat mengompilasi pola"); return; }
                for (int i = 0; i < optional && !ps->failed; i++) {
                    int split = emit(ps, I_SPLIT);
                    if (split < 0) break;
                    int body = prog->count;
                    exits[i] = genIteration(ps, node->left, slot);
                    int next = prog->count;
                    prog->code[split].x = node->greedy ? body : next;
                    prog->code[split].y = node->greedy ? next : body;
                }
                for (int i = 0; i < optional && !ps->failed; i++) {
                    if (exits[i] >= 0) prog->code[exits[i]].y = prog->count;
                }
                free(exits);
            }
            return;
        }
    }
}

static int utf8Decode(const unsigned char* s, int length, int* i, uint32_t* out);

PolaProgram* polaCompile(const char* pattern, int length, int flags,
                         char* error, size_t errorSize) {
    PolaProgram* prog = calloc(1, sizeof(PolaProgram));
    if (prog == NULL) {
        snprintf(error, errorSize, "Memori habis saat mengompilasi pola");
        return NULL;
    }
    prog->flags = flags;

    uint32_t* cps = malloc(sizeof(uint32_t) * (size_t)(length > 0 ? length : 1));
    if (cps == NULL) {
        free(prog);
        snprintf(error, errorSize, "Memori habis saat mengompilasi pola");
        return NULL;
    }
    int n = 0;
    for (int i = 0; i < length;) {
        uint32_t c;
        utf8Decode((const unsigned char*)pattern, length, &i, &c);
        cps[n++] = c;
    }

    Parser ps;
    memset(&ps, 0, sizeof(ps));
    ps.p = cps;
    ps.n = n;
    ps.prog = prog;
    ps.error = error;
    ps.errorSize = errorSize;

    Node* root = parseAlternation(&ps);
    if (!ps.failed && !atEnd(&ps)) fail(&ps, "Tanda kurung ')' tanpa pasangan");
    if (!ps.failed) {
        int pc = emit(&ps, I_SAVE);
        if (pc >= 0) prog->code[pc].x = 0;
        gen(&ps, root);
        pc = emit(&ps, I_SAVE);
        if (pc >= 0) prog->code[pc].x = 1;
        emit(&ps, I_MATCH);
    }
    if (!ps.failed) {
        // Hidden loop slots were numbered -1, -2, ... because the group
        // count was still growing; place them after the capture slots.
        int base = 2 * (prog->groups + 1);
        for (int i = 0; i < prog->count; i++) {
            Inst* inst = &prog->code[i];
            if ((inst->op == I_SAVE || inst->op == I_PROGRESS) && inst->x < 0) {
                inst->x = base + (-inst->x - 1);
            }
        }
    }
    for (int i = 0; i < ps.nodeCount; i++) free(ps.nodes[i]);
    free(ps.nodes);
    free(cps);
    if (ps.failed) {
        polaFree(prog);
        return NULL;
    }
    return prog;
}

void polaFree(PolaProgram* prog) {
    if (prog == NULL) return;
    for (int i = 0; i < prog->classCount; i++) free(prog->classes[i].ranges);
    free(prog->classes);
    if (prog->names != NULL) {
        for (int g = 1; g <= prog->groups; g++) free(prog->names[g]);
        free(prog->names);
    }
    free(prog->code);
    free(prog);
}

int polaGroupCount(const PolaProgram* prog) { return prog->groups; }

const char* polaGroupName(const PolaProgram* prog, int index) {
    if (prog->names == NULL || index < 1 || index > prog->groups) return NULL;
    return prog->names[index];
}

// ---------------------------------------------------------------------------
// Pike VM
// ---------------------------------------------------------------------------

typedef struct {
    int count;
    int* pcs;      // pcs[i]
    int* caps;     // caps[i * nsave ...]
} ThreadList;

typedef struct {
    int pc;
    int slot;      // >= 0: restore caps[slot] = value instead of visiting
    int value;
} Frame;

static int assertionHolds(int kind, const uint32_t* text, int n, int pos, int flags) {
    switch (kind) {
        case A_BOL:
            return pos == 0 || ((flags & POLA_MULTIBARIS) && text[pos - 1] == '\n');
        case A_EOL:
            if (pos == n) return 1;
            if (flags & POLA_MULTIBARIS) return text[pos] == '\n';
            return pos == n - 1 && text[pos] == '\n';
        case A_START:
            return pos == 0;
        case A_END:
            return pos == n;
        case A_WORD:
        case A_NOT_WORD: {
            int before = pos > 0 && isWordAscii(text[pos - 1]);
            int after = pos < n && isWordAscii(text[pos]);
            return kind == A_WORD ? before != after : before == after;
        }
    }
    return 0;
}

typedef struct {
    const PolaProgram* prog;
    const uint32_t* text;
    int n;
    int nsave;
    int* visited;      // generation per pc
    int generation;
    Frame* stack;      // explicit stack: no C recursion for epsilon closures
    int* work;         // working capture array
} Machine;

// Follow every epsilon edge from `pc` and append the character-consuming /
// MATCH instructions it reaches to `list`, in priority order. Iterative so
// a long chain of SPLITs cannot overflow the C stack.
static void addThread(Machine* m, ThreadList* list, int startPc, const int* caps, int pos) {
    memcpy(m->work, caps, sizeof(int) * (size_t)m->nsave);
    int top = 0;
    m->stack[top++] = (Frame){startPc, -1, 0};
    while (top > 0) {
        Frame f = m->stack[--top];
        if (f.slot >= 0) {
            m->work[f.slot] = f.value;
            continue;
        }
        int pc = f.pc;
        if (m->visited[pc] == m->generation) continue;
        m->visited[pc] = m->generation;
        const Inst* inst = &m->prog->code[pc];
        switch (inst->op) {
            case I_JMP:
                m->stack[top++] = (Frame){inst->x, -1, 0};
                break;
            case I_SPLIT:
                // Pushed in reverse so x is explored first (higher priority).
                m->stack[top++] = (Frame){inst->y, -1, 0};
                m->stack[top++] = (Frame){inst->x, -1, 0};
                break;
            case I_SAVE:
                m->stack[top++] = (Frame){0, inst->x, m->work[inst->x]};
                m->work[inst->x] = pos;
                m->stack[top++] = (Frame){pc + 1, -1, 0};
                break;
            case I_ASSERT:
                if (assertionHolds(inst->x, m->text, m->n, pos, m->prog->flags)) {
                    m->stack[top++] = (Frame){pc + 1, -1, 0};
                }
                break;
            case I_PROGRESS:
                m->stack[top++] = (Frame){m->work[inst->x] == pos ? inst->y : pc + 1, -1, 0};
                break;
            default: {
                int i = list->count++;
                list->pcs[i] = pc;
                memcpy(&list->caps[(size_t)i * (size_t)m->nsave], m->work,
                       sizeof(int) * (size_t)m->nsave);
                break;
            }
        }
    }
}

static int consumes(const PolaProgram* prog, const Inst* inst, uint32_t c) {
    int fold = prog->flags & POLA_ABAIKAN_KAPITAL;
    switch (inst->op) {
        case I_CHAR:
            return fold ? foldAscii(c) == foldAscii(inst->c) : c == inst->c;
        case I_ANY:
            return c != '\n';
        case I_ANYNL:
            return 1;
        case I_CLASS:
            return kelasContains(&prog->classes[inst->x], c, fold);
        default:
            return 0;
    }
}

int polaExec(const PolaProgram* prog, const uint32_t* text, int n,
             int start, PolaMode mode, int forbidEmptyAt, int* caps) {
    int ncaps = 2 * (prog->groups + 1);            // what the caller sees
    int nsave = ncaps + prog->loopSlots;           // plus hidden loop slots
    int ninst = prog->count;
    Machine m;
    m.prog = prog;
    m.text = text;
    m.n = n;
    m.nsave = nsave;
    m.generation = 0;
    m.visited = calloc((size_t)ninst, sizeof(int));
    // Each pc is visited at most once per closure, and each visit pushes at
    // most two frames, so 2 * ninst + 1 bounds the stack.
    m.stack = malloc(sizeof(Frame) * (size_t)(2 * ninst + 1));
    m.work = malloc(sizeof(int) * (size_t)nsave);
    ThreadList a, b;
    a.pcs = malloc(sizeof(int) * (size_t)ninst);
    b.pcs = malloc(sizeof(int) * (size_t)ninst);
    a.caps = malloc(sizeof(int) * (size_t)ninst * (size_t)nsave);
    b.caps = malloc(sizeof(int) * (size_t)ninst * (size_t)nsave);
    int* empty = malloc(sizeof(int) * (size_t)nsave);
    int result = -1;
    if (m.visited == NULL || m.stack == NULL || m.work == NULL || a.pcs == NULL ||
        b.pcs == NULL || a.caps == NULL || b.caps == NULL || empty == NULL) {
        goto done;
    }
    for (int i = 0; i < nsave; i++) empty[i] = -1;

    ThreadList* clist = &a;
    ThreadList* nlist = &b;
    clist->count = 0;
    int matched = 0;
    m.generation = 1;

    for (int pos = start; pos <= n; pos++) {
        // A new attempt starting here has the LOWEST priority, so it goes in
        // after the threads that started earlier.
        if (!matched && (mode == POLA_CARI || pos == start)) {
            addThread(&m, clist, 0, empty, pos);
        }
        if (clist->count == 0) {
            if (matched || mode != POLA_CARI) break;
            // Nothing to dedupe against: the next position's attempt needs
            // fresh visited marks, not the ones this failed attempt left.
            m.generation++;
            continue;
        }
        m.generation++;
        nlist->count = 0;
        for (int i = 0; i < clist->count; i++) {
            int pc = clist->pcs[i];
            int* tcaps = &clist->caps[(size_t)i * (size_t)nsave];
            const Inst* inst = &prog->code[pc];
            if (inst->op == I_MATCH) {
                if (mode == POLA_PENUH && pos != n) continue;
                if (forbidEmptyAt >= 0 && tcaps[0] == forbidEmptyAt && pos == forbidEmptyAt) {
                    continue;
                }
                memcpy(caps, tcaps, sizeof(int) * (size_t)ncaps);
                matched = 1;
                break;   // lower-priority threads lose to this one
            }
            if (pos < n && consumes(prog, inst, text[pos])) {
                addThread(&m, nlist, pc + 1, tcaps, pos + 1);
            }
        }
        ThreadList* swap = clist;
        clist = nlist;
        nlist = swap;
    }
    result = matched;

done:
    free(m.visited);
    free(m.stack);
    free(m.work);
    free(a.pcs);
    free(b.pcs);
    free(a.caps);
    free(b.caps);
    free(empty);
    return result;
}

// Decode one UTF-8 sequence at *i; invalid bytes decode as themselves so the
// engine never rejects a string the rest of the language accepts.
static int utf8Decode(const unsigned char* s, int length, int* i, uint32_t* out) {
    unsigned char c = s[*i];
    int extra = 0;
    uint32_t value = c;
    if (c >= 0xF0 && c < 0xF8) { extra = 3; value = c & 0x07; }
    else if (c >= 0xE0) { extra = 2; value = c & 0x0F; }
    else if (c >= 0xC0) { extra = 1; value = c & 0x1F; }
    if (c >= 0xF8 || (c >= 0x80 && c < 0xC0) || *i + extra > length - 1) {
        extra = 0;
        value = c;
    }
    for (int k = 1; k <= extra; k++) {
        unsigned char cc = s[*i + k];
        if ((cc & 0xC0) != 0x80) { extra = 0; value = c; break; }
        value = (value << 6) | (cc & 0x3F);
    }
    *i += 1 + extra;
    *out = value;
    return 1;
}

#ifndef POLA_TANPA_VM
// ---------------------------------------------------------------------------
// Natives
// ---------------------------------------------------------------------------

#include "memory.h"
#include "vm.h"

// Compiled patterns are cached by content (not by ObjString pointer: a
// collected string's address can be reused by a different pattern).
#define POLA_CACHE 16

typedef struct {
    char* pattern;
    int length;
    int flags;
    PolaProgram* program;
} CacheEntry;

static CacheEntry cache[POLA_CACHE];
static int cacheNext = 0;

// The program for (pattern, flags), compiling on a miss. NULL with `error`
// filled when the pattern is invalid.
static PolaProgram* cachedProgram(ObjString* pattern, int flags, char* error, size_t errorSize) {
    for (int i = 0; i < POLA_CACHE; i++) {
        CacheEntry* e = &cache[i];
        if (e->program != NULL && e->flags == flags && e->length == pattern->length &&
            memcmp(e->pattern, pattern->chars, (size_t)pattern->length) == 0) {
            return e->program;
        }
    }
    PolaProgram* program = polaCompile(pattern->chars, pattern->length, flags, error, errorSize);
    if (program == NULL) return NULL;
    char* copy = malloc((size_t)pattern->length + 1);
    if (copy == NULL) {
        // Uncacheable but usable: the caller owns nothing, so leak-free means
        // freeing it right after use -- simpler to just refuse.
        polaFree(program);
        snprintf(error, errorSize, "Memori habis saat mengompilasi pola");
        return NULL;
    }
    memcpy(copy, pattern->chars, (size_t)pattern->length);
    CacheEntry* slot = &cache[cacheNext];
    cacheNext = (cacheNext + 1) % POLA_CACHE;
    polaFree(slot->program);
    free(slot->pattern);
    slot->pattern = copy;
    slot->length = pattern->length;
    slot->flags = flags;
    slot->program = program;
    return program;
}

// Code points of a teks, freshly allocated (caller frees). NULL on OOM.
static uint32_t* decodeText(ObjString* text) {
    uint32_t* out = malloc(sizeof(uint32_t) * (size_t)(text->charCount > 0 ? text->charCount : 1));
    if (out == NULL) return NULL;
    int n = 0;
    for (int i = 0; i < text->length && n < text->charCount;) {
        uint32_t c;
        utf8Decode((const unsigned char*)text->chars, text->length, &i, &c);
        out[n++] = c;
    }
    return out;
}

static int intArg(Value v, int* out) {
    if (!IS_INT(v)) return 0;
    int64_t x = AS_INT(v);
    if (x < -1 || x > 0x7FFFFFFF) return 0;
    *out = (int)x;
    return 1;
}

// _pola_kompilasi(pola, bendera) -> (jumlah_grup, {nama: indeks}), or a teks
// with the error message. Errors are RETURNED, not raised: pustaka/pola.idk
// raises its own GalatPola class with them.
Value nativeFnPolaKompilasi(int argCount, Value* args) {
    int flags;
    if (argCount != 2 || !IS_STRING(args[0]) || !intArg(args[1], &flags)) {
        nativeRaiseKelas(vm.galatTipe, "_pola_kompilasi(pola, bendera) memerlukan teks dan bulat");
        return KOSONG_VAL;
    }
    char error[256];
    PolaProgram* program = cachedProgram(AS_STRING(args[0]), flags, error, sizeof(error));
    if (program == NULL) return OBJ_VAL(copyString(error, (int)strlen(error)));

    ObjDict* names = newDict();
    push(OBJ_VAL(names));                                  // GC: root while filling
    for (int g = 1; g <= polaGroupCount(program); g++) {
        const char* name = polaGroupName(program, g);
        if (name == NULL) continue;
        Value key = OBJ_VAL(copyString(name, (int)strlen(name)));
        push(key);
        dictSet(names, key, INT_VAL(g));
        pop();
    }
    ObjTuple* result = newTuple();
    push(OBJ_VAL(result));
    writeValueArray(&result->items, INT_VAL(polaGroupCount(program)));
    writeValueArray(&result->items, OBJ_VAL(names));
    pop();
    pop();
    return OBJ_VAL(result);
}

// Shared argument check for the two matching natives. On success *program
// and *text are set; on failure an error is pending.
static int matchArgs(int argCount, Value* args, int expected, PolaProgram** program,
                     ObjString** text, int* flags, int* start) {
    if (argCount != expected || !IS_STRING(args[0]) || !intArg(args[1], flags) ||
        !IS_STRING(args[2]) || !intArg(args[3], start) || *start < 0) {
        nativeRaiseKelas(vm.galatTipe, "argumen internal pola tidak sah");
        return 0;
    }
    char error[256];
    *program = cachedProgram(AS_STRING(args[0]), *flags, error, sizeof(error));
    if (*program == NULL) {
        nativeRaiseKelas(vm.galatNilai, "%s", error);
        return 0;
    }
    *text = AS_STRING(args[2]);
    return 1;
}

// Flat list [awal0, akhir0, awal1, akhir1, ...]; -1 for a group that did not
// take part. Built rooted on the stack.
static Value spanList(const int* caps, int nsave) {
    ObjList* list = newList();
    push(OBJ_VAL(list));
    for (int i = 0; i < nsave; i++) writeValueArray(&list->items, INT_VAL(caps[i]));
    pop();
    return OBJ_VAL(list);
}

// _pola_cari(pola, bendera, teks, mulai, mode, larang_kosong) -> span list or kosong.
Value nativeFnPolaCari(int argCount, Value* args) {
    PolaProgram* program;
    ObjString* text;
    int flags, start, mode, forbid;
    if (!matchArgs(argCount, args, 6, &program, &text, &flags, &start)) return KOSONG_VAL;
    if (!intArg(args[4], &mode) || mode < POLA_CARI || mode > POLA_PENUH || !intArg(args[5], &forbid)) {
        nativeRaiseKelas(vm.galatTipe, "argumen internal pola tidak sah");
        return KOSONG_VAL;
    }
    if (start > text->charCount) return KOSONG_VAL;
    uint32_t* cps = decodeText(text);
    int nsave = 2 * (polaGroupCount(program) + 1);
    int* caps = malloc(sizeof(int) * (size_t)nsave);
    if (cps == NULL || caps == NULL) {
        free(cps);
        free(caps);
        nativeRaiseKelas(vm.galatNilai, "Memori habis saat mencocokkan pola");
        return KOSONG_VAL;
    }
    int found = polaExec(program, cps, text->charCount, start, (PolaMode)mode, forbid, caps);
    Value result = KOSONG_VAL;
    if (found < 0) {
        nativeRaiseKelas(vm.galatNilai, "Memori habis saat mencocokkan pola");
    } else if (found > 0) {
        result = spanList(caps, nsave);
    }
    free(cps);
    free(caps);
    return result;
}

// _pola_semua(pola, bendera, teks, mulai, batas) -> list of span lists for
// every non-overlapping match, CPython's finditer rules: after an empty match
// the next one may not be empty at the same place. `batas` 0 = no limit.
// One decode for the whole scan, so cari_semua/ganti/pisah stay linear.
Value nativeFnPolaSemua(int argCount, Value* args) {
    PolaProgram* program;
    ObjString* text;
    int flags, start, limit;
    if (!matchArgs(argCount, args, 5, &program, &text, &flags, &start)) return KOSONG_VAL;
    if (!intArg(args[4], &limit) || limit < 0) {
        nativeRaiseKelas(vm.galatTipe, "argumen internal pola tidak sah");
        return KOSONG_VAL;
    }
    uint32_t* cps = decodeText(text);
    int nsave = 2 * (polaGroupCount(program) + 1);
    int* caps = malloc(sizeof(int) * (size_t)nsave);
    if (cps == NULL || caps == NULL) {
        free(cps);
        free(caps);
        nativeRaiseKelas(vm.galatNilai, "Memori habis saat mencocokkan pola");
        return KOSONG_VAL;
    }
    ObjList* all = newList();
    push(OBJ_VAL(all));                                   // GC: root while filling
    int n = text->charCount;
    int pos = start;
    int forbid = -1;
    while (pos <= n && (limit == 0 || all->items.count < limit)) {
        int found = polaExec(program, cps, n, pos, POLA_CARI, forbid, caps);
        if (found < 0) {
            nativeRaiseKelas(vm.galatNilai, "Memori habis saat mencocokkan pola");
            break;
        }
        if (found == 0) break;
        Value spans = spanList(caps, nsave);
        push(spans);
        writeValueArray(&all->items, spans);
        pop();
        // Next search starts where this match ended. Only after an EMPTY
        // match is another empty one at the same place forbidden (CPython
        // 3.7+): re.sub("x*", "-", "abxd") == "-a-b--d-".
        pos = caps[1];
        forbid = caps[1] == caps[0] ? caps[1] : -1;
    }
    pop();
    free(cps);
    free(caps);
    return OBJ_VAL(all);
}
#endif
