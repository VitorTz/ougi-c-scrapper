#include "../../include/structure/vector.h"
#include "../../include/structure/string_t.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

/* ---------- erro de alocacao (equivalente a operator new falhando) ---------- */
static void string__alloc_error(void) {
    fprintf(stderr, "string_t: falha ao alocar memoria\n");
    abort();
}

static void string__bounds_error(size_t index, size_t length) {
    fprintf(stderr, "string_t: indice %zu fora dos limites (tamanho = %zu)\n", index, length);
    abort();
}

/* garante 'data' com pelo menos min_capacity bytes utilizaveis +1 para o '\0' */
static void string__ensure_capacity(string_t *s, size_t min_capacity) {
    if (min_capacity <= s->capacity) return;

    size_t new_cap = s->capacity ? s->capacity * 2 : 16;
    if (new_cap < min_capacity) new_cap = min_capacity;

    char *new_data = (char *)realloc(s->data, new_cap + 1);
    if (!new_data) string__alloc_error();

    s->data = new_data;
    s->capacity = new_cap;
}

/* ================= construcao / destruicao ================= */

string_t string_new(void) {
    string_t s;
    s.data = (char *)malloc(1);
    if (!s.data) string__alloc_error();
    s.data[0] = '\0';
    s.length = 0;
    s.capacity = 0;
    return s;
}

string_t string_from_n(const char *data, size_t n) {
    string_t s = string_new();
    if (n > 0 && data) {
        string__ensure_capacity(&s, n);
        memcpy(s.data, data, n);
        s.length = n;
        s.data[n] = '\0';
    }
    return s;
}

string_t string_from(const char *cstr) {
    return string_from_n(cstr, cstr ? strlen(cstr) : 0);
}

string_t string_with_capacity(size_t capacity) {
    string_t s = string_new();
    if (capacity > 0) string__ensure_capacity(&s, capacity);
    return s;
}

string_t string_clone(const string_t *s) {
    return string_from_n(s->data, s->length);
}

string_t string_format(const char *fmt, ...) {
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    string_t s = string_new();
    if (needed > 0) {
        string__ensure_capacity(&s, (size_t)needed);
        vsnprintf(s.data, (size_t)needed + 1, fmt, args_copy);
        s.length = (size_t)needed;
    }
    va_end(args_copy);
    return s;
}

void string_free(string_t *s) {
    if (!s) return;
    free(s->data);
    s->data = NULL;
    s->length = 0;
    s->capacity = 0;
}

/* ================= capacidade ================= */

size_t string_length(const string_t *s)   { 
    return s->length; 
}

size_t string_size(const string_t *s)     { 
    return s->length; 
}

size_t string_capacity(const string_t *s) { 
    return s->capacity; 
}


int string_empty(const string_t *s) { 
    return s->length == 0; 
}

void string_reserve(string_t *s, size_t new_capacity) {
    string__ensure_capacity(s, new_capacity);
}

void string_shrink_to_fit(string_t *s) {
    if (s->capacity == s->length) return;
    char *nd = (char *)realloc(s->data, s->length + 1);
    if (!nd) string__alloc_error();
    s->data = nd;
    s->capacity = s->length;
}

void string_resize(string_t *s, size_t n, char fill) {
    if (n <= s->length) {
        s->length = n;
    } else {
        string__ensure_capacity(s, n);
        memset(s->data + s->length, (unsigned char)fill, n - s->length);
        s->length = n;
    }
    s->data[s->length] = '\0';
}

/* ================= acesso a elementos ================= */

char string_at(const string_t *s, size_t index) {
    if (index >= s->length) string__bounds_error(index, s->length);
    return s->data[index];
}

char string_front(const string_t *s) { 
    return s->data[0]; 
}

char string_back(const string_t *s)  { 
    return s->data[s->length - 1]; 
}

const char *string_cstr(const string_t *s) { 
    return s->data; 
}

