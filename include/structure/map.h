/* ============================================================
 *  hashmap.h - hash map generico para C (estilo C++ std::unordered_map)
 * ============================================================
 *
 * Funciona com QUALQUER chave e QUALQUER valor (inclusive structs
 * definidas pelo usuario), seguindo o mesmo estilo do vector.h: o
 * ponteiro "m" e' o proprio array de entradas, com um cabecalho
 * {count, capacity, hash_fn, eq_fn, ...} guardado *antes* dele na
 * memoria. m[i].key / m[i].value continuam acessiveis normalmente.
 *
 * Implementacao: open addressing (linear probing) com lapides
 * ("tombstones") para remocao, igual ao algoritmo classico descrito
 * no capitulo de hash tables do livro Crafting Interpreters. Cresce
 * (rehash completo) quando o fator de carga passa de 75%, o que
 * tambem limpa as lapides acumuladas.
 *
 * Diferente do std::unordered_map, nao ha' nenhum hash padrao
 * magico para structs arbitrarias (C++ tambem nao tem isso -- exige
 * especializar std::hash<T> para tipos customizados). Aqui voce
 * passa hash_fn/eq_fn explicitamente ao criar o mapa.
 *
 * Convencao obrigatoria da struct de entrada:
 *   - o campo "key"   deve ser o PRIMEIRO membro da struct
 *   - o campo "value" deve se chamar exatamente "value"
 *
 * Uso basico (chave int -> valor float):
 *
 *     typedef struct { int key; float value; } Entrada;
 *     Entrada *mapa = NULL;
 *
 *     hashmap_new(mapa, hashmap_hash_int, hashmap_eq_int);
 *
 *     hashmap_put(mapa, 10, 3.14f);
 *     hashmap_put(mapa, 20, 2.71f);
 *
 *     float *v = hashmap_get(mapa, 10);
 *     if (v) printf("%f\n", *v);
 *
 *     hashmap_remove(mapa, 10);
 *     hashmap_free(mapa);
 *
 * Para chaves do tipo struct, gere hash/eq com um one-liner usando
 * HASHMAP_DEFAULT_HASHERS (ver exemplo mais abaixo), exatamente como
 * voce especializaria std::hash<T> em C++.
 *
 * Requer C99+ e a extensao __typeof__ (suportada por GCC e Clang).
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t (*hashmap_hash_fn)(const void *key);
typedef int    (*hashmap_eq_fn)(const void *a, const void *b);

/* ---------- cabecalho interno ---------- */
typedef struct {
    size_t count;          /* elementos vivos no mapa */
    size_t tombstones;      /* slots removidos (lapides) */
    size_t capacity;        /* numero de slots, sempre potencia de 2 (ou 0) */
    size_t entry_size;      /* sizeof da struct {key; value;} do usuario */
    hashmap_hash_fn hash_fn;
    hashmap_eq_fn   eq_fn;
} hm__header_t;

#define HM__HDR(m) ((hm__header_t *)(m) - 1)

enum { HM__EMPTY = 0, HM__OCCUPIED = 1, HM__DELETED = 2 };

#define HM__LOAD_NUM 3   /* fator de carga maximo = 3/4 = 75% */
#define HM__LOAD_DEN 4
#define HM__MIN_CAPACITY 8

/* cria um ponteiro generico para uma copia temporaria de uma chave,
 * permitindo passar valores literais/rvalues como "const void*".
 * usa um compound literal de ARRAY (nao de struct) para que, quando
 * a chave for ela mesma uma struct/aggregate, (k) seja copiado por
 * inteiro em vez de ser tratado como inicializador do 1o membro. */
#define HM__KEYPTR(m, k) ((__typeof__((m)->key)[]){(k)})

/* ---------- erros (equivalentes a exceptions) ---------- */
static inline void hm__alloc_error(void) {
    fprintf(stderr, "hashmap: falha ao alocar memoria\n");
    abort();
}

/* ---------- helpers de layout ---------- */
static inline uint8_t *hm__states(void *entries, const size_t capacity, const size_t entry_size) {
    return (uint8_t *)entries + capacity * entry_size;
}

static inline size_t hm__next_pow2(size_t n) {
    size_t p = HM__MIN_CAPACITY;
    while (p < n) p <<= 1;
    return p;
}

/* ---------- busca o slot de uma chave (leitura) ----------
 * retorna o indice se encontrado, ou (size_t)-1 caso contrario. */
