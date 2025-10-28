# Makefile untuk Bahasa Pemrograman Indonesiaku

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2 -Isrc
LDFLAGS = -lm

# Windows executable extension
ifeq ($(OS),Windows_NT)
    EXE = .exe
    RM = del /Q
    RMDIR = rmdir /S /Q
else
    EXE =
    RM = rm -f
    RMDIR = rm -rf
endif

TARGET = indk$(EXE)

SOURCES = src/main.c \
          src/chunk.c \
          src/compiler.c \
          src/debug.c \
          src/memory.c \
          src/object.c \
          src/scanner.c \
          src/table.c \
          src/value.c \
          src/vm.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo Build berhasil! Jalankan dengan: ./$(TARGET) atau ./$(TARGET) file.idk

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET) examples/hello.idk

.PHONY: all clean run test
