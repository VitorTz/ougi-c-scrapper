/* ============================================================
 *  vector.h - vetor dinamico generico para C (estilo C++ std::vector)
 * ============================================================
 *
 * Funciona com QUALQUER tipo, incluindo structs definidas pelo usuario,
 * sem precisar declarar o tipo em nenhum lugar nem gerar codigo por tipo.
 * O truque: o ponteiro "v" e' o proprio array (igual um array C normal),
 * e um cabecalho com {length, capacity} fica guardado *antes* do
 * primeiro elemento na memoria. Por isso v[i] continua funcionando
 * normalmente, e os macros descobrem o tamanho do elemento sozinhos
 * via sizeof(*(v)).
 *
 * Uso basico:
 *
 *     typedef struct { float x, y; } Ponto;
 *
 *     Ponto *pontos = NULL;            // vetor vazio, igual "std::vector<Ponto> pontos;"
 *
 *     vec_push_back(pontos, ((Ponto){1.0f, 2.0f}));
 *     vec_push_back(pontos, ((Ponto){3.0f, 4.0f}));
 *
 *     for (size_t i = 0; i < vec_len(pontos); i++)
 *         printf("%.1f %.1f\n", pontos[i].x, pontos[i].y);
 *
 *     vec_free(pontos);                // libera memoria, pontos volta a ser NULL
 *
 * Nenhuma chamada de "criacao" e' necessaria: um ponteiro NULL e' um
 * vetor vazio valido, exatamente como o construtor default do
 * std::vector. Todas as funcoes/macros tratam NULL com seguranca.
 *
 * Requer C99 ou superior.
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "introsort.h"


/* ---------- cabecalho interno (fica antes dos dados) ---------- */
typedef struct {
    size_t length;
    size_t capacity;
} vec__header_t;

#define VEC__HDR(v) ((vec__header_t *)(v) - 1)

/* ---------- erro de alocacao / limites (equivalentes a exceptions) ---------- */
static inline void vec__alloc_error(void) {
    fprintf(stderr, "vector: falha ao alocar memoria\n");
    abort();
}

static inline void vec__bounds_error(const char *file, int line, size_t index, size_t length) {
    fprintf(stderr, "vector: indice %zu fora dos limites (tamanho = %zu) em %s:%d\n",
            index, length, file, line);
    abort();
}

/* ---------- garante capacidade >= min_capacity, preservando o conteudo ----------
 * Nao deve ser chamada diretamente pelo usuario; usada pelos macros abaixo. */
static inline void *vec__grow(void *arr, size_t min_capacity, size_t elem_size) {
    size_t old_cap = arr ? VEC__HDR(arr)->capacity : 0;
    if (min_capacity <= old_cap) return arr;

    size_t new_cap = old_cap ? old_cap * 2 : 4;
    if (new_cap < min_capacity) new_cap = min_capacity;

    size_t new_size = sizeof(vec__header_t) + new_cap * elem_size;
    vec__header_t *new_hdr;

    if (arr) {
        new_hdr = (vec__header_t *)realloc(VEC__HDR(arr), new_size);
    } else {
        new_hdr = (vec__header_t *)malloc(new_size);
        if (new_hdr) new_hdr->length = 0;
    }

    if (!new_hdr) vec__alloc_error();

    new_hdr->capacity = new_cap;
    return (void *)(new_hdr + 1);
}

/* ---------- internal helper for vec_contains ---------- */
static inline bool vec__contains_impl(const void *arr, size_t len, size_t elem_size, const void *target, int (*cmp)(const void *, const void *)) {
    if (!arr || len == 0) return false;
    
    const char *ptr = (const char *)arr;
    for (size_t i = 0; i < len; i++) {
        if (cmp(ptr + (i * elem_size), target) == 0) {
            return true;
        }
    }
    return false;
}

/* ================= API ================= */

/* tamanho atual (numero de elementos) */
#define vec_len(v)  ((v) ? VEC__HDR(v)->length : (size_t)0)
#define vec_size(v) vec_len(v)               /* alias, igual std::vector::size() */

/* returns the total number of bytes currently occupied by the elements.
 * it excludes the unused capacity and the internal header size. */
#define vec_total_bytes(v) \
    (vec_len(v) * sizeof(*(v)))

/* capacidade alocada atualmente */
#define vec_cap(v)      ((v) ? VEC__HDR(v)->capacity : (size_t)0)
#define vec_capacity(v) vec_cap(v)

