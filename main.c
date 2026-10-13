#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *solve_v2(const char *puzzle);
char *solve_v3(const char *puzzle);
char *solve_v4(const char *puzzle);
char *solve_v5(const char *puzzle);
char *solve_v6(const char *puzzle);

int main(int argc, char **argv) {
    int version = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v2") == 0) version = 2;
        else if (strcmp(argv[i], "-v3") == 0) version = 3;
        else if (strcmp(argv[i], "-v4") == 0) version = 4;
        else if (strcmp(argv[i], "-v5") == 0) version = 5;
        else if (strcmp(argv[i], "-v6") == 0) version = 6;
    }

    char line[512];

    while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len == 0 || line[0] == '#') continue;

        char *solution = NULL;
        if (version == 6) solution = solve_v6(line);
        else if (version == 5) solution = solve_v5(line);
        else if (version == 4) solution = solve_v4(line);
        else if (version == 3) solution = solve_v3(line);
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
