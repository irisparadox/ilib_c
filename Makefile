CC = gcc
CFLAGS = -D_GNU_SOURCE -D_POSIX_C_SOURCE=200809L -Iinclude -Iarch -Wall -Wextra -std=c99 -pedantic -pthread
DEBUGF = -fsanitize=address,undefined -fno-omit-frame-pointer -g3 -O0

TARGET = out/main

ARCH = x86-64
ARCHDIR = arch/$(ARCH)

ASMSRC = $(wildcard $(ARCHDIR)/*.S $(ARCHDIR)/*.c)
LIBSRC = $(wildcard src/*.c)

SRC = tests/schedstress.c $(ASMSRC) $(LIBSRC)

GENOFFSETS = out/genoffsets
ASMOFFSETS = arch/x86-64/i_context_asm.h

all: $(TARGET)

out:
	mkdir -p out

$(GENOFFSETS): buildtools/genoffsets.c | out
	$(CC) $(CFLAGS) $(DEBUGF) $< -o $@

$(ASMOFFSETS): $(GENOFFSETS) include/icontext.h arch/x86-64/i_context.h
	$(GENOFFSETS) > $@

$(TARGET): $(SRC) $(ASMOFFSETS) | out
	$(CC) $(CFLAGS) $(DEBUGF) $(SRC) -o $@

clean:
	rm -rf out

run:
	make
	out/main

.PHONY: all clean run
