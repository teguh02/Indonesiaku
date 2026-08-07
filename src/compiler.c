#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "object.h"
#include "memory.h"

#if 0
// Disable debug printing of compiled bytecode by default in release builds.
// Define DEBUG_PRINT_CODE for local debugging when needed.
// #define DEBUG_PRINT_CODE

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif
#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // atau
    PREC_AND,         // dan
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * / %
    PREC_POWER,       // **
    PREC_UNARY,       // tidak -
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
    bool isCaptured;
} Local;

typedef struct {
    uint8_t index;
    bool isLocal;
} Upvalue;

typedef enum {
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
} FunctionType;

typedef struct Compiler {
    struct Compiler* enclosing;
    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    Upvalue upvalues[UINT8_COUNT];
    int scopeDepth;
} Compiler;

Parser parser;
Compiler* current = NULL;

typedef struct ClassCompiler {
    struct ClassCompiler* enclosing;
    bool hasSuperclass;
} ClassCompiler;

ClassCompiler* currentClass = NULL;

// Tracks the innermost enclosing loop so 'hentikan' (break) and 'lanjut'
// (continue) know where to jump and how many locals to discard.
typedef struct LoopCtx {
    struct LoopCtx* enclosing;
    int continueTarget;   // bytecode offset to jump to for 'lanjut'
    int scopeDepth;       // scope depth of the loop body
    int breakJumps[UINT8_COUNT];
    int breakCount;
} LoopCtx;

LoopCtx* currentLoop = NULL;

static Chunk* currentChunk() {
    return &current->function->chunk;
}

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;
    parser.panicMode = true;
    fprintf(stderr, "[baris %d] Kesalahan", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " di akhir");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        fprintf(stderr, " di '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitLoop(int loopStart) {
    emitByte(OP_LOOP);

    // Calculate distance to jump back to loopStart
    // After emitting OP_LOOP, currentCount has increased by 1
    // We'll emit 2 more bytes for the offset
    // At runtime, after reading the full OP_LOOP instruction (3 bytes), IP will be at count
    // We want IP -= offset to make IP = loopStart
    // So offset = currentCount + 2 - loopStart
    //            (this is the position of IP after reading, minus where we want to go)
    int offset = currentChunk()->count - loopStart + 2;
    if (offset > UINT16_MAX) error("Loop body terlalu besar.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

static int emitJump(uint8_t instruction) {
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->count - 2;
}

static void emitReturn() {
    if (current->type == TYPE_INITIALIZER) {
        emitBytes(OP_GET_LOCAL, 0);  // return 'diri'
    } else {
        emitByte(OP_KOSONG);
    }
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    // Protect value from a GC that may be triggered while growing the
    // constants array (writeValueArray allocates). It is only referenced
    // by this local until addConstant stores it.
    push(value);
    int constant = addConstant(currentChunk(), value);
    pop();
    if (constant > UINT8_MAX) {
        error("Terlalu banyak konstanta dalam satu chunk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void patchJump(int offset) {
    // Defense-in-depth: never write outside the emitted code. A malformed
    // program combined with a compiler bug could otherwise corrupt the heap
    // (found via fuzzing). If the offset is invalid, report and bail.
    if (offset < 0 || offset + 1 >= currentChunk()->count) {
        error("Kesalahan internal: offset lompatan tidak valid.");
        return;
    }

    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = currentChunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Terlalu banyak kode untuk dilompati.");
    }

    currentChunk()->code[offset] = (jump >> 8) & 0xff;
    currentChunk()->code[offset + 1] = jump & 0xff;
}

static void initCompiler(Compiler* compiler, FunctionType type) {
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction();
    current = compiler;
    if (type != TYPE_SCRIPT) {
        current->function->name = copyString(parser.previous.start,
                                            parser.previous.length);
    }

    Local* local = &current->locals[current->localCount++];
    local->depth = 0;
    local->isCaptured = false;
    if (type != TYPE_FUNCTION && type != TYPE_SCRIPT) {
        // Methods reserve slot 0 for 'diri' (the receiver / this).
        local->name.start = "diri";
        local->name.length = 4;
    } else {
        local->name.start = "";
        local->name.length = 0;
    }
}

static ObjFunction* endCompiler() {
    emitReturn();
    ObjFunction* function = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), function->name != NULL
            ? function->name->chars : "<skrip>");
    }
#endif

    current = current->enclosing;
    return function;
}

