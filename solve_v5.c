#include "rebus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * v5: столбцовый перебор + эвристика проверки длины результата.
 *
 * перед началом перебора проверяем, может ли результат такой длины
 * получиться из слагаемых таких длин. если нет - сразу no solution.
 *
 * пример: A + B = C, где A и B трёхзначные, C пятизначное.
 * максимум 999 + 999 = 1998, это 4 цифры. пятизначное невозможно.
 * значит решения нет, не тратим время на перебор.
 *
 * работает для всех операций (+, -, *, /).
 * для + и - используется столбцовый метод.
 * для * и / - перебор с отсечениями.
 */

static const rebus_t *g_r;
static int g_digit[26];
static int g_leading[26];
static int g_max_len;
static char g_order[32];
static int g_n_letters;

// максимально возможное значение слова длины n
static long long max_value(int len) {
    long long v = 0;
    for (int i = 0; i < len; i++) v = v * 10 + 9;
    return v;
}

// минимально возможное значение слова длины n (без ведущего нуля)
static long long min_value(int len) {
    if (len <= 1) return 0;
    long long v = 1;
    for (int i = 1; i < len; i++) v = v * 10 + 0;
    return v;
}

/*
 * эвристика длины результата.
 * возвращает 1 если результат теоретически возможен, 0 если точно нет.
 */
static int check_length_feasible(void) {
    int res_len = (int)strlen(g_r->result);
    long long res_max = max_value(res_len);
    long long res_min = min_value(res_len);

    long long op_max, op_min;

    if (g_r->op == OP_ADD) {
        // максимальная сумма = сумма максимумов всех слагаемых
        op_max = 0;
        for (int i = 0; i < g_r->n_terms; i++) {
            op_max += max_value((int)strlen(g_r->terms[i]));
        }
        // минимальная сумма = сумма минимумов (первое слагаемое без ведущего нуля)
        op_min = 0;
        for (int i = 0; i < g_r->n_terms; i++) {
            op_min += min_value((int)strlen(g_r->terms[i]));
        }
    } else if (g_r->op == OP_SUB) {
        // максимальная разность - первое минус минимум остальных
        op_max = max_value((int)strlen(g_r->terms[0]));
        for (int i = 1; i < g_r->n_terms; i++) {
            op_max -= min_value((int)strlen(g_r->terms[i]));
        }
        // минимальная разность
        op_min = min_value((int)strlen(g_r->terms[0]));
        for (int i = 1; i < g_r->n_terms; i++) {
            op_min -= max_value((int)strlen(g_r->terms[i]));
        }
    } else if (g_r->op == OP_MUL) {
        // максимум произведения
        op_max = 1;
        for (int i = 0; i < g_r->n_terms; i++) {
            op_max *= max_value((int)strlen(g_r->terms[i]));
        }
        // минимум произведения
        op_min = 1;
        for (int i = 0; i < g_r->n_terms; i++) {
            op_min *= min_value((int)strlen(g_r->terms[i]));
        }
    } else {  // OP_DIV
        // максимум частного - первое делим на минимум второго
        op_max = max_value((int)strlen(g_r->terms[0]));
        for (int i = 1; i < g_r->n_terms; i++) {
            long long denom_min = min_value((int)strlen(g_r->terms[i]));
            if (denom_min == 0) denom_min = 1;
            op_max /= denom_min;
        }
        // минимум частного - первое делим на максимум
        op_min = min_value((int)strlen(g_r->terms[0]));
        for (int i = 1; i < g_r->n_terms; i++) {
            long long denom_max = max_value((int)strlen(g_r->terms[i]));
            if (denom_max == 0) denom_max = 1;
            op_min /= denom_max;
        }
        if (op_min < 0) op_min = 0;
    }

    // если диапазоны не пересекаются - решения нет
    if (op_max < res_min) return 0;
    if (op_min > res_max) return 0;

    return 1;
}

static long long word_to_number(const char *word) {
    long long v = 0;
    for (const char *p = word; *p; p++) {
        v = v * 10 + g_digit[*p - 'A'];
    }
    return v;
}

