# Makefile untuk Bahasa Pemrograman Indonesiaku

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -Isrc
LDFLAGS = -lm

# Windows executable extension
ifeq ($(OS),Windows_NT)
    EXE = .exe
    RM = del /Q
    RMDIR = rmdir /S /Q
    # On Windows, del requires backslash paths; / is parsed as a switch.
    CLEAN = del /Q src\*.o $(TARGET) 2>NUL & exit 0
else
    EXE =
    RM = rm -f
    RMDIR = rm -rf
    CLEAN = rm -f $(OBJECTS) $(TARGET)
endif

TARGET = indk$(EXE)

SOURCES = src/main.c \
          src/chunk.c \
          src/compiler.c \
          src/debug.c \
          src/memory.c \
          src/native.c \
          src/object.c \
          src/scanner.c \
          src/table.c \
          src/value.c \
          src/vm.c

OBJECTS = $(SOURCES:.c=.o)

# All headers. Because this project uses a single-pass, tightly-coupled design
# where almost every .c includes many headers, treat every header as a
# dependency of every object. This is coarse but prevents stale builds after a
# header change (the previous rule had no header deps, causing silent
# miscompiles when only .h files changed).
HEADERS = $(wildcard src/*.h)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo Build berhasil! Jalankan dengan: ./$(TARGET) atau ./$(TARGET) file.idk

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(CLEAN)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) examples/hello.idk

.PHONY: all clean run test