static void beginScope() {
    current->scopeDepth++;
}

static void endScope() {
    current->scopeDepth--;

    while (current->localCount > 0 &&
           current->locals[current->localCount - 1].depth >
               current->scopeDepth) {
        if (current->locals[current->localCount - 1].isCaptured) {
            emitByte(OP_CLOSE_UPVALUE);
        } else {
            emitByte(OP_POP);
        }
        current->localCount--;
    }
}

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);
static void beginLoop(LoopCtx* loop, int continueTarget);
static void endLoop();

static uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static bool identifiersEqual(Token* a, Token* b) {
    if (a->length != b->length) return false;
    return memcmp(a->start, b->start, a->length) == 0;
}

static int resolveLocal(Compiler* compiler, Token* name) {
    for (int i = compiler->localCount - 1; i >= 0; i--) {
        Local* local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name)) {
            if (local->depth == -1) {
                error("Tidak dapat membaca variabel lokal dalam inisialisasinya sendiri.");
            }
            return i;
        }
    }

    return -1;
}

static int addUpvalue(Compiler* compiler, uint8_t index, bool isLocal) {
    int upvalueCount = compiler->function->upvalueCount;

    for (int i = 0; i < upvalueCount; i++) {
        Upvalue* upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->isLocal == isLocal) {
            return i;
        }
    }

    if (upvalueCount == UINT8_COUNT) {
        error("Terlalu banyak variabel closure dalam fungsi.");
        return 0;
    }

    compiler->upvalues[upvalueCount].isLocal = isLocal;
    compiler->upvalues[upvalueCount].index = index;
    return compiler->function->upvalueCount++;
}

static int resolveUpvalue(Compiler* compiler, Token* name) {
    if (compiler->enclosing == NULL) return -1;

    int local = resolveLocal(compiler->enclosing, name);
    if (local != -1) {
        compiler->enclosing->locals[local].isCaptured = true;
        return addUpvalue(compiler, (uint8_t)local, true);
    }

    int upvalue = resolveUpvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return addUpvalue(compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Terlalu banyak variabel lokal dalam fungsi.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
    local->isCaptured = false;
}

// Add a synthetic, already-initialized local whose name cannot collide with
// a user identifier. Returns its stack slot. Used to desugar 'untuk...dalam'.
static int addSyntheticLocal(const char* text) {
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    token.type = TOKEN_IDENTIFIER;
    token.line = parser.previous.line;
    addLocal(token);
    current->locals[current->localCount - 1].depth = current->scopeDepth;
    return current->localCount - 1;
}

static void declareVariable() {
    if (current->scopeDepth == 0) return;

    Token* name = &parser.previous;
    for (int i = current->localCount - 1; i >= 0; i--) {
        Local* local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scopeDepth) {
            break;
        }

        if (identifiersEqual(name, &local->name)) {
            error("Sudah ada variabel dengan nama ini dalam scope ini.");
        }
    }

    addLocal(*name);
}

static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);

    declareVariable();
    if (current->scopeDepth > 0) return 0;

    return identifierConstant(&parser.previous);
}

static void markInitialized() {
    if (current->scopeDepth == 0) return;
    current->locals[current->localCount - 1].depth = current->scopeDepth;
}

