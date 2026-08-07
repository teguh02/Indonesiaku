#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

//#define DEBUG_TRACE_EXECUTION

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"
#include "native.h"

VM vm;

static void closeUpvalues(Value* last);

static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = NULL;
    vm.tryCount = 0;
}

void runtimeError(const char* format, ...) {
    vm.hadError = true;

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[baris %d] dalam ",
                function->chunk.lines[instruction]);
        if (function->name == NULL) {
            fprintf(stderr, "skrip\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}

// Raise a *catchable* error from a native function. Formats a message string
// and stashes it; callValue will convert it into a throwValue() unwind after
// the native returns, so `coba/kecuali` can catch it. Unlike runtimeError,
// this does not tear down the VM.
void nativeRaise(const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    vm.nativeErrorValue = OBJ_VAL(copyString(buffer, (int)strlen(buffer)));
    vm.nativeErrorPending = true;
}

static void defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

void initVM() {
    resetStack();
    vm.objects = NULL;
    vm.hadError = false;
    vm.nativeErrorPending = false;
    vm.nativeErrorValue = KOSONG_VAL;

    srand((unsigned int)time(NULL));

    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;
    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = NULL;

    initTable(&vm.globals);
    initTable(&vm.strings);
    initTable(&vm.loadedModules);

    vm.initString = NULL;
    vm.initString = copyString("init", 4);

    defineNative("jam", clockNative);
    defineNative("min", nativeFnMin);
    defineNative("max", nativeFnMax);
    defineNative("abs", nativeFnAbs);
    defineNative("akar", nativeFnAkar);
    defineNative("pangkat", nativeFnPangkat);
    defineNative("bulat", nativeFnBulat);
    defineNative("lantai", nativeFnLantai);
    defineNative("atap", nativeFnAtap);
    defineNative("acak", nativeFnAcak);

    defineNative("panjang", nativeFnPanjang);
    defineNative("huruf_besar", nativeFnHurofBesar);
    defineNative("huruf_kecil", nativeFnHurofKecil);
    defineNative("ganti", nativeFnGanti);

    defineNative("tambah", nativeFnTambah);
    defineNative("hapus", nativeFnHapus);

    defineNative("kamus", nativeFnKamus);
    defineNative("punya", nativeFnPunya);
    defineNative("hapus_kunci", nativeFnHapusKunci);
    defineNative("kunci", nativeFnKunci);

    defineNative("jenis", nativeFnJenis);
    defineNative("input", nativeFnInput);
    defineNative("henti", nativeFnHenti);

    defineNative("ke_angka", nativeFnKeAngka);
    defineNative("ke_teks", nativeFnKeTeks);
    defineNative("format", nativeFnFormat);
    defineNative("acak_bulat", nativeFnAcakBulat);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeTable(&vm.loadedModules);
    vm.initString = NULL;
    freeObjects();
}

// Expose command-line arguments to the program as a global list `argumen`.
// argv[start..argc) become string elements (start skips the script path).
void defineArgs(int argc, const char* argv[], int start) {
    ObjList* list = newList();
    push(OBJ_VAL(list));  // keep reachable during allocation
    for (int i = start; i < argc; i++) {
        Value s = OBJ_VAL(copyString(argv[i], (int)strlen(argv[i])));
        push(s);
        writeValueArray(&list->items, s);
        pop();
    }
    ObjString* name = copyString("argumen", 7);
    push(OBJ_VAL(name));
    tableSet(&vm.globals, name, OBJ_VAL(list));
    pop();  // name
    pop();  // list
}

void push(Value value) {
    if (vm.stackTop >= vm.stack + STACK_MAX) {
        runtimeError("Stack overflow: maksimum kedalaman stack %d terlampaui", STACK_MAX);
        return;
    }
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    if (vm.stackTop <= vm.stack) {
        runtimeError("Stack underflow: mencoba mengambil nilai dari stack kosong");
        return KOSONG_VAL;
    }
    vm.stackTop--;
    Value result = *vm.stackTop;
    return result;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure* closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError("Harapkan %d argumen tetapi mendapat %d.",
                    closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount >= FRAMES_MAX) {
        runtimeError("Call stack overflow: kedalaman maksimal panggilan fungsi %d terlampaui. Kemungkinan rekursi tak terbatas?", FRAMES_MAX);
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
                vm.stackTop[-argCount - 1] = bound->receiver;
                return call(bound->method, argCount);
            }
            case OBJ_CLASS: {
                ObjClass* klass = AS_CLASS(callee);
                vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
                Value initializer;
                if (tableGet(&klass->methods, vm.initString, &initializer)) {
                    return call(AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Harapkan 0 argumen tetapi mendapat %d.", argCount);
                    return false;
                }
                return true;
            }
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default:
                break; // Non-callable object type.
        }
    }
    runtimeError("Hanya dapat memanggil fungsi dan kelas.");
    return false;
}

static bool bindMethod(ObjClass* klass, ObjString* name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Properti tidak terdefinisi '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod* bound = newBoundMethod(peek(0), AS_CLOSURE(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}

static bool invokeFromClass(ObjClass* klass, ObjString* name, int argCount) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Metode tidak terdefinisi '%s'.", name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), argCount);
}

static bool invoke(ObjString* name, int argCount) {
    Value receiver = peek(argCount);

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Hanya objek yang memiliki metode.");
        return false;
    }

    ObjInstance* instance = AS_INSTANCE(receiver);

    // A field holding a callable takes precedence over a method.
    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount - 1] = value;
        return callValue(value, argCount);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