static inline size_t hm__find(
    void *entries, 
    const size_t capacity, 
    const size_t entry_size,
    const void *key, 
    hashmap_hash_fn hash_fn, 
    hashmap_eq_fn eq_fn
) {
    if (capacity == 0) return (size_t)-1;
    uint8_t *states = hm__states(entries, capacity, entry_size);
    size_t mask = capacity - 1;
    size_t idx = hash_fn(key) & mask;

    for (size_t probe = 0; probe < capacity; probe++) {
        size_t i = (idx + probe) & mask;
        if (states[i] == HM__EMPTY) return (size_t)-1;
        if (states[i] == HM__OCCUPIED) {
            const void *slot_key = (const char *)entries + i * entry_size;
            if (eq_fn(slot_key, key)) return i;
        }
        /* HM__DELETED: continua o probe, lapide nao termina a busca */
    }
    return (size_t)-1;
}

/* ---------- acha (ou reserva) o slot para insercao ----------
 * assume que ja' ha' espaco (chame hm__maybe_grow antes). */
static inline size_t hm__find_slot_for_insert(
    void *entries, 
    const size_t capacity, 
    const size_t entry_size,
    const void *key, 
    hashmap_hash_fn hash_fn,
    hashmap_eq_fn eq_fn,
    int *out_is_new
) {
    uint8_t *states = hm__states(entries, capacity, entry_size);
    size_t mask = capacity - 1;
    size_t idx = hash_fn(key) & mask;
    size_t first_tombstone = (size_t)-1;

    for (size_t probe = 0; probe < capacity; probe++) {
        size_t i = (idx + probe) & mask;
        if (states[i] == HM__OCCUPIED) {
            const void *slot_key = (const char *)entries + i * entry_size;
            if (eq_fn(slot_key, key)) { *out_is_new = 0; return i; }
        } else if (states[i] == HM__DELETED) {
            if (first_tombstone == (size_t)-1) first_tombstone = i;
        } else { /* HM__EMPTY */
            *out_is_new = 1;
            return (first_tombstone != (size_t)-1) ? first_tombstone : i;
        }
    }
    *out_is_new = 1; /* nao deveria acontecer se o fator de carga for respeitado */
    return idx;
}

/* ---------- garante espaco para mais um elemento (rehash completo se necessario) ---------- */
static inline void *hm__maybe_grow(
    void *entries, 
    const size_t entry_size,
    hashmap_hash_fn hash_fn, 
    hashmap_eq_fn eq_fn
) {
    hm__header_t *hdr = HM__HDR(entries);
    size_t used = hdr->count + hdr->tombstones;

    if (hdr->capacity != 0 && (used + 1) * HM__LOAD_DEN <= hdr->capacity * HM__LOAD_NUM) {
        return entries; /* ainda cabe confortavelmente */
    }

    size_t new_capacity = hm__next_pow2((hdr->count + 1) * 2);
    size_t new_block_size = sizeof(hm__header_t) + new_capacity * entry_size + new_capacity;

    hm__header_t *new_hdr = (hm__header_t *)malloc(new_block_size);
    if (!new_hdr) hm__alloc_error();

    void *new_entries = (void *)(new_hdr + 1);
    uint8_t *new_states = hm__states(new_entries, new_capacity, entry_size);
    memset(new_states, HM__EMPTY, new_capacity);

    new_hdr->count = 0;
    new_hdr->tombstones = 0;
    new_hdr->capacity = new_capacity;
    new_hdr->entry_size = entry_size;
    new_hdr->hash_fn = hash_fn;
    new_hdr->eq_fn = eq_fn;

    if (hdr->capacity > 0) {
        uint8_t *old_states = hm__states(entries, hdr->capacity, entry_size);
        for (size_t i = 0; i < hdr->capacity; i++) {
            if (old_states[i] == HM__OCCUPIED) {
                const void *old_key = (const char *)entries + i * entry_size;
                int is_new;
                size_t slot = hm__find_slot_for_insert(new_entries, new_capacity, entry_size,
                                                        old_key, hash_fn, eq_fn, &is_new);
                memcpy((char *)new_entries + slot * entry_size,
                       (char *)entries + i * entry_size, entry_size);
                new_states[slot] = HM__OCCUPIED;
                new_hdr->count++;
            }
        }
    }

    free(hdr);
    return new_entries;
}

