# Makefile — сборка решателя числовых ребусов

CC      = clang
CFLAGS  = -O2 -Wall -Wextra -std=c11

SRC       = rebus.c solve.c solve_v2.c solve_v3.c solve_v4.c solve_v5.c solve_v6.c main.c
BENCH_SRC = rebus.c solve.c solve_v2.c solve_v3.c solve_v4.c solve_v5.c solve_v6.c bench.c

all: rebus bench

rebus: $(SRC) rebus.h
	$(CC) $(CFLAGS) -o rebus $(SRC)

bench: $(BENCH_SRC) rebus.h
	$(CC) $(CFLAGS) -o bench $(BENCH_SRC)

clean:
	rm -f rebus bench *.o

.PHONY: all rebus bench clean
