#ifndef indonesiaku_chunk_h
#define indonesiaku_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_KOSONG,
    OP_BENAR,
    OP_SALAH,
    OP_POP,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_IDENTICAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_POWER,
    OP_FLOOR_DIVIDE,
    OP_IN,
    OP_CHAIN_COMPARE,
    OP_CHAIN_CLEANUP,
    OP_NOT,
    OP_NEGATE,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
    OP_CALL,
    OP_CALL_BERNAMA,
    OP_ISI_BAWAAN,
    OP_CLOSURE,
    OP_GET_UPVALUE,
    OP_SET_UPVALUE,
    OP_CLOSE_UPVALUE,
    OP_BUILD_LIST,
    OP_BUILD_DICT,
    OP_INDEX_GET,
    OP_INDEX_SET,
    OP_SLICE,
    OP_LIST_LEN,
    OP_CLASS,
    OP_INHERIT,
    OP_METHOD,
    OP_GET_PROPERTY,
    OP_SET_PROPERTY,
    OP_INVOKE,
    OP_GET_SUPER,
    OP_SUPER_INVOKE,
    OP_TRY_BEGIN,
    OP_TRY_END,
    OP_THROW,
    OP_PASTIKAN,
    OP_IS_INSTANCE,
    OP_IMPORT,
    OP_RETURN,
} OpCode;

typedef enum {
    CHAIN_EQUAL,
    CHAIN_NOT_EQUAL,
    CHAIN_GREATER,
    CHAIN_LESS,
    CHAIN_GREATER_EQUAL,
    CHAIN_LESS_EQUAL,
    CHAIN_IN,
    CHAIN_NOT_IN,
    CHAIN_IDENTITY,
    CHAIN_NOT_IDENTITY,
} ChainCompare;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif
