#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/structure/map.h"

/* ============================================================
 * Test 1: Simple Types (int key -> float value)
 * ============================================================ */

typedef struct {
    int key;
    float value;
} IntFloatMap;

void test_simple_types(void) {
    IntFloatMap *map = NULL;

    hashmap_new(map, hashmap_hash_int, hashmap_eq_int);

    assert(hashmap_empty(map));
    assert(hashmap_size(map) == 0);

    hashmap_put(map, 10, 3.14f);
    hashmap_put(map, 20, 2.71f);
    assert(hashmap_size(map) == 2);
    assert(!hashmap_empty(map));

    float *val_10 = hashmap_get(map, 10);
    assert(val_10 != NULL && *val_10 == 3.14f);

    float *val_20 = hashmap_get(map, 20);
    assert(val_20 != NULL && *val_20 == 2.71f);

    float *val_missing = hashmap_get(map, 99);
    assert(val_missing == NULL);

    assert(hashmap_contains(map, 10));
    assert(!hashmap_contains(map, 99));

    hashmap_put(map, 10, 6.28f);
    assert(hashmap_size(map) == 2);
    assert(*hashmap_get(map, 10) == 6.28f);

    int removed = hashmap_remove(map, 20);
    assert(removed == 1);
    assert(hashmap_size(map) == 1);
    assert(!hashmap_contains(map, 20));

    hashmap_clear(map);
    assert(hashmap_size(map) == 0);
    assert(!hashmap_contains(map, 10));

    hashmap_free(map);
    assert(map == NULL);
}

/* ============================================================
 * Test 2: String Keys (const char* key -> int value)
 * ============================================================ */

typedef struct {
    const char *key;
    int value;
} StrIntMap;

void test_string_keys(void) {
    StrIntMap *map = NULL;

    hashmap_new(map, hashmap_hash_str, hashmap_eq_str);

    hashmap_put(map, "apple",  5);
    hashmap_put(map, "banana", 10);
    hashmap_put(map, "orange", 15);

    assert(hashmap_size(map) == 3);
    assert(*hashmap_get(map, "banana") == 10);
    assert(hashmap_get(map, "grape") == NULL);

    /* Caso simples sem break: hashmap_foreach é suficiente. */
    int sum   = 0;
    int count = 0;
    hashmap_foreach(StrIntMap, it, map) {
        sum += it->value;
        count++;
    }
    assert(count == 3);
    assert(sum == 30); /* 5 + 10 + 15 */

    hashmap_free(map);
}

/* ============================================================
 * Test 3: Complex Struct Keys and Values
 * ============================================================ */

typedef struct { int x; int y; } Point;

HASHMAP_DEFAULT_HASHERS(Point, hash_point, eq_point)

typedef struct {
    double  metadata;
    char   *name;
} ComplexValue;

typedef struct {
    Point        key;
    ComplexValue value;
} PointMap;

void test_complex_types(void) {
    PointMap *map = NULL;
    hashmap_new(map, hash_point, eq_point);

    Point p1 = {10, 20};
    Point p2 = {-5, 15};

    ComplexValue v1 = {1.5, strdup("Point Alpha")};
    ComplexValue v2 = {2.5, strdup("Point Beta")};

    hashmap_put(map, p1, v1);
    hashmap_put(map, p2, v2);

    assert(hashmap_size(map) == 2);

    ComplexValue *retrieved = hashmap_get(map, p1);
    assert(retrieved != NULL);
    assert(retrieved->metadata == 1.5);
    assert(strcmp(retrieved->name, "Point Alpha") == 0);

    /*
     * Cleanup de recursos com hm_iter: break funciona corretamente,
     * o que é importante caso um free intermediário precise ser
     * abortado (ex.: detecção de erro).
     */
    hm_iter_t it = hm_iter(map);
    PointMap *entry;
    while ((entry = hm_iter_next(&it, map)) != NULL) {
        free(entry->value.name);
    }

    hashmap_free(map);
}

/* ============================================================
 * Test 4: Stress Testing (Rehashing & Tombstones)
 * ============================================================ */

void test_stress_and_rehashing(void) {
    IntFloatMap *map = NULL;
    hashmap_new(map, hashmap_hash_int, hashmap_eq_int);

    for (int i = 0; i < 20; i++) {
        hashmap_put(map, i, (float)(i * 2));
    }

    assert(hashmap_size(map) == 20);
    assert(hashmap_capacity(map) >= 32);

    for (int i = 0; i < 20; i++) {
        float *val = hashmap_get(map, i);
        assert(val != NULL && *val == (float)(i * 2));
    }

    for (int i = 0; i < 15; i++) {
        hashmap_remove(map, i);
    }

    assert(hashmap_size(map) == 5);
    assert(hashmap_get(map, 0) == NULL);
    assert(*hashmap_get(map, 19) == 38.0f);

    for (int i = 0; i < 30; i++) {
        hashmap_put(map, i + 100, (float)i);
    }

    assert(hashmap_size(map) == 35);
    assert(*hashmap_get(map, 105) == 5.0f);

    hashmap_free(map);
}

/* ============================================================
 * Test 5: Iterator — break/continue correctness
 * ============================================================ */

void test_iter_break(void) {
    IntFloatMap *map = NULL;
    hashmap_new(map, hashmap_hash_int, hashmap_eq_int);

    for (int i = 0; i < 10; i++) {
        hashmap_put(map, i, (float)i);
    }

    assert(hashmap_size(map) == 10);

    /*
     * Para na primeira chave par encontrada.
     * Com o double-for antigo, o break saía só do inner loop
     * e a iteração continuava — aqui deve parar de verdade.
     */
    int found_key = -1;
    int steps     = 0;

    hm_iter_t it = hm_iter(map);
    IntFloatMap *entry;
    while ((entry = hm_iter_next(&it, map)) != NULL) {
        steps++;
        if (entry->key % 2 == 0) {
            found_key = entry->key;
            break;
        }
    }

    assert(found_key != -1);       /* alguma chave par foi encontrada   */
    assert(steps < 10);            /* iteração parou antes de esgotar   */

    /*
     * Retoma iteração a partir de onde parou:
     * os elementos restantes somados com os já visitados
     * devem cobrir todos os 10 slots ocupados.
     */
    int remaining = 0;
    while ((entry = hm_iter_next(&it, map)) != NULL) {
        remaining++;
    }

    assert(steps + remaining == 10);

    hashmap_free(map);
}

/* ============================================================
 * Main
 * ============================================================ */

int main(void) {
    printf("Running hashmap tests...\n");

    test_simple_types();
    printf("[-] Simple types tests passed.\n");

    test_string_keys();
    printf("[-] String keys tests passed.\n");

    test_complex_types();
    printf("[-] Complex types tests passed.\n");

    test_stress_and_rehashing();
    printf("[-] Stress and rehashing tests passed.\n");

    test_iter_break();
    printf("[-] Iterator break/continue tests passed.\n");

    printf("All tests completed successfully!\n");
    return 0;
}