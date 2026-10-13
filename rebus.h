#ifndef REBUS_H
#define REBUS_H

#include <stddef.h>

#define MAX_TERMS 7
#define MAX_WORD 32

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} op_kind_t;

typedef struct {
    char raw[512];
    int n_terms;
    char terms[MAX_TERMS][MAX_WORD];
    char result[MAX_WORD];
    char letters[32];
    int n_letters;
    op_kind_t op;
} rebus_t;

int rebus_parse(const char *input, rebus_t *out);
char *solve(const char *puzzle);

#endif
