#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * v4: столбцовый перебор с переносом.
 *
 * идея: идём по разрядам справа налево, как складывают в столбик.
 * на каждом разряде знаем перенос из младшего разряда и цифры слагаемых
 * в этом разряде. если все цифры известны - проверяем арифметику
 * и переходим к следующему разряду. если какие-то буквы не назначены -
 * перебираем их цифры.
 *
 * отсечение происходит на каждом разряде, а не в самом конце.
 *
 * ограничение: считаем, что буква результата в разряде не совпадает
 * с буквами слагаемых в этом же разряде. для таких случаев
 * используйте v1/v2/v3.
 */

#define V4_MAX_TERMS 7

static const rebus_t *g_r;

static int g_digit[26];
static int g_leading[26];
static int g_max_len;

// буква слова word в разряде pos (0 = единицы), -1 если за пределами
static int get_letter(const char *word, int word_len, int pos) {
    if (pos >= word_len) return -1;
    return word[word_len - 1 - pos] - 'A';
}

// используется ли цифра d
static int digit_used(int d) {
    for (int i = 0; i < 26; i++) {
        if (g_digit[i] == d) return 1;
    }
    return 0;
}

// слово в число
static long long word_to_number(const char *word) {
    long long v = 0;
    for (const char *p = word; *p; p++) {
        v = v * 10 + g_digit[*p - 'A'];
    }
    return v;
}

// собрать строку решения
static char *format_solution(void) {
    size_t cap = 1024;
    char *buf = malloc(cap);
    if (!buf) return NULL;

    size_t pos = 0;
    buf[0] = '\0';

    for (int i = 0; i < g_r->n_terms; i++) {
        if (i > 0) {
            int n = snprintf(buf + pos, cap - pos, " + ");
            if (n < 0) { free(buf); return NULL; }
            pos += (size_t)n;
        }
        long long v = word_to_number(g_r->terms[i]);
        int n = snprintf(buf + pos, cap - pos, "%lld", v);
        if (n < 0) { free(buf); return NULL; }
        pos += (size_t)n;
    }

    long long res = word_to_number(g_r->result);
    snprintf(buf + pos, cap - pos, " = %lld", res);

    return buf;
}

// перебор по столбцу: pos - разряд, carry - перенос
static int rec_column(int pos, int carry) {
    if (pos >= g_max_len) {
        return carry == 0;
    }

    int term_letters[V4_MAX_TERMS];
    int term_count = 0;
    int sum_known = carry;
    int has_unknown = 0;

    for (int i = 0; i < g_r->n_terms; i++) {
        int wlen = (int)strlen(g_r->terms[i]);
        int L = get_letter(g_r->terms[i], wlen, pos);
        if (L < 0) continue;
        term_letters[term_count++] = L;
        if (g_digit[L] == -1) {
            has_unknown = 1;
        } else {
            sum_known += g_digit[L];
        }
    }

    int rlen = (int)strlen(g_r->result);
    int R = get_letter(g_r->result, rlen, pos);

    // все буквы слагаемых известны
    if (!has_unknown) {
        if (R < 0) {
            if (sum_known != 0) return 0;
            return rec_column(pos + 1, 0);
        }

        int expected = sum_known % 10;
        int new_carry = sum_known / 10;

        if (g_digit[R] == -1) {
            if (g_leading[R] && expected == 0) return 0;
            if (digit_used(expected)) return 0;
            g_digit[R] = expected;
            if (rec_column(pos + 1, new_carry)) return 1;
            g_digit[R] = -1;
            return 0;
        } else {
            if (g_digit[R] != expected) return 0;
            return rec_column(pos + 1, new_carry);
        }
    }

    // есть неизвестные - перебираем первую
    for (int i = 0; i < term_count; i++) {
        int L = term_letters[i];
        if (g_digit[L] != -1) continue;

        for (int d = 0; d <= 9; d++) {
            if (digit_used(d)) continue;
            if (g_leading[L] && d == 0) continue;

            g_digit[L] = d;
            if (rec_column(pos, carry)) return 1;
            g_digit[L] = -1;
        }
        return 0;
    }

    return 0;
}

char *solve_v4(const char *puzzle) {
    rebus_t r;
    if (rebus_parse(puzzle, &r) != 0) return NULL;

    g_r = &r;

    for (int i = 0; i < 26; i++) g_digit[i] = -1;
    memset(g_leading, 0, sizeof(g_leading));

    g_max_len = 0;
    for (int i = 0; i < r.n_terms; i++) {
        int len = (int)strlen(r.terms[i]);
        if (len > g_max_len) g_max_len = len;
        if (len > 1) g_leading[r.terms[i][0] - 'A'] = 1;
    }
    int rlen = (int)strlen(r.result);
    if (rlen > g_max_len) g_max_len = rlen;
    if (rlen > 1) g_leading[r.result[0] - 'A'] = 1;

    if (rec_column(0, 0)) {
        return format_solution();
    }
    return NULL;
}