static void defineVariable(uint8_t global) {
    if (current->scopeDepth > 0) {
        markInitialized();
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argumentList() {
    uint8_t argCount = 0;
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            if (argCount == 255) {
                error("Tidak boleh lebih dari 255 argumen.");
            }
            argCount++;
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Harapkan ')' setelah argumen.");
    return argCount;
}

static void and_(bool canAssign) {
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parsePrecedence(PREC_AND);

    patchJump(endJump);
}

static void binary(bool canAssign) {
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:       emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:          emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_PLUS:          emitByte(OP_ADD); break;
        case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
        case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
        case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
        case TOKEN_PERCENT:       emitByte(OP_MODULO); break;
        case TOKEN_POWER:         emitByte(OP_POWER); break;
        default: return; // Unreachable.
    }
}

static void call(bool canAssign) {
    uint8_t argCount = argumentList();
    emitBytes(OP_CALL, argCount);
}

static void dot(bool canAssign) {
    consume(TOKEN_IDENTIFIER, "Harapkan nama properti setelah '.'.");
    uint8_t name = identifierConstant(&parser.previous);

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_PROPERTY, name);
    } else if (match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList();
        emitBytes(OP_INVOKE, name);
        emitByte(argCount);
    } else {
        emitBytes(OP_GET_PROPERTY, name);
    }
}

static void listLiteral(bool canAssign) {
    // '[' already consumed. Parse comma-separated elements.
    int count = 0;
    if (!check(TOKEN_RIGHT_BRACKET)) {
        do {
            // Allow trailing newlines inside list literals.
            while (match(TOKEN_NEWLINE)) {}
            if (check(TOKEN_RIGHT_BRACKET)) break;
            expression();
            if (count == 255) {
                error("Tidak boleh lebih dari 255 elemen dalam literal list.");
            }
            count++;
            while (match(TOKEN_NEWLINE)) {}
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_BRACKET, "Harapkan ']' setelah elemen list.");
    emitBytes(OP_BUILD_LIST, (uint8_t)count);
}

static void subscript(bool canAssign) {
    // The list/target is already on the stack. '[' already consumed.
    expression();
    consume(TOKEN_RIGHT_BRACKET, "Harapkan ']' setelah indeks.");

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitByte(OP_INDEX_SET);
    } else {
        emitByte(OP_INDEX_GET);
    }
}

static void literal(bool canAssign) {
    switch (parser.previous.type) {
        case TOKEN_SALAH: emitByte(OP_SALAH); break;
        case TOKEN_KOSONG: emitByte(OP_KOSONG); break;
        case TOKEN_BENAR: emitByte(OP_BENAR); break;
        default: return; // Unreachable.
    }
}

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Harapkan ')' setelah ekspresi.");
}

static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void or_(bool canAssign) {
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parsePrecedence(PREC_OR);
    patchJump(endJump);
}

