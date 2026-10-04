#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// версия 2 объявлена в solve_v2.c
char *solve_v2(const char *puzzle);

// читает строки из stdin, вызывает нужную версию
int main(int argc, char **argv) {
    int use_v2 = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v2") == 0) {
            use_v2 = 1;
        }
    }

    char line[512];

    while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len == 0 || line[0] == '#') continue;

        char *solution = use_v2 ? solve_v2(line) : solve(line);
        if (solution) {
            printf("%s\n", solution);
            free(solution);
        } else {
            printf("no solution: %s\n", line);
        }
    }

    return 0;
}