static void defineMethod(ObjString* name) {
    Value method = peek(0);
    ObjClass* klass = AS_CLASS(peek(1));
    tableSet(&klass->methods, name, method);
    pop();
}

// Read an entire file into a NUL-terminated heap buffer, or NULL on failure.
static char* readModuleSource(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) return NULL;

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    if (size < 0) { fclose(file); return NULL; }
    rewind(file);

    char* buffer = (char*)malloc((size_t)size + 1);
    if (buffer == NULL) { fclose(file); return NULL; }

    size_t read = fread(buffer, sizeof(char), (size_t)size, file);
    buffer[read] = '\0';
    fclose(file);
    return buffer;
}

// Compile and execute another .idk file in the current global namespace.
// Modules are cached by path so re-importing is a no-op. Returns false on
// a load/compile failure (a runtime error is raised).
static bool importModule(ObjString* path) {
    // Already loaded? Skip.
    Value dummy;
    if (tableGet(&vm.loadedModules, path, &dummy)) {
        return true;
    }

    char* source = readModuleSource(path->chars);
    if (source == NULL) {
        runtimeError("Tidak dapat mengimpor file '%s'.", path->chars);
        return false;
    }

    // Mark as loaded before running to break import cycles.
    tableSet(&vm.loadedModules, path, BOOL_VAL(true));

    ObjFunction* function = compile(source);
    free(source);
    if (function == NULL) {
        runtimeError("Gagal mengompilasi modul '%s'.", path->chars);
        return false;
    }

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    return call(closure, 0);
}

// Unwind to the nearest active 'coba' handler. Restores frame count and
// stack top to the try-block's entry state and points the current frame's
// ip at the 'kecuali' handler. Returns false if there is no handler.
static bool throwValue(Value error) {
    (void)error;
    if (vm.tryCount == 0) return false;

    TryHandler* handler = &vm.tryHandlers[--vm.tryCount];
    // Close any upvalues that live above the try block's stack top.
    closeUpvalues(handler->stackTop);
    vm.frameCount = handler->frameCount;
    vm.stackTop = handler->stackTop;

    CallFrame* frame = &vm.frames[vm.frameCount - 1];
    frame->ip = handler->handlerIp;
    return true;
}

// After a call, a native may have flagged a catchable error via nativeRaise.
// Convert it into a throwValue() unwind. Returns:
//   0 = no pending error (continue normally)
//   1 = error raised and caught (a handler is now active; caller must refresh
//       its frame pointer and push the error value for 'kecuali')
//  -1 = error raised but uncaught (caller should abort with RUNTIME_ERROR)
static int resolveNativeError() {
    if (!vm.nativeErrorPending) return 0;
    vm.nativeErrorPending = false;
    Value error = vm.nativeErrorValue;
    vm.nativeErrorValue = KOSONG_VAL;

    if (!throwValue(error)) {
        if (IS_STRING(error)) {
            runtimeError("%s", AS_CSTRING(error));
        } else {
            runtimeError("Kesalahan tak tertangani dari fungsi bawaan.");
        }
        return -1;
    }
    push(error);  // make the raised value available to 'kecuali'
    return 1;
}