static void string(bool canAssign) {
    // The token includes the surrounding quotes. Process escape sequences
    // (\n \t \r \\ \" \' \0) into a fresh buffer, then intern it.
    const char* src = parser.previous.start + 1;
    int rawLen = parser.previous.length - 2;

    char* chars = ALLOCATE(char, rawLen + 1);
    int len = 0;
    for (int i = 0; i < rawLen; i++) {
        char c = src[i];
        if (c == '\\' && i + 1 < rawLen) {
            char next = src[++i];
            switch (next) {
                case 'n':  chars[len++] = '\n'; break;
                case 't':  chars[len++] = '\t'; break;
                case 'r':  chars[len++] = '\r'; break;
                case '\\': chars[len++] = '\\'; break;
                case '"':  chars[len++] = '"';  break;
                case '\'': chars[len++] = '\''; break;
                case '0':  chars[len++] = '\0'; break;
                default:
                    // Unknown escape: keep the backslash and the character.
                    chars[len++] = '\\';
                    chars[len++] = next;
                    break;
            }
        } else {
            chars[len++] = c;
        }
    }
    chars[len] = '\0';

    ObjString* str = takeString(chars, len);
    emitConstant(OBJ_VAL(str));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else if ((arg = resolveUpvalue(current, &name)) != -1) {
        getOp = OP_GET_UPVALUE;
        setOp = OP_SET_UPVALUE;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        parsePrecedence(PREC_ASSIGNMENT);
        // Jika assignment terjadi pada scope global (top-level),
        // perlakukan sebagai deklarasi/immediate definition.
        // Ini memungkinkan penulisan: x = 10 tanpa kata kunci 'variabel'.
        if (setOp == OP_SET_GLOBAL && current->scopeDepth == 0) {
            // OP_DEFINE_GLOBAL consumes the value; re-load it so that a
            // top-level assignment still evaluates to the assigned value
            // (assignment is an expression, e.g. cetak(x = 5)).
            emitBytes(OP_DEFINE_GLOBAL, (uint8_t)arg);
            emitBytes(OP_GET_GLOBAL, (uint8_t)arg);
        } else {
            emitBytes(setOp, (uint8_t)arg);
        }
    } else if (canAssign && match(TOKEN_PLUS_EQUAL)) {
        // x += 5 becomes x = x + 5
        emitBytes(getOp, (uint8_t)arg);
        parsePrecedence(PREC_TERM);
        emitByte(OP_ADD);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_MINUS_EQUAL)) {
        // x -= 5 becomes x = x - 5
        emitBytes(getOp, (uint8_t)arg);
        parsePrecedence(PREC_TERM);
        emitByte(OP_SUBTRACT);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_STAR_EQUAL)) {
        // x *= 5 becomes x = x * 5
        emitBytes(getOp, (uint8_t)arg);
        parsePrecedence(PREC_FACTOR);
        emitByte(OP_MULTIPLY);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_SLASH_EQUAL)) {
        // x /= 5 becomes x = x / 5
        emitBytes(getOp, (uint8_t)arg);
        parsePrecedence(PREC_FACTOR);
        emitByte(OP_DIVIDE);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_PERCENT_EQUAL)) {
        // x %= 5 becomes x = x % 5
        emitBytes(getOp, (uint8_t)arg);
        parsePrecedence(PREC_FACTOR);
        emitByte(OP_MODULO);
        emitBytes(setOp, (uint8_t)arg);
    } else if (canAssign && match(TOKEN_POWER_EQUAL)) {
        // x **= 5 becomes x = x ** 5
        emitBytes(getOp, (uint8_t)arg);
        parsePrecedence(PREC_POWER);
        emitByte(OP_POWER);
        emitBytes(setOp, (uint8_t)arg);
    } else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static Token syntheticToken(const char* text) {
    Token token;
    token.start = text;
    token.length = (int)strlen(text);
    return token;
}

static void diri_(bool canAssign) {
    if (currentClass == NULL) {
        error("Tidak dapat menggunakan 'diri' di luar kelas.");
        return;
    }
    variable(false);
}

static void super_(bool canAssign) {
    if (currentClass == NULL) {
        error("Tidak dapat menggunakan 'super' di luar kelas.");
    } else if (!currentClass->hasSuperclass) {
        error("Tidak dapat menggunakan 'super' pada kelas tanpa induk.");
    }

    consume(TOKEN_DOT, "Harapkan '.' setelah 'super'.");
    consume(TOKEN_IDENTIFIER, "Harapkan nama metode induk.");
    uint8_t name = identifierConstant(&parser.previous);

    namedVariable(syntheticToken("diri"), false);
    if (match(TOKEN_LEFT_PAREN)) {
        uint8_t argCount = argumentList();
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_SUPER_INVOKE, name);
        emitByte(argCount);
    } else {
        namedVariable(syntheticToken("super"), false);
        emitBytes(OP_GET_SUPER, name);
    }
}