char *string_data(string_t *s) { 
    return s->data; 
}

/* ================= modificadores ================= */

void string_clear(string_t *s) {
    s->length = 0;
    s->data[0] = '\0';
}

void string_push_back(string_t *s, char c) {
    string__ensure_capacity(s, s->length + 1);
    s->data[s->length++] = c;
    s->data[s->length] = '\0';
}

void string_pop_back(string_t *s) {
    if (s->length == 0) return;
    s->length--;
    s->data[s->length] = '\0';
}

void string_assign_n(string_t *s, const char *data, size_t n) {
    string__ensure_capacity(s, n);
    if (n > 0 && data) memmove(s->data, data, n);
    s->length = n;
    s->data[n] = '\0';
}

void string_assign(string_t *s, const char *cstr) {
    if (cstr) {
        string_assign_n(s, cstr, cstr ? strlen(cstr) : 0);
    }
}

void string_append_n(string_t *s, const char *data, size_t n) {
    if (n == 0 || !data) return;
    string__ensure_capacity(s, s->length + n);
    memmove(s->data + s->length, data, n);
    s->length += n;
    s->data[s->length] = '\0';
}

void string_append(string_t *s, const char *cstr) {
    if (cstr) string_append_n(s, cstr, strlen(cstr));
}

void string_append_char(string_t *s, char c) {
    string_push_back(s, c);
}

void string_append_string(string_t *s, const string_t *other) {
    string_append_n(s, other->data, other->length);
}

void string_append_format(string_t *s, const char *fmt, ...) {
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed > 0) {
        string__ensure_capacity(s, s->length + (size_t)needed);
        vsnprintf(s->data + s->length, (size_t)needed + 1, fmt, args_copy);
        s->length += (size_t)needed;
    }
    va_end(args_copy);
}

void string_insert(string_t *s, size_t pos, const char *cstr) {
    size_t n = cstr ? strlen(cstr) : 0;
    if (n == 0) return;
    string__ensure_capacity(s, s->length + n);
    /* desloca o restante (incluindo o '\0') n posicoes para a direita */
    memmove(s->data + pos + n, s->data + pos, s->length - pos + 1);
    memcpy(s->data + pos, cstr, n);
    s->length += n;
}

void string_erase(string_t *s, size_t pos, size_t len) {
    if (pos >= s->length) return;
    if (len > s->length - pos) len = s->length - pos;
    /* desloca o restante (incluindo o '\0') para cobrir o trecho removido */
    memmove(s->data + pos, s->data + pos + len, s->length - pos - len + 1);
    s->length -= len;
}

void string_replace(string_t *s, size_t pos, size_t len, const char *replacement) {
    if (pos > s->length) return;
    if (len > s->length - pos) len = s->length - pos;

    size_t rlen = replacement ? strlen(replacement) : 0;
    size_t tail_len = s->length - pos - len;

    if (rlen > len) string__ensure_capacity(s, s->length - len + rlen);

    memmove(s->data + pos + rlen, s->data + pos + len, tail_len + 1); /* +1 = '\0' */
    if (rlen > 0) memcpy(s->data + pos, replacement, rlen);
    s->length = s->length - len + rlen;
}

void string_swap(string_t *a, string_t *b) {
    string_t tmp = *a;
    *a = *b;
    *b = tmp;
}

void string_trim_left(string_t *s) {
    size_t i = 0;
    while (i < s->length && isspace((unsigned char)s->data[i])) i++;
    if (i > 0) {
        memmove(s->data, s->data + i, s->length - i + 1);
        s->length -= i;
    }
}

void string_trim_right(string_t *s) {
    while (s->length > 0 && isspace((unsigned char)s->data[s->length - 1])) {
        s->length--;
    }
    s->data[s->length] = '\0';
}

void string_trim(string_t *s) {
    string_trim_right(s);
    string_trim_left(s);
}