#define vec_empty(v) (vec_len(v) == 0)

/* libera a memoria e zera o ponteiro */
#define vec_free(v) \
    do { if (v) { free(VEC__HDR(v)); (v) = NULL; } } while (0)

/* esvazia o vetor (mantem a memoria/capacidade alocada) */
#define vec_clear(v) \
    do { if (v) VEC__HDR(v)->length = 0; } while (0)

/* garante espaco para pelo menos n elementos, sem alterar o tamanho atual */
#define vec_reserve(v, n) \
    ((v) = vec__grow((v), (size_t)(n), sizeof(*(v))))

/* adiciona um elemento ao final (amortizado O(1), igual push_back) */
#define vec_push_back(v, value) \
    do { \
        (v) = vec__grow((v), vec_len(v) + 1, sizeof(*(v))); \
        (v)[VEC__HDR(v)->length++] = (value); \
    } while (0)

/* assigns raw memory to the vector, overwriting any existing data.
 * uses memmove to safely handle overlapping memory regions. */
#define vec_assign(v, ptr, n_bytes) \
    do { \
        size_t _bytes = (size_t)(n_bytes); \
        if (_bytes == 0) { \
            vec_clear(v); \
        } else { \
            size_t _n_elems = _bytes / sizeof(*(v)); \
            (v) = vec__grow((v), _n_elems, sizeof(*(v))); \
            memmove((v), (ptr), _bytes); \
            VEC__HDR(v)->length = _n_elems; \
        } \
    } while (0)

/* inserts a block of raw memory at the specified index, shifting existing elements.
 * supports inserting at the beginning, middle, or end of the vector. */
#define vec_insert_block(v, idx, ptr, n_bytes) \
    do { \
        size_t _bytes = (size_t)(n_bytes); \
        if (_bytes > 0) { \
            size_t _i = (size_t)(idx); \
            size_t _old_len = vec_len(v); \
            size_t _n_elems = _bytes / sizeof(*(v)); \
            (v) = vec__grow((v), _old_len + _n_elems, sizeof(*(v))); \
            if (_i < _old_len) { \
                memmove(&(v)[_i + _n_elems], &(v)[_i], (_old_len - _i) * sizeof(*(v))); \
            } \
            memmove(&(v)[_i], (ptr), _bytes); \
            VEC__HDR(v)->length += _n_elems; \
        } \
    } while (0)

/* sorts the vector using the standard c library qsort function.
 * the sort_func must match the standard qsort comparator signature:
 * int (*)(const void*, const void*) */
#define vec_sort(v, sort_func) \
    do { \
        if (vec_len(v) > 1) { \
            introsort((v), vec_len(v), sizeof(*(v)), (sort_func)); \
        } \
    } while (0)

/* remove e retorna o ultimo elemento (UB se o vetor estiver vazio,
 * exatamente como pop_back() do C++) */
#define vec_pop_back(v) \
    ((v)[--VEC__HDR(v)->length])

#define vec_front(v) ((v)[0])
#define vec_back(v)  ((v)[vec_len(v) - 1])

/* acesso SEM checagem de limites, igual operator[] -- use v[i] direto.
 * acesso COM checagem de limites, igual .at() (aborta com mensagem se invalido) */
#define vec_at(v, i) \
    (*((size_t)(i) < vec_len(v) \
        ? &(v)[(size_t)(i)] \
        : (vec__bounds_error(__FILE__, __LINE__, (size_t)(i), vec_len(v)), &(v)[(size_t)(i)])))

/* redimensiona para n elementos; elementos novos sao zerados,
 * elementos excedentes sao descartados (igual resize()) */
#define vec_resize(v, n) \
    do { \
        size_t _n = (size_t)(n); \
        if (_n == 0) { \
            vec_clear(v); \
        } else { \
            size_t _old = vec_len(v); \
            (v) = vec__grow((v), _n, sizeof(*(v))); \
            if (_n > _old) memset(&(v)[_old], 0, (_n - _old) * sizeof(*(v))); \
            VEC__HDR(v)->length = _n; \
        } \
    } while (0)

/* insere um elemento na posicao idx, deslocando os seguintes (igual insert()) */
#define vec_insert(v, idx, value) \
    do { \
        size_t _i = (size_t)(idx); \
        (v) = vec__grow((v), vec_len(v) + 1, sizeof(*(v))); \
        memmove(&(v)[_i + 1], &(v)[_i], (VEC__HDR(v)->length - _i) * sizeof(*(v))); \
        (v)[_i] = (value); \
        VEC__HDR(v)->length++; \
    } while (0)