static void unary(bool canAssign) {
    TokenType operatorType = parser.previous.type;

    // Compile the operand.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction.
    switch (operatorType) {
        case TOKEN_BANG:
        case TOKEN_TIDAK: emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return; // Unreachable.
    }
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, call,   PREC_CALL},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACKET]  = {listLiteral, subscript, PREC_CALL},
  [TOKEN_RIGHT_BRACKET] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     dot,    PREC_CALL},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_COLON]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_PERCENT]       = {NULL,     binary, PREC_FACTOR},
  [TOKEN_POWER]         = {NULL,     binary, PREC_POWER},
  [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_DAN]           = {NULL,     and_,   PREC_AND},
  [TOKEN_KELAS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SELAIN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SALAH]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_UNTUK]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUNGSI]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_JIKA]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_KOSONG]        = {literal,  NULL,   PREC_NONE},
  [TOKEN_ATAU]          = {NULL,     or_,    PREC_OR},
  [TOKEN_CETAK]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_KEMBALIKAN]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {super_,   NULL,   PREC_NONE},
  [TOKEN_DIRI]          = {diri_,    NULL,   PREC_NONE},
  [TOKEN_BENAR]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_VARIABEL]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SELAGI]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TIDAK]         = {unary,    NULL,   PREC_NONE},
  [TOKEN_NEWLINE]       = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH_SLASH]   = {NULL,     binary, PREC_FACTOR},
  [TOKEN_PLUS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STAR_EQUAL]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PERCENT_EQUAL] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_POWER_EQUAL]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};static void parsePrecedence(Precedence precedence) {
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Harapkan ekspresi.");
        return;
    }

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Target assignment tidak valid.");
    }
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void block() {
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }

    consume(TOKEN_RIGHT_BRACE, "Harapkan '}' setelah blok.");
}

static void function(FunctionType type) {
    Compiler compiler;
    initCompiler(&compiler, type);
    beginScope();

    // A function body is a fresh compilation unit with its own chunk. The
    // loop context (used to patch break/continue jumps) must NOT leak in from
    // an enclosing loop, or break/continue would record jump offsets against
    // the outer chunk and later patch them into this smaller one — a heap
    // buffer overflow. Reset it for the duration of the body and restore after.
    LoopCtx* savedLoop = currentLoop;
    currentLoop = NULL;

    consume(TOKEN_LEFT_PAREN, "Harapkan '(' setelah nama fungsi.");
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                errorAtCurrent("Tidak boleh lebih dari 255 parameter.");
            }
            uint8_t constant = parseVariable("Harapkan nama parameter.");
            defineVariable(constant);
        } while (match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "Harapkan ')' setelah parameter.");
    consume(TOKEN_LEFT_BRACE, "Harapkan '{' sebelum badan fungsi.");
    block();

    ObjFunction* function = endCompiler();

    currentLoop = savedLoop;

    emitBytes(OP_CLOSURE, makeConstant(OBJ_VAL(function)));

    for (int i = 0; i < function->upvalueCount; i++) {
        emitByte(compiler.upvalues[i].isLocal ? 1 : 0);
        emitByte(compiler.upvalues[i].index);
    }
}

static void funDeclaration() {
    uint8_t global = parseVariable("Harapkan nama fungsi.");
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
}

static void method() {
    consume(TOKEN_IDENTIFIER, "Harapkan nama metode.");
    uint8_t constant = identifierConstant(&parser.previous);

    FunctionType type = TYPE_METHOD;
    if (parser.previous.length == 4 &&
        memcmp(parser.previous.start, "init", 4) == 0) {
        type = TYPE_INITIALIZER;
    }
    function(type);
    emitBytes(OP_METHOD, constant);
}

