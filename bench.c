#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *solve_v2(const char *puzzle);
char *solve_v3(const char *puzzle);
char *solve_v4(const char *puzzle);
char *solve_v5(const char *puzzle);
char *solve_v6(const char *puzzle);

typedef struct {
    const char *puzzle;
    int skip_v1;
} bench_case_t;

static bench_case_t cases[] = {
    { "SEND + MORE = MONEY", 0 },
    { "BE + BE = MOO", 0 },
    { "BIG + CAT = LION", 0 },
    { "ELEVEN + NINE + FIVE + FIVE = THIRTY", 0 },
    { "TWO - ONE = ONE", 0 },
    { "AB * C = DEF", 0 },
    { "AB / C = D", 0 },
};

#define N_CASES (int)(sizeof(cases) / sizeof(cases[0]))

typedef char *(*solver_fn)(const char *);

static double time_solver(solver_fn fn, const char *puzzle, int *ok) {
    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    char *sol = fn(puzzle);

    clock_gettime(CLOCK_MONOTONIC, &t1);

    *ok = (sol != NULL);
    if (sol) free(sol);

    double ms = (t1.tv_sec - t0.tv_sec) * 1000.0
              + (t1.tv_nsec - t0.tv_nsec) / 1e6;
    return ms;
}

int main(void) {
    printf("=== Benchmark: solver versions ===\n\n");
    printf("%-42s %10s %10s %10s %10s %10s %10s\n",
           "puzzle", "v1(ms)", "v2(ms)", "v3(ms)", "v4(ms)", "v5(ms)", "v6(ms)");
    printf("%-42s %10s %10s %10s %10s %10s %10s\n",
           "------", "------", "------", "------", "------", "------", "------");

    for (int c = 0; c < N_CASES; c++) {
        const char *p = cases[c].puzzle;
        int ok;

        double t1 = -1, t2, t3, t4, t5, t6;

        if (!cases[c].skip_v1) {
            t1 = time_solver(solve, p, &ok);
        }

        t2 = time_solver(solve_v2, p, &ok);
        t3 = time_solver(solve_v3, p, &ok);
        t4 = time_solver(solve_v4, p, &ok);
        t5 = time_solver(solve_v5, p, &ok);
        t6 = time_solver(solve_v6, p, &ok);

        printf("%-42s", p);

        if (t1 < 0) printf(" %10s", "skip");
        else        printf(" %10.3f", t1);

        printf(" %10.3f %10.3f %10.3f %10.3f %10.3f\n",
               t2, t3, t4, t5, t6);
    }

    printf("\nall times in milliseconds (ms)\n");
    return 0;
}
