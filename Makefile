CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic -pthread

TARGET = out/main

all: $(TARGET)

out:
	mkdir -p out

$(TARGET): main.c | out
	$(CC) $(CFLAGS) main.c -o $@

clean:
	rm -rf out

run:
	make
	out/main

.PHONY: all clean
