#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "format.h"
#include "scanner.h"

// Growable output buffer.
typedef struct {
    char* chars;
    size_t count;
    size_t capacity;
} Out;

static void outInit(Out* o) {
    o->capacity = 1024;
    o->count = 0;
    o->chars = (char*)malloc(o->capacity);
    if (o->chars != NULL) o->chars[0] = '\0';
}

static void outAppend(Out* o, const char* text, size_t length) {
    if (o->chars == NULL) return;
    if (o->count + length + 1 > o->capacity) {
        while (o->count + length + 1 > o->capacity) o->capacity *= 2;
        char* grown = (char*)realloc(o->chars, o->capacity);
        if (grown == NULL) { free(o->chars); o->chars = NULL; return; }
        o->chars = grown;
    }
    memcpy(o->chars + o->count, text, length);
    o->count += length;
    o->chars[o->count] = '\0';
}

static void outChar(Out* o, char c) { outAppend(o, &c, 1); }

static void outIndent(Out* o, int level) {
    for (int i = 0; i < level * 4; i++) outChar(o, ' ');
}

// ---------------------------------------------------------------------------
// Token classification
// ---------------------------------------------------------------------------

static bool isOpener(TokenType t) {
    return t == TOKEN_LEFT_PAREN || t == TOKEN_LEFT_BRACKET || t == TOKEN_LEFT_BRACE;
}

static bool isCloser(TokenType t) {
    return t == TOKEN_RIGHT_PAREN || t == TOKEN_RIGHT_BRACKET || t == TOKEN_RIGHT_BRACE;
}

// Operators that always sit between two operands.
static bool isBinaryOperator(TokenType t) {
    switch (t) {
        case TOKEN_PLUS: case TOKEN_STAR: case TOKEN_SLASH:
        case TOKEN_PERCENT: case TOKEN_POWER: case TOKEN_SLASH_SLASH:
        case TOKEN_EQUAL: case TOKEN_EQUAL_EQUAL: case TOKEN_BANG_EQUAL:
        case TOKEN_GREATER: case TOKEN_GREATER_EQUAL:
        case TOKEN_LESS: case TOKEN_LESS_EQUAL:
        case TOKEN_PLUS_EQUAL: case TOKEN_MINUS_EQUAL:
        case TOKEN_STAR_EQUAL: case TOKEN_SLASH_EQUAL:
        case TOKEN_PERCENT_EQUAL: case TOKEN_POWER_EQUAL:
        case TOKEN_SLASH_SLASH_EQUAL:
        case TOKEN_AMPERSAND: case TOKEN_PIPE: case TOKEN_CARET:
        case TOKEN_LESS_LESS: case TOKEN_GREATER_GREATER:
        case TOKEN_AMPERSAND_EQUAL: case TOKEN_PIPE_EQUAL: case TOKEN_CARET_EQUAL:
        case TOKEN_LESS_LESS_EQUAL: case TOKEN_GREATER_GREATER_EQUAL:
        case TOKEN_DAN: case TOKEN_ATAU: case TOKEN_DALAM:
        case TOKEN_ADALAH:
            return true;
        default:
            return false;
    }
}

// A token that reads as a word and therefore wants a space around it.
static bool isWordToken(TokenType t) {
    if (t == TOKEN_IDENTIFIER || t == TOKEN_NUMBER || t == TOKEN_STRING ||
        t == TOKEN_FSTRING) return true;
    switch (t) {
        case TOKEN_DAN: case TOKEN_KELAS: case TOKEN_SELAIN: case TOKEN_SALAH:
        case TOKEN_UNTUK: case TOKEN_FUNGSI: case TOKEN_JIKA: case TOKEN_KOSONG:
        case TOKEN_ATAU: case TOKEN_KEMBALIKAN: case TOKEN_SUPER:
        case TOKEN_DIRI: case TOKEN_BENAR: case TOKEN_VARIABEL: case TOKEN_SELAGI:
        case TOKEN_JIKA_LAIN: case TOKEN_DALAM: case TOKEN_HENTIKAN:
        case TOKEN_LANJUT: case TOKEN_BUANG: case TOKEN_LEWATI:
        case TOKEN_IMPOR: case TOKEN_DARI: case TOKEN_SEBAGAI: case TOKEN_PASTIKAN:
        case TOKEN_HASILKAN: case TOKEN_COCOKKAN: case TOKEN_KASUS:
        case TOKEN_DENGAN: case TOKEN_COBA: case TOKEN_KECUALI: case TOKEN_AKHIRNYA:
        case TOKEN_GLOBAL: case TOKEN_NONLOKAL:
        case TOKEN_NAIKKAN: case TOKEN_TIDAK: case TOKEN_ADALAH: case TOKEN_BUKAN:
            return true;
        default:
            return false;
    }
}

