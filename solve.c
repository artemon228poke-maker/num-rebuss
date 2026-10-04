#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// слово в число по таблице цифр
static long long word_to_number(const char *word, const int *digit_of_letter) {
    long long v = 0;
    for (const char *p = word; *p; p++) {
        v = v * 10 + digit_of_letter[*p - 'A'];
    }
    return v;
}

// есть ли ведущий ноль
static int has_leading_zero(const char *word, const int *digit_of_letter) {
    if (strlen(word) <= 1) return 0;
    return digit_of_letter[word[0] - 'A'] == 0;
}

// проверка что сумма сходится
static int check_arith(const rebus_t *r, const int *digit_of_letter) {
    long long sum = 0;
    for (int i = 0; i < r->n_terms; i++) {
        sum += word_to_number(r->terms[i], digit_of_letter);
    }
    long long res = word_to_number(r->result, digit_of_letter);
    return sum == res;
}

// собрать строку вида 9567 + 1085 = 10652
static char *format_solution(const rebus_t *r, const int *digit_of_letter) {
    size_t cap = 1024;
    char *buf = malloc(cap);
    if (!buf) return NULL;

    size_t pos = 0;
    buf[0] = '\0';

    for (int i = 0; i < r->n_terms; i++) {
        if (i > 0) {
            int n = snprintf(buf + pos, cap - pos, " + ");
            if (n < 0) { free(buf); return NULL; }
            pos += (size_t)n;
        }
        long long v = word_to_number(r->terms[i], digit_of_letter);
        int n = snprintf(buf + pos, cap - pos, "%lld", v);
        if (n < 0) { free(buf); return NULL; }
        pos += (size_t)n;
    }

    long long res = word_to_number(r->result, digit_of_letter);
    snprintf(buf + pos, cap - pos, " = %lld", res);

    return buf;
}

// рекурсивный перебор цифр по буквам
static int rec_naive(const rebus_t *r, int idx,
                     int *digit_of_letter, int *used_digit,
                     char **out_solution) {
    // все буквы назначены, проверяем
    if (idx == r->n_letters) {
        for (int i = 0; i < r->n_terms; i++) {
            if (has_leading_zero(r->terms[i], digit_of_letter)) return 0;
        }
        if (has_leading_zero(r->result, digit_of_letter)) return 0;
        if (!check_arith(r, digit_of_letter)) return 0;

        *out_solution = format_solution(r, digit_of_letter);
        return (*out_solution != NULL) ? 1 : 0;
    }

    int letter_idx = r->letters[idx] - 'A';

    for (int d = 0; d <= 9; d++) {
        if (used_digit[d]) continue;

        used_digit[d] = 1;
        digit_of_letter[letter_idx] = d;

        if (rec_naive(r, idx + 1, digit_of_letter, used_digit, out_solution))
            return 1;

        used_digit[d] = 0;
        digit_of_letter[letter_idx] = -1;
    }

    return 0;
}

char *solve(const char *puzzle) {
    rebus_t r;
    if (rebus_parse(puzzle, &r) != 0) return NULL;

    int digit_of_letter[26];
    for (int i = 0; i < 26; i++) digit_of_letter[i] = -1;

    int used_digit[10] = {0};

    char *solution = NULL;
    if (rec_naive(&r, 0, digit_of_letter, used_digit, &solution))
        return solution;

    return NULL;
}
