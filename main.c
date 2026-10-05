#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// версии объявлены в solve_v2.c и solve_v3.c
char *solve_v2(const char *puzzle);
char *solve_v3(const char *puzzle);

// читает строки из stdin, вызывает нужную версию
int main(int argc, char **argv) {
    int version = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v2") == 0) version = 2;
        else if (strcmp(argv[i], "-v3") == 0) version = 3;
    }

    char line[512];

    while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len == 0 || line[0] == '#') continue;

        char *solution = NULL;
        if (version == 3) solution = solve_v3(line);
        else if (version == 2) solution = solve_v2(line);
        else solution = solve(line);

        if (solution) {
            printf("%s\n", solution);
            free(solution);
        } else {
            printf("no solution: %s\n", line);
        }
    }

    return 0;
}
