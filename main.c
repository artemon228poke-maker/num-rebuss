#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// читает строки из stdin, для каждой вызывает solve
int main(void) {
    char line[512];

    while (fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        if (len == 0 || line[0] == '#') continue;

        char *solution = solve(line);
        if (solution) {
            printf("%s\n", solution);
            free(solution);
        } else {
            printf("no solution: %s\n", line);
        }
    }

    return 0;
}
