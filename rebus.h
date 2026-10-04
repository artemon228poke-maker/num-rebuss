#ifndef REBUS_H
#define REBUS_H

#include <stddef.h>

// максимум слагаемых слева
#define MAX_TERMS 7

// макс длина одного слова
#define MAX_WORD 32

// разобранный ребус
typedef struct {
    char raw[512];
    int n_terms;
    char terms[MAX_TERMS][MAX_WORD];
    char result[MAX_WORD];
    char letters[32];
    int n_letters;
} rebus_t;

// разбор строки, 0 = ок, -1 = ошибка
int rebus_parse(const char *input, rebus_t *out);

// принимает ребус, возвращает строку решения или NULL
// результат надо освободить через free
char *solve(const char *puzzle);

#endif
