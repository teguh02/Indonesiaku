#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "object.h"

#define DEBUG_PRINT_CODE

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
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
} Local;

typedef enum {
    TYPE_FUNCTION,
    TYPE_SCRIPT
} FunctionType;

typedef struct Compiler {
    struct Compiler* enclosing;
    ObjFunction* function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int localCount;
    int scopeDepth;
} Compiler;

Parser parser;
Compiler* current = NULL;

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
    emitByte(OP_KOSONG);
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
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
    local->name.start = "";
    local->name.length = 0;
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
        emitByte(OP_POP);
        current->localCount--;
    }
}

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

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

static void addLocal(Token name) {
    if (current->localCount == UINT8_COUNT) {
        error("Terlalu banyak variabel lokal dalam fungsi.");
        return;
    }

    Local* local = &current->locals[current->localCount++];
    local->name = name;
    local->depth = -1;
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
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t getOp, setOp;
    int arg = resolveLocal(current, &name);
    if (arg != -1) {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    } else {
        arg = identifierConstant(&name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        // Jika assignment terjadi pada scope global (top-level),
        // perlakukan sebagai deklarasi/immediate definition.
        // Ini memungkinkan penulisan: x = 10 tanpa kata kunci 'variabel'.
        if (setOp == OP_SET_GLOBAL && current->scopeDepth == 0) {
            emitBytes(OP_DEFINE_GLOBAL, (uint8_t)arg);
        } else {
            emitBytes(setOp, (uint8_t)arg);
        }
    } else {
        emitBytes(getOp, (uint8_t)arg);
    }
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
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
  [TOKEN_LEFT_BRACKET]  = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACKET] = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
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
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DIRI]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_BENAR]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_VARIABEL]      = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SELAGI]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TIDAK]         = {unary,    NULL,   PREC_NONE},
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
    emitBytes(OP_CONSTANT, makeConstant(OBJ_VAL(function)));
}

static void funDeclaration() {
    uint8_t global = parseVariable("Harapkan nama fungsi.");
    markInitialized();
    function(TYPE_FUNCTION);
    defineVariable(global);
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
    beginScope();
    
    // 'untuk' identifier 'dalam' expression '{'
    consume(TOKEN_IDENTIFIER, "Harapkan nama variabel setelah 'untuk'.");
    Token varName = parser.previous;
    
    consume(TOKEN_DALAM, "Harapkan 'dalam' setelah nama variabel.");
    expression();
    consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah ekspresi dalam 'untuk'.");
    
    int loopStart = currentChunk()->count;
    block();
    emitLoop(loopStart);
    
    endScope();
}

static void ifStatement() {
    expression();
    consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah kondisi 'jika'.");
    block();

    int elseJump = emitJump(OP_JUMP);
    int thenJump = currentChunk()->count;
    emitByte(OP_POP);

    if (match(TOKEN_JIKA_LAIN)) {
        ifStatement();
    } else if (match(TOKEN_SELAIN)) {
        consume(TOKEN_LEFT_BRACE, "Harapkan '{' setelah 'selain'.");
        block();
    }

    patchJump(elseJump);
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
    block();
    
    emitLoop(loopStart);

    patchJump(exitJump);
    emitByte(OP_POP);
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