static void classDeclaration() {
    consume(TOKEN_IDENTIFIER, "Harapkan nama kelas.");
    Token className = parser.previous;
    uint8_t nameConstant = identifierConstant(&parser.previous);
    declareVariable();

    emitBytes(OP_CLASS, nameConstant);
    defineVariable(nameConstant);

    ClassCompiler classCompiler;
    classCompiler.hasSuperclass = false;
    classCompiler.enclosing = currentClass;
    currentClass = &classCompiler;

    // Optional superclass:  kelas Anak < Induk { ... }
    if (match(TOKEN_LESS)) {
        consume(TOKEN_IDENTIFIER, "Harapkan nama kelas induk.");
        variable(false);

        if (identifiersEqual(&className, &parser.previous)) {
            error("Sebuah kelas tidak dapat mewarisi dirinya sendiri.");
        }

        beginScope();
        addLocal(syntheticToken("super"));
        defineVariable(0);

        namedVariable(className, false);
        emitByte(OP_INHERIT);
        classCompiler.hasSuperclass = true;
    }

    namedVariable(className, false);  // push the class for OP_METHOD
    consume(TOKEN_LEFT_BRACE, "Harapkan '{' sebelum badan kelas.");
    while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        while (match(TOKEN_NEWLINE)) {}
        if (check(TOKEN_RIGHT_BRACE)) break;
        method();
        while (match(TOKEN_NEWLINE)) {}
    }
    consume(TOKEN_RIGHT_BRACE, "Harapkan '}' setelah badan kelas.");
    emitByte(OP_POP);  // pop the class

    if (classCompiler.hasSuperclass) {
        endScope();
    }

    currentClass = currentClass->enclosing;
}

static void varDeclaration() {
    uint8_t global = parseVariable("Harapkan nama variabel.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_KOSONG);
    }

    // consume(TOKEN_SEMICOLON, "Harapkan ';' setelah deklarasi variabel.");
    defineVariable(global);
}

static void expressionStatement() {
    expression();
    // consume(TOKEN_SEMICOLON, "Harapkan ';' setelah ekspresi.");
    emitByte(OP_POP);
}

static void forStatement() {
    // Desugar:  untuk x dalam <list> { body }
    // into an index-based loop over the list, using hidden local slots.
    beginScope();

    consume(TOKEN_IDENTIFIER, "Harapkan nama variabel setelah 'untuk'.");
    Token varName = parser.previous;

    consume(TOKEN_DALAM, "Harapkan 'dalam' setelah nama variabel.");

    // Evaluate the iterable and stash it in a hidden slot.
    expression();
    int seqSlot = addSyntheticLocal("  untuk seq");

    // Hidden length slot.
    emitBytes(OP_GET_LOCAL, (uint8_t)seqSlot);
    emitByte(OP_LIST_LEN);
    int lenSlot = addSyntheticLocal("  untuk len");

    // Hidden index slot = -1 (the increment runs before the first
    // condition check, bringing it to 0 for the first iteration).
    emitConstant(NUMBER_VAL(-1));
    int idxSlot = addSyntheticLocal("  untuk idx");

    // User-visible loop variable, initialized to kosong for now.
    emitByte(OP_KOSONG);
    addLocal(varName);
    markInitialized();
    int varSlot = current->localCount - 1;

    consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah ekspresi dalam 'untuk'.");

    // Layout (so 'lanjut' can target the increment):
    //   incrementStart:  idx = idx + 1
    //   condStart:       if !(idx < len) goto exit
    //                    x = seq[idx]; body; goto incrementStart
    // On first entry we jump straight to condStart, skipping the increment.
    int incrementStart = currentChunk()->count;

    // idx = idx + 1
    emitBytes(OP_GET_LOCAL, (uint8_t)idxSlot);
    emitConstant(NUMBER_VAL(1));
    emitByte(OP_ADD);
    emitBytes(OP_SET_LOCAL, (uint8_t)idxSlot);
    emitByte(OP_POP);

    int condStart = currentChunk()->count;

    // Condition: idx < len
    emitBytes(OP_GET_LOCAL, (uint8_t)idxSlot);
    emitBytes(OP_GET_LOCAL, (uint8_t)lenSlot);
    emitByte(OP_LESS);
    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP); // condition

    // x = seq[idx]
    emitBytes(OP_GET_LOCAL, (uint8_t)seqSlot);
    emitBytes(OP_GET_LOCAL, (uint8_t)idxSlot);
    emitByte(OP_INDEX_GET);
    emitBytes(OP_SET_LOCAL, (uint8_t)varSlot);
    emitByte(OP_POP);

    // Loop body. 'lanjut' jumps to incrementStart; 'hentikan' to the end.
    LoopCtx loop;
    beginLoop(&loop, incrementStart);
    beginScope();
    block();
    endScope();

    emitLoop(incrementStart);

    patchJump(exitJump);
    emitByte(OP_POP); // condition

    endLoop();  // break jumps land here, past the condition pop

    endScope();
}