static char *format_solution(void) {
    size_t cap = 1024;
    char *buf = malloc(cap);
    if (!buf) return NULL;

    size_t pos = 0;
    buf[0] = '\0';

    const char *op_str;
    switch (g_r->op) {
        case OP_ADD: op_str = " + "; break;
        case OP_SUB: op_str = " - "; break;
        case OP_MUL: op_str = " * "; break;
        case OP_DIV: op_str = " / "; break;
        default: free(buf); return NULL;
    }

    for (int i = 0; i < g_r->n_terms; i++) {
        if (i > 0) {
            int n = snprintf(buf + pos, cap - pos, "%s", op_str);
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

static void mark_leading(void) {
    memset(g_leading, 0, sizeof(g_leading));
    for (int i = 0; i < g_r->n_terms; i++) {
        if (strlen(g_r->terms[i]) > 1) g_leading[g_r->terms[i][0] - 'A'] = 1;
    }
    if (strlen(g_r->result) > 1) g_leading[g_r->result[0] - 'A'] = 1;
}

static int check_arith(void) {
    long long vals[7];
    for (int i = 0; i < g_r->n_terms; i++) {
        vals[i] = word_to_number(g_r->terms[i]);
    }
    long long res = word_to_number(g_r->result);

    long long acc = vals[0];
    for (int i = 1; i < g_r->n_terms; i++) {
        switch (g_r->op) {
            case OP_ADD: acc += vals[i]; break;
            case OP_SUB: acc -= vals[i]; break;
            case OP_MUL: acc *= vals[i]; break;
            case OP_DIV:
                if (vals[i] == 0) return 0;
                if (acc % vals[i] != 0) return 0;
                acc /= vals[i];
                break;
        }
    }
    return acc == res;
}

// перебор для *, /
static int rec_bruteforce(int idx) {
    if (idx == g_n_letters) {
        return check_arith();
    }

    int L = g_order[idx] - 'A';
    int leading = g_leading[L];

    for (int d = 0; d <= 9; d++) {
        int used = 0;
        for (int i = 0; i < idx; i++) {
            if (g_digit[g_order[i] - 'A'] == d) { used = 1; break; }
        }
        if (used) continue;
        if (leading && d == 0) continue;

        g_digit[L] = d;
        if (rec_bruteforce(idx + 1)) return 1;
        g_digit[L] = -1;
    }

    return 0;
}

static int freq_of(int letter) {
    int f = 0;
    for (int i = 0; i < g_r->n_terms; i++) {
        for (const char *p = g_r->terms[i]; *p; p++) {
            if (*p - 'A' == letter) f++;
        }
    }
    for (const char *p = g_r->result; *p; p++) {
        if (*p - 'A' == letter) f++;
    }
    return f;
}

static void sort_letters(void) {
    for (int i = 1; i < g_n_letters; i++) {
        char key = g_order[i];
        int key_idx = key - 'A';
        int j = i - 1;

        while (j >= 0) {
            int cur_idx = g_order[j] - 'A';
            int better = 0;

            int fk = freq_of(key_idx);
            int fc = freq_of(cur_idx);

            if (fk > fc) better = 1;
            else if (fk == fc && g_leading[key_idx] && !g_leading[cur_idx]) better = 1;

            if (!better) break;
            g_order[j + 1] = g_order[j];
            j--;
        }
        g_order[j + 1] = key;
    }
}

static char *solve_bruteforce(void) {
    memcpy(g_order, g_r->letters, g_r->n_letters);
    g_n_letters = g_r->n_letters;
    sort_letters();

    for (int i = 0; i < 26; i++) g_digit[i] = -1;

    if (rec_bruteforce(0)) {
        return format_solution();
    }
    return NULL;
}

// столбцовый метод для + и -
static int get_letter(const char *word, int word_len, int pos) {
    if (pos >= word_len) return -1;
    return word[word_len - 1 - pos] - 'A';
}

static int digit_used(int d) {
    for (int i = 0; i < 26; i++) {
        if (g_digit[i] == d) return 1;
    }
    return 0;
}

static int rec_column_add(int pos, int carry) {
    if (pos >= g_max_len) return carry == 0;

    int term_letters[7];
    int term_count = 0;
    int sum_known = carry;
    int has_unknown = 0;

    for (int i = 0; i < g_r->n_terms; i++) {
        int wlen = (int)strlen(g_r->terms[i]);
        int L = get_letter(g_r->terms[i], wlen, pos);
        if (L < 0) continue;
        term_letters[term_count++] = L;
        if (g_digit[L] == -1) has_unknown = 1;
        else sum_known += g_digit[L];
    }

    int rlen = (int)strlen(g_r->result);
    int R = get_letter(g_r->result, rlen, pos);

    if (!has_unknown) {
        if (R < 0) {
            if (sum_known != 0) return 0;
            return rec_column_add(pos + 1, 0);
        }

        int expected = sum_known % 10;
        int new_carry = sum_known / 10;

        if (g_digit[R] == -1) {
            if (g_leading[R] && expected == 0) return 0;
            if (digit_used(expected)) return 0;
            g_digit[R] = expected;
            if (rec_column_add(pos + 1, new_carry)) return 1;
            g_digit[R] = -1;
            return 0;
        } else {
            if (g_digit[R] != expected) return 0;
            return rec_column_add(pos + 1, new_carry);
        }
    }

    for (int i = 0; i < term_count; i++) {
        int L = term_letters[i];
        if (g_digit[L] != -1) continue;

        for (int d = 0; d <= 9; d++) {
            if (digit_used(d)) continue;
            if (g_leading[L] && d == 0) continue;

            g_digit[L] = d;
            if (rec_column_add(pos, carry)) return 1;
            g_digit[L] = -1;
        }
        return 0;
    }

    return 0;
}

static char *solve_column_add(void) {
    for (int i = 0; i < 26; i++) g_digit[i] = -1;

    g_max_len = 0;
    for (int i = 0; i < g_r->n_terms; i++) {
        int len = (int)strlen(g_r->terms[i]);
        if (len > g_max_len) g_max_len = len;
    }
    int rlen = (int)strlen(g_r->result);
    if (rlen > g_max_len) g_max_len = rlen;

    if (rec_column_add(0, 0)) return format_solution();
    return NULL;
}

char *solve_v5(const char *puzzle) {
    rebus_t r;
    if (rebus_parse(puzzle, &r) != 0) return NULL;

    g_r = &r;
    mark_leading();

    // эвристика: если результат по длине невозможен - сразу выход
    if (!check_length_feasible()) {
        return NULL;
    }

    if (r.op == OP_ADD) {
        return solve_column_add();
    } else {
        return solve_bruteforce();
    }
}
