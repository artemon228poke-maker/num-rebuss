# Makefile — сборка решателя числовых ребусов

CC      = clang
CFLAGS  = -O2 -Wall -Wextra -std=c11

all: rebus

rebus: rebus.c solve.c main.c rebus.h
	$(CC) $(CFLAGS) -o rebus rebus.c solve.c main.c

clean:
	rm -f rebus bench *.o

.PHONY: all rebus clean