void string_to_upper(string_t *s) {
    for (size_t i = 0; i < s->length; i++)
        s->data[i] = (char)toupper((unsigned char)s->data[i]);
}

void string_to_lower(string_t *s) {
    for (size_t i = 0; i < s->length; i++)
        s->data[i] = (char)tolower((unsigned char)s->data[i]);
}

/* ================= operacoes ================= */

int string_compare(const string_t *a, const string_t *b) {
    size_t min_len = a->length < b->length ? a->length : b->length;
    int r = memcmp(a->data, b->data, min_len);
    if (r != 0) return r;
    if (a->length < b->length) return -1;
    if (a->length > b->length) return 1;
    return 0;
}

int string_compare_cstr(const string_t *a, const char *cstr) {
    return strcmp(a->data, cstr ? cstr : "");
}

int string_equals(const string_t *a, const string_t *b) {
    return a->length == b->length && memcmp(a->data, b->data, a->length) == 0;
}

int string_equals_cstr(const string_t *a, const char *cstr) {
    if (!cstr) return a->length == 0;
    return strcmp(a->data, cstr) == 0;
}

string_t string_substr(const string_t *s, size_t pos, size_t len) {
    if (pos > s->length) pos = s->length;
    if (len > s->length - pos) len = s->length - pos;
    return string_from_n(s->data + pos, len);
}

string_t string_concat(const string_t *a, const string_t *b) {
    string_t r = string_with_capacity(a->length + b->length);
    string_append_n(&r, a->data, a->length);
    string_append_n(&r, b->data, b->length);
    return r;
}

size_t string_find(const string_t *s, const char *needle, size_t start_pos) {
    if (!needle || start_pos > s->length) return STRING_NPOS;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return start_pos;
    if (start_pos == s->length) return STRING_NPOS;

    const char *found = strstr(s->data + start_pos, needle);
    if (!found) return STRING_NPOS;
    return (size_t)(found - s->data);
}

size_t string_find_char(const string_t *s, char c, size_t start_pos) {
    if (start_pos >= s->length) return STRING_NPOS;
    const char *found = (const char *)memchr(s->data + start_pos, (unsigned char)c, s->length - start_pos);
    if (!found) return STRING_NPOS;
    return (size_t)(found - s->data);
}

size_t string_rfind(const string_t *s, const char *needle) {
    if (!needle) return STRING_NPOS;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return s->length;
    if (needle_len > s->length) return STRING_NPOS;

    size_t i = s->length - needle_len;
    for (;;) {
        if (memcmp(s->data + i, needle, needle_len) == 0) return i;
        if (i == 0) break;
        i--;
    }
    return STRING_NPOS;
}

int string_starts_with(const string_t *s, const char *prefix) {
    if (!prefix) return 0;
    size_t plen = strlen(prefix);
    if (plen > s->length) return 0;
    return memcmp(s->data, prefix, plen) == 0;
}

int string_ends_with(const string_t *s, const char *suffix) {
    if (!suffix) return 0;
    size_t slen = strlen(suffix);
    if (slen > s->length) return 0;
    return memcmp(s->data + s->length - slen, suffix, slen) == 0;
}

int string_contains(const string_t *s, const char *needle) {
    return string_find(s, needle, 0) != STRING_NPOS;
}

/* ================= split ================= */

string_t *string_split(const string_t *s, char delim) {
    string_t *result = NULL;
    size_t start = 0;

    for (size_t i = 0; i <= s->length; i++) {
        if (i == s->length || s->data[i] == delim) {
            string_t piece = string_from_n(s->data + start, i - start);
            vec_push_back(result, piece);
            start = i + 1;
        }
    }
    return result;
}

void string_array_free(string_t **arr) {
    if (!arr || !*arr) return;
    size_t n = vec_len(*arr);
    for (size_t i = 0; i < n; i++) string_free(&(*arr)[i]);
    vec_free(*arr);
}


void string_print(const string_t *s) {
    printf("%s\n", s->data);
}