#ifndef indonesiaku_vm_h
#define indonesiaku_vm_h

#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

// Stack protection checks
#define STACK_UNDERFLOW_CHECK(count) \
    do { \
        if (vm.stackTop - vm.stack < (count)) { \
            runtimeError("Stack underflow: attempted to pop %d values but only %td available", \
                        (count), (vm.stackTop - vm.stack)); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
    } while (false)

#define STACK_OVERFLOW_CHECK() \
    do { \
        if (vm.stackTop >= vm.stack + STACK_MAX) { \
            runtimeError("Stack overflow: maximum stack depth %d exceeded", STACK_MAX); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
    } while (false)

#define FRAME_OVERFLOW_CHECK() \
    do { \
        if (vm.frameCount >= FRAMES_MAX) { \
            runtimeError("Frame overflow: maximum call depth %d exceeded", FRAMES_MAX); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
    } while (false)

typedef struct {
    ObjFunction* function;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    Value stack[STACK_MAX];
    Value* stackTop;
    int frameCount;
    Table globals;
    Table strings;
    Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif
