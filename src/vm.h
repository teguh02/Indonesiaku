#ifndef indonesiaku_vm_h
#define indonesiaku_vm_h

#include "object.h"
#include "table.h"
#include "value.h"

// Maximum call depth, matching Python's default recursion limit. The value
// stack is sized for the worst case (256 slots per frame) and lives in the
// global `vm`, so it costs static memory, not C stack.
#define FRAMES_MAX 1000
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)
#define TRY_MAX 64

// Stack protection checks
#define STACK_UNDERFLOW_CHECK(count) \
    do { \
        if (vm.stackTop - vm.stack < (count)) { \
            fatalError("Stack underflow: attempted to pop %d values but only %td available", \
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
    ObjClosure* closure;
    uint8_t* ip;
    Value* slots;
} CallFrame;

// Records an active 'coba' (try) block so a raised error can unwind to its
// 'kecuali' (catch) handler.
typedef struct {
    uint8_t* handlerIp;   // where the catch handler begins
    int frameCount;       // frame depth when the try block was entered
    Value* stackTop;      // stack top when the try block was entered
} TryHandler;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    Value stack[STACK_MAX];
    Value* stackTop;
    int frameCount;
    TryHandler tryHandlers[TRY_MAX];
    int tryCount;
    int nativeCallDepth;   // >0 while a native is running an Indonesiaku callback
    // Frame depth at which run() must stop and return instead of continuing.
    // 0 at top level; raised by callFromNative() so a callback invoked from C
    // returns to its C caller instead of running the rest of the program.
    int exitFrame;
    Table globals;
    Table strings;
    Table loadedModules;
    ObjString* initString;
    ObjClass* galat;
    ObjClass* galatAritmetika;
    ObjClass* galatBagiNol;
    ObjClass* galatTipe;
    ObjClass* galatNilai;
    ObjClass* galatIndeks;
    ObjClass* galatNama;
    ObjUpvalue* openUpvalues;
    Obj* objects;
    bool hadError;

    // Set by a native function that wants to raise a *catchable* error
    // (via nativeRaise). callValue converts it into a throwValue unwind.
    bool nativeErrorPending;
    Value nativeErrorValue;

    // Set by runtimeError() when a 'coba' handler is active: the error is
    // caught instead of aborting. interpret() unwinds to the handler and
    // resumes run() with the message as the caught value.
    bool runtimeErrorPending;
    ObjClass* runtimeErrorClass;
    char runtimeErrorMessage[512];

    // Garbage collector bookkeeping
    size_t bytesAllocated;
    size_t nextGC;
    int grayCount;
    int grayCapacity;
    Obj** grayStack;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
void defineArgs(int argc, const char* argv[], int start);
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();
void runtimeError(const char* format, ...);
void fatalError(const char* format, ...);
void nativeRaise(const char* format, ...);
void nativeRaiseTipe(const char* format, ...);
// Call an Indonesiaku callable from inside a native function. `callee` and
// `argCount` arguments must already be on the VM stack (callee first); on
// success they are replaced by the single result value, which is also
// returned. Returns false after flagging the error (nativeRaise-style) when
// the call fails or raises.
bool callFromNative(int argCount, Value* result);
bool valueIsFalsey(Value value);

#endif