static ObjUpvalue* captureUpvalue(Value* local) {
    ObjUpvalue* prevUpvalue = NULL;
    ObjUpvalue* upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue* createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value* last) {
    while (vm.openUpvalues != NULL &&
           vm.openUpvalues->location >= last) {
        ObjUpvalue* upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

static bool isFalsey(Value value) {
    return IS_KOSONG(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    // Keep operands on the stack (via peek) so a GC triggered by the
    // allocations below cannot free them mid-concatenation.
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)

#define READ_SHORT() \
    (frame->ip += 2, \
    (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))

#define READ_CONSTANT() \
    (frame->closure->function->chunk.constants.values[READ_BYTE()])

#define READ_STRING() AS_STRING(READ_CONSTANT())

#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operand harus berupa angka."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

    for (;;) {
        // A runtime error may have been raised by push()/pop() or a native
        // function without a direct return path. Abort cleanly instead of
        // continuing to execute with a corrupted stack.
        if (vm.hadError) return INTERPRET_RUNTIME_ERROR;

#ifdef DEBUG_TRACE_EXECUTION
        int ipOffset = (int)(frame->ip - frame->closure->function->chunk.code);
        fprintf(stderr, "[LOOP START] ip=%d, frameCount=%d\n", ipOffset, vm.frameCount);
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(&frame->closure->function->chunk, ipOffset);
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_KOSONG: push(KOSONG_VAL); break;
            case OP_BENAR: push(BOOL_VAL(true)); break;
            case OP_SALAH: push(BOOL_VAL(false)); break;
            case OP_POP: pop(); break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                // Don't pop - leave value on stack (assignment is an expression)
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Variabel tidak terdefinisi '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Variabel tidak terdefinisi '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                // Don't pop - leave value on stack (assignment is an expression)
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operand harus dua angka atau dua string.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
            case OP_MODULO: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(fmod(a, b)));
                break;
            }
            case OP_POWER: {
                if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(pow(a, b)));
                break;
            }
            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand harus berupa angka.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                // Ensure we don't jump to invalid bytecode
                uint8_t* newIp = frame->ip - offset;
                if (newIp < frame->closure->function->chunk.code ||
                    newIp >= frame->closure->function->chunk.code + frame->closure->function->chunk.count) {
                    runtimeError("Invalid loop jump offset: offset=%d, ip was %td",
                                 offset, (frame->ip - frame->closure->function->chunk.code));
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame->ip = newIp;
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) {
                    frame->ip += offset;
                }
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                int ne = resolveNativeError();
                if (ne == -1) return INTERPRET_RUNTIME_ERROR;
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_CLOSURE: {
                ObjFunction* function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure* closure = newClosure(function);
                push(OBJ_VAL(closure));
                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }
            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }
            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }
            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;
            case OP_BUILD_LIST: {
                int count = READ_BYTE();
                ObjList* list = newList();
                // Keep the list reachable while appending (append may
                // grow the item array and trigger a GC).
                push(OBJ_VAL(list));
                for (int i = count; i >= 1; i--) {
                    writeValueArray(&list->items, peek(i));
                }
                pop();                       // the list
                vm.stackTop -= count;        // the elements
                push(OBJ_VAL(list));
                break;
            }
            case OP_INDEX_GET: {
                if (IS_LIST(peek(1)) && IS_NUMBER(peek(0))) {
                    int index = (int)AS_NUMBER(pop());
                    ObjList* list = AS_LIST(pop());
                    if (index < 0 || index >= list->items.count) {
                        runtimeError("Indeks list di luar jangkauan (indeks %d, panjang %d).",
                                     index, list->items.count);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    push(list->items.values[index]);
                } else if (IS_DICT(peek(1)) && IS_STRING(peek(0))) {
                    ObjString* key = AS_STRING(pop());
                    ObjDict* dict = AS_DICT(pop());
                    Value value;
                    if (!tableGet(&dict->table, key, &value)) {
                        push(KOSONG_VAL);
                    } else {
                        push(value);
                    }
                } else {
                    runtimeError("Pengindeksan tidak valid: gunakan list[angka] atau kamus[string].");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_INDEX_SET: {
                // Stack: target, index/key, value
                if (IS_LIST(peek(2)) && IS_NUMBER(peek(1))) {
                    Value value = pop();
                    int index = (int)AS_NUMBER(pop());
                    ObjList* list = AS_LIST(pop());
                    if (index < 0 || index >= list->items.count) {
                        runtimeError("Indeks list di luar jangkauan (indeks %d, panjang %d).",
                                     index, list->items.count);
                        return INTERPRET_RUNTIME_ERROR;
                    }
                    list->items.values[index] = value;
                    push(value);
                } else if (IS_DICT(peek(2)) && IS_STRING(peek(1))) {
                    Value value = pop();
                    ObjString* key = AS_STRING(pop());
                    ObjDict* dict = AS_DICT(pop());
                    tableSet(&dict->table, key, value);
                    push(value);
                } else {
                    runtimeError("Pengindeksan tidak valid: gunakan list[angka] atau kamus[string].");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_LIST_LEN: {
                if (!IS_LIST(peek(0))) {
                    runtimeError("'untuk ... dalam' hanya dapat mengiterasi list.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjList* list = AS_LIST(pop());
                push(NUMBER_VAL(list->items.count));
                break;
            }
            case OP_CLASS:
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            case OP_INHERIT: {
                Value superclass = peek(1);
                if (!IS_CLASS(superclass)) {
                    runtimeError("Kelas induk harus berupa kelas.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjClass* subclass = AS_CLASS(peek(0));
                tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
                pop();  // subclass
                break;
            }
            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
            case OP_GET_PROPERTY: {
                if (!IS_INSTANCE(peek(0))) {
                    runtimeError("Hanya objek yang memiliki properti.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(0));
                ObjString* name = READ_STRING();

                Value value;
                if (tableGet(&instance->fields, name, &value)) {
                    pop();  // instance
                    push(value);
                    break;
                }

                if (!bindMethod(instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Hanya objek yang memiliki properti.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                ObjInstance* instance = AS_INSTANCE(peek(1));
                tableSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();  // instance
                push(value);
                break;
            }
            case OP_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                if (!invoke(method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                int ne = resolveNativeError();
                if (ne == -1) return INTERPRET_RUNTIME_ERROR;
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_GET_SUPER: {
                ObjString* name = READ_STRING();
                ObjClass* superclass = AS_CLASS(pop());
                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUPER_INVOKE: {
                ObjString* method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass* superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_TRY_BEGIN: {
                uint16_t offset = READ_SHORT();
                if (vm.tryCount == TRY_MAX) {
                    runtimeError("Terlalu banyak blok 'coba' bersarang.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                TryHandler* handler = &vm.tryHandlers[vm.tryCount++];
                handler->handlerIp = frame->ip + offset;
                handler->frameCount = vm.frameCount;
                handler->stackTop = vm.stackTop;
                break;
            }
            case OP_TRY_END:
                // Normal completion of a 'coba' block: unregister its handler.
                if (vm.tryCount > 0) vm.tryCount--;
                break;
            case OP_THROW: {
                Value error = pop();
                if (!throwValue(error)) {
                    // No handler: report and abort.
                    if (IS_STRING(error)) {
                        runtimeError("Kesalahan tak tertangani: %s", AS_CSTRING(error));
                    } else {
                        runtimeError("Kesalahan tak tertangani.");
                    }
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                push(error);  // make the raised value available to 'kecuali'
                break;
            }
            case OP_IMPORT: {
                ObjString* path = AS_STRING(pop());
                bool alreadyLoaded = false;
                Value dummy;
                if (tableGet(&vm.loadedModules, path, &dummy)) {
                    alreadyLoaded = true;
                }
                if (!importModule(path)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (alreadyLoaded) {
                    // No module frame was pushed; supply the value that the
                    // trailing OP_POP expects.
                    push(KOSONG_VAL);
                } else {
                    // A new frame is now executing the module.
                    frame = &vm.frames[vm.frameCount - 1];
                }
                break;
            }
            case OP_RETURN: {
                Value result = pop();
                closeUpvalues(frame->slots);
                vm.frameCount--;
                if (vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
    vm.hadError = false;
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    call(closure, 0);

    return run();
}
