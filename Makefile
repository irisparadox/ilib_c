CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O3

TARGET = out/main

all: $(TARGET)

out:
	mkdir -p out

$(TARGET): main.c | out
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf out

run:
	out/main

.PHONY: all clean