static void beginLoop(LoopCtx* loop, int continueTarget) {
    loop->enclosing = currentLoop;
    loop->continueTarget = continueTarget;
    loop->scopeDepth = current->scopeDepth;
    loop->breakCount = 0;
    currentLoop = loop;
}

static void endLoop() {
    for (int i = 0; i < currentLoop->breakCount; i++) {
        patchJump(currentLoop->breakJumps[i]);
    }
    currentLoop = currentLoop->enclosing;
}

// Emit OP_POP for each local declared deeper than the given scope depth, so
// break/continue leave the operand stack balanced. Does not remove the
// compiler's record of those locals (they remain in scope for the rest of
// the body); endScope() handles the real cleanup.
static void popLocalsToDepth(int depth) {
    for (int i = current->localCount - 1;
         i >= 0 && current->locals[i].depth > depth;
         i--) {
        emitByte(OP_POP);
    }
}

static void breakStatement() {
    if (currentLoop == NULL) {
        error("'hentikan' hanya boleh di dalam perulangan.");
        return;
    }
    popLocalsToDepth(currentLoop->scopeDepth);
    if (currentLoop->breakCount == UINT8_COUNT) {
        error("Terlalu banyak 'hentikan' dalam satu perulangan.");
        return;
    }
    currentLoop->breakJumps[currentLoop->breakCount++] = emitJump(OP_JUMP);
}

static void continueStatement() {
    if (currentLoop == NULL) {
        error("'lanjut' hanya boleh di dalam perulangan.");
        return;
    }
    popLocalsToDepth(currentLoop->scopeDepth);
    emitLoop(currentLoop->continueTarget);
}

static void ifStatement() {
    expression();
    consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah kondisi 'jika'.");
    
    // Emit conditional jump - jump if condition is false
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);  // Pop condition from stack if true
    
    beginScope();
    block();
    endScope();
    
    // Jump over else/jika_lain block if condition was true
    int elseJump = emitJump(OP_JUMP);
    patchJump(thenJump);  // Patch the false condition jump
    emitByte(OP_POP);  // Pop condition from stack if false

    if (match(TOKEN_JIKA_LAIN)) {
        ifStatement();
    } else if (match(TOKEN_SELAIN)) {
        consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah 'selain'.");
        beginScope();
        block();
        endScope();
    }

    patchJump(elseJump);  // Patch the true path jump to skip else
}

static void raiseStatement() {
    // naikkan <ekspresi>  -> evaluate then throw.
    expression();
    emitByte(OP_THROW);
}

static void importStatement() {
    // impor "path.idk"   -> load & run another file in the same namespace.
    consume(TOKEN_STRING, "Harapkan nama file (string) setelah 'impor'.");
    // Reuse the string literal handling to push the path (minus quotes).
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
    emitByte(OP_IMPORT);
    emitByte(OP_POP);  // discard the module's return value (kosong)
}

static void tryStatement() {
    // coba { body } kecuali <var> { handler }
    int tryJump = emitJump(OP_TRY_BEGIN);  // operand = offset to catch handler

    beginScope();
    consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah 'coba'.");
    block();
    endScope();

    emitByte(OP_TRY_END);                  // normal path: unregister handler
    int endJump = emitJump(OP_JUMP);       // skip over the catch handler

    // Catch handler starts here. The VM has pushed the raised value on top.
    patchJump(tryJump);

    consume(TOKEN_KECUALI, "Harapkan 'kecuali' setelah blok 'coba'.");

    beginScope();
    // Bind the raised value (already on the stack) to the catch variable.
    consume(TOKEN_IDENTIFIER, "Harapkan nama variabel setelah 'kecuali'.");
    addLocal(parser.previous);
    markInitialized();

    consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah nama variabel 'kecuali'.");
    block();
    endScope();  // pops the catch variable (the raised value)

    patchJump(endJump);
}