/* remove o elemento na posicao idx, deslocando os seguintes (igual erase()) */
#define vec_erase(v, idx) \
    do { \
        size_t _i = (size_t)(idx); \
        memmove(&(v)[_i], &(v)[_i + 1], (VEC__HDR(v)->length - _i - 1) * sizeof(*(v))); \
        VEC__HDR(v)->length--; \
    } while (0)

/* libera memoria nao usada, deixando capacidade == tamanho (igual shrink_to_fit()) */
#define vec_shrink_to_fit(v) \
    do { \
        if (v) { \
            size_t _len = vec_len(v); \
            if (_len == 0) { \
                vec_free(v); \
            } else { \
                vec__header_t *_h = (vec__header_t *)realloc(VEC__HDR(v), sizeof(vec__header_t) + _len * sizeof(*(v))); \
                if (!_h) vec__alloc_error(); \
                _h->capacity = _len; \
                (v) = (void *)(_h + 1); \
            } \
        } \
    } while (0)

/* faz uma copia independente de src para dst (igual copy constructor) */
#define vec_clone(dst, src) \
    do { \
        size_t _len = vec_len(src); \
        (dst) = NULL; \
        if (_len > 0) { \
            (dst) = vec__grow((dst), _len, sizeof(*(src))); \
            memcpy((dst), (src), _len * sizeof(*(src))); \
            VEC__HDR(dst)->length = _len; \
        } \
    } while (0)

/* clears the vector and fills it with 'n' copies of 'value'.
 * equivalent to std::vector<T> v(n, value) or v.assign(n, value). */
#define vec_fill(v, n, value) \
    do { \
        size_t _n = (size_t)(n); \
        vec_clear(v); \
        if (_n > 0) { \
            (v) = vec__grow((v), _n, sizeof(*(v))); \
            for (size_t _i = 0; _i < _n; _i++) { \
                (v)[_i] = (value); \
            } \
            VEC__HDR(v)->length = _n; \
        } \
    } while (0)

/* iteracao estilo "for (auto& item : v)" -- item e' um ponteiro para o elemento */
#define vec_foreach(type, item, v) \
    for (type *item = (v); item < (v) + vec_len(v); item++)

/* checks if the vector contains a specific item using a custom comparison function.
 * the sort/cmp function must match the standard qsort signature: int (*)(const void*, const void*)
 * returns true if the item is found, false otherwise.
 * note: 'item' must be an lvalue (e.g., a variable) since we take its address under the hood. */
#define vec_contains(v, item, cmp_func) \
    (vec_len(v) > 0 ? vec__contains_impl((v), vec_len(v), sizeof(*(v)), &(item), (cmp_func)) : false)


/* filters the vector in-place in O(N) time.
 * the 'keep_func' must take a pointer to the element type and return a boolean 
 * (true to keep the element, false to remove it).
 * the vector capacity remains unchanged; only the length is reduced. */
#define vec_filter(v, keep_func) \
    do { \
        if (v) { \
            size_t _write_idx = 0; \
            for (size_t _read_idx = 0; _read_idx < vec_len(v); _read_idx++) { \
                if ((keep_func)(&(v)[_read_idx])) { \
                    if (_write_idx != _read_idx) { \
                        (v)[_write_idx] = (v)[_read_idx]; \
                    } \
                    _write_idx++; \
                } \
            } \
            VEC__HDR(v)->length = _write_idx; \
        } \
    } while (0)


/* inverte a ordem de todos os elementos do vetor in-place.
 * funciona com qualquer tipo usando swap via memcpy seguro. */
#define vec_reverse(v) \
    do { \
        if (v) { \
            size_t _len = vec_len(v); \
            if (_len > 1) { \
                size_t _half = _len / 2; \
                for (size_t _i = 0; _i < _half; _i++) { \
                    size_t _j = _len - 1 - _i; \
                    unsigned char _tmp[sizeof(*(v))]; \
                    memcpy(_tmp, &(v)[_i], sizeof(*(v))); \
                    memcpy(&(v)[_i], &(v)[_j], sizeof(*(v))); \
                    memcpy(&(v)[_j], _tmp, sizeof(*(v))); \
                } \
            } \
        } \
    } while (0)


#endif /* VECTOR_H */