#include "rebus.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return s;
    char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return s;
}

static int copy_word(const char *src, char *dest, size_t cap) {
    size_t len = strlen(src);
    if (len == 0 || len >= cap) return -1;
    for (size_t i = 0; i < len; i++) {
        if (src[i] < 'A' || src[i] > 'Z') return -1;
    }
    memcpy(dest, src, len + 1);
    return 0;
}

static int parse_op(char c, op_kind_t *out) {
    switch (c) {
        case '+': *out = OP_ADD; return 0;
        case '-': *out = OP_SUB; return 0;
        case '*': *out = OP_MUL; return 0;
        case '/': *out = OP_DIV; return 0;
        default:  return -1;
    }
}

int rebus_parse(const char *input, rebus_t *out) {
    if (!input || !out) return -1;
    memset(out, 0, sizeof(*out));
    out->op = OP_ADD;

    size_t in_len = strlen(input);
    if (in_len == 0 || in_len >= sizeof(out->raw)) return -1;
    memcpy(out->raw, input, in_len + 1);

    char *eq = strchr(out->raw, '=');
    if (!eq) return -1;
    *eq = '\0';

    char *lhs = trim(out->raw);
    char *rhs = trim(eq + 1);
    if (*lhs == '\0' || *rhs == '\0') return -1;

    for (const char *p = rhs; *p; p++) {
        if (*p == ' ') return -1;
    }
    if (copy_word(rhs, out->result, sizeof(out->result)) != 0) return -1;

    out->n_terms = 0;
    int op_set = 0;
    char *p = lhs;
    char *term_start = p;

    while (*p) {
        op_kind_t this_op;
        if (parse_op(*p, &this_op) == 0) {
            if (!op_set) {
                out->op = this_op;
                op_set = 1;
            } else if (out->op != this_op) {
                return -1;
            }
            *p = '\0';
            char *term = trim(term_start);
            if (out->n_terms >= MAX_TERMS) return -1;
            if (copy_word(term, out->terms[out->n_terms], MAX_WORD) != 0) return -1;
            out->n_terms++;
            term_start = p + 1;
        }
        p++;
    }
    {
        char *term = trim(term_start);
        if (out->n_terms >= MAX_TERMS) return -1;
        if (copy_word(term, out->terms[out->n_terms], MAX_WORD) != 0) return -1;
        out->n_terms++;
    }

    if (out->n_terms < 2 || out->n_terms > MAX_TERMS) return -1;

    int seen[26] = {0};
    out->n_letters = 0;

    for (int i = 0; i < out->n_terms; i++) {
        for (const char *q = out->terms[i]; *q; q++) {
            int idx = *q - 'A';
            if (!seen[idx]) {
                seen[idx] = 1;
                out->letters[out->n_letters++] = *q;
            }
        }
    }
    for (const char *q = out->result; *q; q++) {
        int idx = *q - 'A';
        if (!seen[idx]) {
            seen[idx] = 1;
            out->letters[out->n_letters++] = *q;
        }
    }

    if (out->n_letters > 10) return -1;

    return 0;
}