static void printStatement() {
    consume(TOKEN_LEFT_PAREN, "Harapkan '(' setelah 'cetak'.");
    if (!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            emitByte(OP_PRINT);
        } while (match(TOKEN_COMMA));
    }
    
    consume(TOKEN_RIGHT_PAREN, "Harapkan ')' setelah argumen.");
}

static void returnStatement() {
    if (current->type == TYPE_SCRIPT) {
        error("Tidak dapat 'kembalikan' dari kode tingkat atas.");
    }

    if (match(TOKEN_NEWLINE) || check(TOKEN_EOF)) {
        emitReturn();
    } else {
        if (current->type == TYPE_INITIALIZER) {
            error("Tidak dapat mengembalikan nilai dari 'init'.");
        }
        expression();
        emitByte(OP_RETURN);
    }
}

static void whileStatement() {
    int loopStart = currentChunk()->count;
    expression();
    consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah kondisi 'selagi'.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);

    LoopCtx loop;
    beginLoop(&loop, loopStart);
    beginScope();
    block();
    endScope();

    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);

    endLoop();  // break jumps land here, past the condition pop
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_NEWLINE) return;
        switch (parser.current.type) {
            case TOKEN_KELAS:
            case TOKEN_FUNGSI:
            case TOKEN_VARIABEL:
            case TOKEN_UNTUK:
            case TOKEN_JIKA:
            case TOKEN_SELAGI:
            case TOKEN_CETAK:
            case TOKEN_KEMBALIKAN:
                return;

            default:
                ; // Do nothing.
        }

        advance();
    }
}

static void declaration() {
    // Skip stray newlines between declarations
    while (match(TOKEN_NEWLINE)) {}

    if (match(TOKEN_FUNGSI)) {
        funDeclaration();
    } else if (match(TOKEN_KELAS)) {
        classDeclaration();
    } else if (match(TOKEN_VARIABEL)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode) synchronize();
}

static void statement() {
    if (match(TOKEN_CETAK)) {
        printStatement();
    } else if (match(TOKEN_UNTUK)) {
        forStatement();
    } else if (match(TOKEN_JIKA)) {
        ifStatement();
    } else if (match(TOKEN_KEMBALIKAN)) {
        returnStatement();
    } else if (match(TOKEN_SELAGI)) {
        whileStatement();
    } else if (match(TOKEN_HENTIKAN)) {
        breakStatement();
    } else if (match(TOKEN_LANJUT)) {
        continueStatement();
    } else if (match(TOKEN_COBA)) {
        tryStatement();
    } else if (match(TOKEN_NAIKKAN)) {
        raiseStatement();
    } else if (match(TOKEN_IMPOR)) {
        importStatement();
    } else if (match(TOKEN_LEFT_BRACE)) {
        beginScope();
        block();
        endScope();
    } else {
        expressionStatement();
    }
    
    // Consume optional newlines after statements
    while (match(TOKEN_NEWLINE)) {}
}

ObjFunction* compile(const char* source) {
    initScanner(source);
    Compiler compiler;
    initCompiler(&compiler, TYPE_SCRIPT);

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    while (!check(TOKEN_EOF)) {
        declaration();
    }

    ObjFunction* function = endCompiler();
    return parser.hadError ? NULL : function;
}

// Mark every function currently being compiled so the GC does not free
// them mid-compilation (a collection can be triggered by allocations in
// copyString/makeConstant while parsing).
void markCompilerRoots() {
    Compiler* compiler = current;
    while (compiler != NULL) {
        markObject((Obj*)compiler->function);
        compiler = compiler->enclosing;
    }
}