// A word that ENDS an operand: after one of these, `-` is subtraction.
static bool endsOperand(TokenType t) {
    return t == TOKEN_IDENTIFIER || t == TOKEN_NUMBER || t == TOKEN_STRING ||
           t == TOKEN_FSTRING || t == TOKEN_DIRI || t == TOKEN_BENAR ||
           t == TOKEN_SALAH || t == TOKEN_KOSONG || t == TOKEN_SUPER ||
           isCloser(t);
}

// `-`, `~`, `!` are unary when nothing before them can end an operand.
static bool isUnaryPosition(TokenType prev, bool atLineStart) {
    if (atLineStart) return true;
    return !endsOperand(prev);
}

// A `{` is a DICT LITERAL when it stands where a value is expected, and a
// BLOCK otherwise. Decided from the previous token alone, which is enough:
// `= {`, `( {`, `, {` and `kembalikan {` are values; `) {` and `kelas A {`
// are blocks. Getting this wrong puts a space inside `{"a": 1}`.
static bool braceIsDict(TokenType prev) {
    if (prev == TOKEN_EQUAL || prev == TOKEN_COMMA || prev == TOKEN_COLON ||
        prev == TOKEN_LEFT_PAREN || prev == TOKEN_LEFT_BRACKET ||
        prev == TOKEN_KEMBALIKAN || prev == TOKEN_HASILKAN ||
        prev == TOKEN_DALAM || prev == TOKEN_KASUS) {
        return true;
    }
    return isBinaryOperator(prev);
}

// Should a space be written between `prev` and `cur` on the same line?
// `prevIsDictBrace` / `curIsDictBrace` say whether a brace token is a dict
// literal rather than a block.
static bool wantsSpace(TokenType prev, TokenType cur, bool prevWasUnary,
                       bool prevIsDictBrace, bool curIsDictBrace) {
    // Never a space just inside brackets, or before a separator.
    // A BLOCK brace is the exception: `{ kembalikan x }` reads as a block
    // written on one line, while a dict stays tight: `{"a": 1}`.
    if (prev == TOKEN_LEFT_BRACE) return !prevIsDictBrace;
    if (cur == TOKEN_RIGHT_BRACE) return !curIsDictBrace;
    if (isOpener(prev)) return false;
    if (isCloser(cur)) return false;
    if (cur == TOKEN_COMMA || cur == TOKEN_COLON || cur == TOKEN_SEMICOLON) return false;

    // Attribute access and decorators bind tight.
    if (prev == TOKEN_DOT || cur == TOKEN_DOT) return false;
    if (prev == TOKEN_AT) return false;

    // A unary sign sticks to its operand: -x, ~x -- not - x.
    if (prevWasUnary) return false;

    // Call / index: f(x), d[k], a[1][2] -- no space before the bracket.
    // `fungsi(x)` is an anonymous function and binds just as tightly.
    if (cur == TOKEN_LEFT_PAREN || cur == TOKEN_LEFT_BRACKET) {
        if (endsOperand(prev) || prev == TOKEN_FUNGSI) return false;
    }

    // `{` always gets a space before it -- both `fungsi f() {` and
    // `variabel d = {"a": 1}`. Only the space INSIDE differs, handled above.
    if (cur == TOKEN_LEFT_BRACE) return true;
    if (prev == TOKEN_RIGHT_BRACE && !prevIsDictBrace) return true;

    if (isBinaryOperator(prev) || isBinaryOperator(cur)) return true;
    if (prev == TOKEN_MINUS || cur == TOKEN_MINUS) return true;
    if (prev == TOKEN_COMMA || prev == TOKEN_COLON) return true;
    if (prev == TOKEN_BANG || prev == TOKEN_TILDE) return false;

    if (isWordToken(prev) && isWordToken(cur)) return true;
    if (isWordToken(prev) && isOpener(cur)) return true;
    if (isCloser(prev) && isWordToken(cur)) return true;

    return false;
}

// ---------------------------------------------------------------------------

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;      // line where the token STARTS
    int endLine;   // line where it ENDS (differs for """multi-line""" text)
    bool dictBrace;
} Tok;