static inline void *hm__new(
    const size_t entry_size,
    hashmap_hash_fn hash_fn, 
    hashmap_eq_fn eq_fn
) {
    hm__header_t *hdr = (hm__header_t *)malloc(sizeof(hm__header_t));
    if (!hdr) hm__alloc_error();
    hdr->count = 0;
    hdr->tombstones = 0;
    hdr->capacity = 0;
    hdr->entry_size = entry_size;
    hdr->hash_fn = hash_fn;
    hdr->eq_fn = eq_fn;
    return (void *)(hdr + 1);
}

static inline void *hm__get(
    void *entries, 
    const size_t capacity, 
    const size_t entry_size, 
    const size_t value_offset,
    const void *key, 
    hashmap_hash_fn hash_fn, 
    hashmap_eq_fn eq_fn
) {
    size_t slot = hm__find(entries, capacity, entry_size, key, hash_fn, eq_fn);
    if (slot == (size_t)-1) return NULL;
    return (char *)entries + slot * entry_size + value_offset;
}

static inline int hm__remove(
    void *entries, 
    const size_t capacity, 
    const size_t entry_size,
    const void *key, 
    hashmap_hash_fn hash_fn,
    hashmap_eq_fn eq_fn,
    size_t *count, 
    size_t *tombstones
) {
    size_t slot = hm__find(entries, capacity, entry_size, key, hash_fn, eq_fn);
    if (slot == (size_t)-1) return 0;
    hm__states(entries, capacity, entry_size)[slot] = HM__DELETED;
    (*count)--;
    (*tombstones)++;
    return 1;
}

/* avança _hm_it até o próximo slot OCCUPIED a partir do índice atual,
 * retornando 1 se encontrou um, 0 se esgotou o mapa. */
static inline int hm__iter_next(
    void    *entries,
    size_t   capacity,
    size_t   entry_size,
    size_t  *i,       /* in/out: índice atual */
    void   **out      /* out: ponteiro para a entrada */
) {
    uint8_t *states = hm__states(entries, capacity, entry_size);
    while (*i < capacity) {
        size_t cur = (*i)++;
        if (states[cur] == HM__OCCUPIED) {
            *out = (char *)entries + cur * entry_size;
            return 1;
        }
    }
    return 0;
}

typedef struct { size_t i; } hm_iter_t;

#define hm_iter(m)  ((hm_iter_t){ 0 })

#define hm_iter_next(it, m)                                             \
    ((__typeof__(m)) hm__iter_next_impl(                                \
        (it), (m), HM__HDR(m)->capacity, sizeof(*(m))))

static inline void *hm__iter_next_impl(
    hm_iter_t *it,
    void      *entries,
    size_t     capacity,
    size_t     entry_size
) {
    uint8_t *states = hm__states(entries, capacity, entry_size);
    while (it->i < capacity) {
        size_t cur = it->i++;
        if (states[cur] == HM__OCCUPIED)
            return (char *)entries + cur * entry_size;
    }
    return NULL;
}

/* ================= API ================= */

/* cria o mapa vazio. deve ser chamado antes de qualquer outra operacao,
 * igual ao construtor (implicito em C++, explicito aqui). */
#define hashmap_new(m, hashfn, eqfn) \
    ((m) = hm__new(sizeof(*(m)), (hashfn), (eqfn)))

#define hashmap_free(m) \
    do { if (m) { free(HM__HDR(m)); (m) = NULL; } } while (0)

#define hashmap_size(m)     (HM__HDR(m)->count)
#define hashmap_count(m)    hashmap_size(m)
#define hashmap_capacity(m) (HM__HDR(m)->capacity)
#define hashmap_empty(m)    (hashmap_size(m) == 0)

/* insere ou atualiza (igual operator[]= / insert_or_assign) */
#define hashmap_put(m, k, v) \
    do { \
        (m) = hm__maybe_grow((m), sizeof(*(m)), HM__HDR(m)->hash_fn, HM__HDR(m)->eq_fn); \
        int _hm_is_new; \
        size_t _hm_slot = hm__find_slot_for_insert((m), HM__HDR(m)->capacity, sizeof(*(m)), \
                               HM__KEYPTR(m, k), HM__HDR(m)->hash_fn, HM__HDR(m)->eq_fn, &_hm_is_new); \
        (m)[_hm_slot].key   = (k); \
        (m)[_hm_slot].value = (v); \
        if (_hm_is_new) { \
            hm__states((m), HM__HDR(m)->capacity, sizeof(*(m)))[_hm_slot] = HM__OCCUPIED; \
            HM__HDR(m)->count++; \
        } \
    } while (0)

