#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// слово в число по таблице цифр
static long long v3_word_to_number(const char *word, const int *digit_of_letter) {
    long long v = 0;
    for (const char *p = word; *p; p++) {
        v = v * 10 + digit_of_letter[*p - 'A'];
    }
    return v;
}

// проверка что сумма сходится
static int v3_check_arith(const rebus_t *r, const int *digit_of_letter) {
    long long sum = 0;
    for (int i = 0; i < r->n_terms; i++) {
        sum += v3_word_to_number(r->terms[i], digit_of_letter);
    }
    long long res = v3_word_to_number(r->result, digit_of_letter);
    return sum == res;
}

// собрать строку решения
static char *v3_format_solution(const rebus_t *r, const int *digit_of_letter) {
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
        long long v = v3_word_to_number(r->terms[i], digit_of_letter);
        int n = snprintf(buf + pos, cap - pos, "%lld", v);
        if (n < 0) { free(buf); return NULL; }
        pos += (size_t)n;
    }

    long long res = v3_word_to_number(r->result, digit_of_letter);
    snprintf(buf + pos, cap - pos, " = %lld", res);

    return buf;
}

// посчитать сколько раз каждая буква встречается в ребусе
static void v3_count_freq(const rebus_t *r, int *freq) {
    memset(freq, 0, 26 * sizeof(int));
    for (int i = 0; i < r->n_terms; i++) {
        for (const char *p = r->terms[i]; *p; p++) freq[*p - 'A']++;
    }
    for (const char *p = r->result; *p; p++) freq[*p - 'A']++;
}

// отметить какие буквы ведущие
static void v3_mark_leading(const rebus_t *r, int *is_leading) {
    memset(is_leading, 0, 26 * sizeof(int));
    for (int i = 0; i < r->n_terms; i++) {
        if (strlen(r->terms[i]) > 1) is_leading[r->terms[i][0] - 'A'] = 1;
    }
    if (strlen(r->result) > 1) is_leading[r->result[0] - 'A'] = 1;
}

// сортировка букв: сначала с большей частотой, при равной частоте ведущие раньше
static void v3_sort_letters(char *letters, int n, const int *freq, const int *is_leading) {
    // простая сортировка вставками, n максимум 10
    for (int i = 1; i < n; i++) {
        char key = letters[i];
        int key_idx = key - 'A';
        int j = i - 1;

        while (j >= 0) {
            char cur = letters[j];
            int cur_idx = cur - 'A';

            int better = 0;
            if (freq[key_idx] > freq[cur_idx]) better = 1;
            else if (freq[key_idx] == freq[cur_idx]) {
                if (is_leading[key_idx] && !is_leading[cur_idx]) better = 1;
            }

            if (!better) break;
            letters[j + 1] = letters[j];
            j--;
        }
        letters[j + 1] = key;
    }
}

// перебор с ранним отсечением, буквы идут по убыванию частоты
static int v3_rec(const rebus_t *r, const char *order, int idx, const int *is_leading,
                  int *digit_of_letter, unsigned used_mask,
                  char **out_solution) {
    if (idx == r->n_letters) {
        if (!v3_check_arith(r, digit_of_letter)) return 0;
        *out_solution = v3_format_solution(r, digit_of_letter);
        return (*out_solution != NULL) ? 1 : 0;
    }

    int letter_idx = order[idx] - 'A';
    int leading = is_leading[letter_idx];

    for (int d = 0; d <= 9; d++) {
        if (used_mask & (1u << d)) continue;
        if (leading && d == 0) continue;

        digit_of_letter[letter_idx] = d;
        if (v3_rec(r, order, idx + 1, is_leading, digit_of_letter,
                   used_mask | (1u << d), out_solution))
            return 1;
        digit_of_letter[letter_idx] = -1;
    }

    return 0;
}

char *solve_v3(const char *puzzle) {
    rebus_t r;
    if (rebus_parse(puzzle, &r) != 0) return NULL;

    int freq[26];
    int is_leading[26];
    v3_count_freq(&r, freq);
    v3_mark_leading(&r, is_leading);

    // копируем буквы в отдельный массив и сортируем
    char order[32];
    memcpy(order, r.letters, r.n_letters);
    v3_sort_letters(order, r.n_letters, freq, is_leading);

    int digit_of_letter[26];
    for (int i = 0; i < 26; i++) digit_of_letter[i] = -1;

    char *solution = NULL;
    if (v3_rec(&r, order, 0, is_leading, digit_of_letter, 0u, &solution))
        return solution;

    return NULL;
}