// Note: CRLF input comes out as LF, because readFile() normalises newlines
// before we ever see it. That is the desired formatter behaviour (one line
// ending style), not an accident.
//
// ponytail: layout is decided from the token stream alone -- no parser, no
// AST. Line breaks follow the author's original lines; the formatter fixes
// indentation and spacing, it does not re-wrap or join statements. Add
// re-wrapping only if a line-length rule is ever actually wanted.
char* formatSource(const char* source) {
    // Collect the whole stream first: a standalone comment takes its indent
    // from the line that FOLLOWS it, which needs lookahead.
    int capacity = 256, count = 0;
    Tok* toks = (Tok*)malloc(sizeof(Tok) * capacity);
    if (toks == NULL) return NULL;

    scannerKeepComments(true);
    initScanner(source);
    for (;;) {
        Token t = scanToken();
        if (t.type == TOKEN_ERROR) {
            scannerKeepComments(false);
            free(toks);
            return NULL;
        }
        if (t.type == TOKEN_EOF) break;
        // Line breaks are rebuilt from line numbers, so the newline token
        // itself is dropped. Inside brackets the scanner suppresses it
        // entirely, yet token.line still advances -- which is what keeps a
        // multi-line list looking like a multi-line list.
        if (t.type == TOKEN_NEWLINE) continue;
        if (count == capacity) {
            capacity *= 2;
            Tok* grown = (Tok*)realloc(toks, sizeof(Tok) * capacity);
            if (grown == NULL) { scannerKeepComments(false); free(toks); return NULL; }
            toks = grown;
        }
        // The scanner reports the line a token ENDS on. For a multi-line
        // string that is not where it starts, and using it as the start line
        // makes the next token look like it is on an earlier line -- which
        // used to split `variabel puisi = """...` across three lines.
        int newlines = 0;
        for (int k = 0; k < t.length; k++) if (t.start[k] == '\n') newlines++;
        toks[count].type = t.type;
        toks[count].start = t.start;
        toks[count].length = t.length;
        toks[count].line = t.line - newlines;
        toks[count].endLine = t.line;
        toks[count].dictBrace = false;
        count++;
    }
    scannerKeepComments(false);

    // Tag each brace as dict-literal or block, pairing them with a stack so a
    // `}` knows what it closes.
    {
        int stack[256];
        int top = 0;
        TokenType before = TOKEN_EOF;
        for (int i = 0; i < count; i++) {
            if (toks[i].type == TOKEN_KOMENTAR) continue;
            if (toks[i].type == TOKEN_LEFT_BRACE) {
                toks[i].dictBrace = braceIsDict(before);
                if (top < 256) stack[top++] = i;
            } else if (toks[i].type == TOKEN_RIGHT_BRACE) {
                if (top > 0) toks[i].dictBrace = toks[stack[--top]].dictBrace;
            }
            before = toks[i].type;
        }
    }

    Out out;
    outInit(&out);
    if (out.chars == NULL) { free(toks); return NULL; }

    int depth = 0;
    int prevLine = count > 0 ? toks[0].line : 1;
    bool lineStart = true;
    bool prevUnary = false;
    bool prevDictBrace = false;
    TokenType prev = TOKEN_EOF;

    for (int i = 0; i < count; i++) {
        Tok* t = &toks[i];

        // A closer dedents itself so `}` lines up with the line that opened it.
        if (isCloser(t->type) && depth > 0) depth--;

        bool newLine = (i == 0) || (t->line > prevLine);

        // CORRECTNESS, not style: a comment runs to end of line, so any token
        // placed after it on the same line would be swallowed into it.
        if (!newLine && prev == TOKEN_KOMENTAR) newLine = true;

        if (newLine) {
            if (i > 0) {
                outChar(&out, '\n');
                if (t->line - prevLine > 1) outChar(&out, '\n');  // keep one blank line
            }
            // Plan rule: a comment sitting just before a closing `}` keeps the
            // indent of the block BODY, not of the `}`. Since the closer has
            // not been seen yet, `depth` is still the body depth -- so the
            // plain value is already correct and no lookahead is needed.
            outIndent(&out, depth);
            lineStart = true;
        } else if (i > 0) {
            if (t->type == TOKEN_KOMENTAR) {
                outAppend(&out, "  ", 2);       // two spaces before a trailing #
            } else if (wantsSpace(prev, t->type, prevUnary,
                                  prevDictBrace, t->dictBrace)) {
                outChar(&out, ' ');
            }
        }

        outAppend(&out, t->start, (size_t)t->length);
        if (out.chars == NULL) { free(toks); return NULL; }

        prevUnary = (t->type == TOKEN_MINUS || t->type == TOKEN_TILDE ||
                     t->type == TOKEN_BANG) &&
                    isUnaryPosition(prev, lineStart);

        if (isOpener(t->type)) depth++;

        prev = t->type;
        prevDictBrace = t->dictBrace;
        prevLine = t->endLine;      // multi-line strings end lower than they start
        lineStart = false;
    }

    if (count > 0) outChar(&out, '\n');   // exactly one trailing newline

    free(toks);
    return out.chars;
}