/* retorna ponteiro para o valor, ou NULL se a chave nao existir (igual find()) */
#define hashmap_get(m, k) \
    ((__typeof__(&(m)->value)) hm__get((m), HM__HDR(m)->capacity, sizeof(*(m)), \
        offsetof(__typeof__(*(m)), value), HM__KEYPTR(m, k), HM__HDR(m)->hash_fn, HM__HDR(m)->eq_fn))

/* 1 se a chave existe, 0 caso contrario (igual contains() / count()) */
#define hashmap_contains(m, k) \
    (hm__find((m), HM__HDR(m)->capacity, sizeof(*(m)), HM__KEYPTR(m, k), \
              HM__HDR(m)->hash_fn, HM__HDR(m)->eq_fn) != (size_t)-1)

/* remove a chave se existir; retorna 1 se removeu, 0 caso contrario (igual erase(key)) */
#define hashmap_remove(m, k) \
    hm__remove((m), HM__HDR(m)->capacity, sizeof(*(m)), HM__KEYPTR(m, k), \
               HM__HDR(m)->hash_fn, HM__HDR(m)->eq_fn, &HM__HDR(m)->count, &HM__HDR(m)->tombstones)

/* remove todas as entradas, mantendo a capacidade alocada */
#define hashmap_clear(m) \
    do { \
        if (HM__HDR(m)->capacity > 0) \
            memset(hm__states((m), HM__HDR(m)->capacity, sizeof(*(m))), HM__EMPTY, HM__HDR(m)->capacity); \
        HM__HDR(m)->count = 0; \
        HM__HDR(m)->tombstones = 0; \
    } while (0)

/* itera sobre as entradas ocupadas. break/continue funcionam corretamente.
 *
 *   hashmap_foreach(Entrada, e, mapa) {
 *       printf("%d -> %f\n", e->key, e->value);
 *   }
 */
#define hashmap_foreach(type, item, m)                                      \
    for (                                                                    \
        struct { size_t i; type *p; } _hm_it = { 0, NULL };                \
        hm__iter_next((m), HM__HDR(m)->capacity, sizeof(*(m)),              \
                      &_hm_it.i, (void **)&_hm_it.p);                      \
    )                                                                        \
        for (type *item = _hm_it.p; item; item = NULL)

/* ---------- hashers/comparadores prontos ---------- */

/* hash/eq genericos por bytes crus -- use para gerar hashers de qualquer
 * tipo POD (sem ponteiros/padding relevante) com HASHMAP_DEFAULT_HASHERS */
static inline size_t hashmap_hash_bytes(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    size_t h = (size_t)14695981039346656037ULL; /* FNV-1a offset basis */
    for (size_t i = 0; i < len; i++) {
        h ^= p[i];
        h *= (size_t)1099511628211ULL; /* FNV-1a prime */
    }
    return h;
}

static inline int hashmap_eq_bytes(const void *a, const void *b, size_t len) {
    return memcmp(a, b, len) == 0;
}

/* gera hash_fn/eq_fn por bytes crus para um tipo de chave especifico,
 * igual a especializar std::hash<T> em C++. uso:
 *     HASHMAP_DEFAULT_HASHERS(MinhaChave, hash_minha_chave, eq_minha_chave)
 *     hashmap_new(mapa, hash_minha_chave, eq_minha_chave); */
#define HASHMAP_DEFAULT_HASHERS(KeyType, hash_name, eq_name) \
    static inline size_t hash_name(const void *key) { return hashmap_hash_bytes(key, sizeof(KeyType)); } \
    static inline int eq_name(const void *a, const void *b) { return hashmap_eq_bytes(a, b, sizeof(KeyType)); }

/* prontos para chave int (caso mais comum) */
static inline size_t hashmap_hash_int(const void *key) { return hashmap_hash_bytes(key, sizeof(int)); }
static inline int    hashmap_eq_int(const void *a, const void *b) { return hashmap_eq_bytes(a, b, sizeof(int)); }

/* prontos para chave do tipo "const char *" (string com dono externo) */
static inline size_t hashmap_hash_str(const void *key) {
    const char *s = *(const char *const *)key;
    size_t h = (size_t)14695981039346656037ULL;
    while (*s) {
        h ^= (uint8_t)(*s++);
        h *= (size_t)1099511628211ULL;
    }
    return h;
}

static inline int hashmap_eq_str(const void *a, const void *b) {
    const char *sa = *(const char *const *)a;
    const char *sb = *(const char *const *)b;
    return strcmp(sa, sb) == 0;
}

#ifdef __cplusplus
}
#endif

#endif /* HASHMAP_H */