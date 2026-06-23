#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/structure/vector.h"

/* ============================================================
 * Helper functions
 * ============================================================ */

/* Comparator function for vec_sort testing */
int compare_ints(const void *a, const void *b) {
    int int_a = *(const int *)a;
    int int_b = *(const int *)b;
    return (int_a > int_b) - (int_a < int_b);
}

/* ============================================================
 * Test Cases
 * ============================================================ */

void test_simple_types(void) {
    int *v = NULL;

    /* Test empty state */
    assert(vec_empty(v));
    assert(vec_len(v) == 0);
    assert(vec_cap(v) == 0);

    /* Test push_back and capacity growth */
    vec_push_back(v, 10);
    vec_push_back(v, 20);
    vec_push_back(v, 30);
    
    assert(!vec_empty(v));
    assert(vec_len(v) == 3);
    assert(vec_cap(v) >= 3);
    assert(vec_front(v) == 10);
    assert(vec_back(v) == 30);
    assert(v[1] == 20);

    /* Test at (bounds checking) */
    assert(vec_at(v, 2) == 30);

    /* Test pop_back */
    int last = vec_pop_back(v);
    assert(last == 30);
    assert(vec_len(v) == 2);

    /* Test insert */
    vec_insert(v, 1, 15);
    assert(vec_len(v) == 3);
    assert(v[1] == 15);
    assert(v[2] == 20);

    /* Test erase */
    vec_erase(v, 1);
    assert(vec_len(v) == 2);
    assert(v[1] == 20);

    /* Test resize */
    vec_resize(v, 5);
    assert(vec_len(v) == 5);
    assert(v[2] == 0); /* new elements should be zero-initialized */
    assert(v[4] == 0);

    /* Test fill */
    vec_fill(v, 4, 42);
    assert(vec_len(v) == 4);
    assert(v[0] == 42 && v[3] == 42);

    /* Test sort */
    vec_clear(v);
    vec_push_back(v, 50);
    vec_push_back(v, 10);
    vec_push_back(v, 30);
    vec_sort(v, compare_ints);
    assert(v[0] == 10);
    assert(v[1] == 30);
    assert(v[2] == 50);

    /* Test clone */
    int *v_clone = NULL;
    vec_clone(v_clone, v);
    assert(vec_len(v_clone) == 3);
    assert(v_clone[0] == 10 && v_clone[2] == 50);
    assert(v_clone != v); /* Must be an independent copy */

    /* Test clear and shrink_to_fit */
    vec_clear(v);
    assert(vec_len(v) == 0);
    assert(vec_cap(v) >= 3); 
    vec_shrink_to_fit(v);
    assert(vec_len(v) == 0);
    assert(vec_cap(v) == 0);

    /* Cleanup */
    vec_free(v);
    vec_free(v_clone);
    assert(v == NULL);
}

/* ------------------------------------------------------------ */

/* Complex struct definition */
typedef struct {
    int id;
    char *name;
    double values[3];
} Entity;

void test_complex_types(void) {
    Entity *entities = NULL;

    /* Push complex types */
    Entity e1 = {1, strdup("Alice"), {1.0, 2.0, 3.0}};
    Entity e2 = {2, strdup("Bob"), {4.0, 5.0, 6.0}};
    
    vec_push_back(entities, e1);
    vec_push_back(entities, e2);

    assert(vec_len(entities) == 2);
    assert(strcmp(entities[0].name, "Alice") == 0);
    assert(entities[1].values[2] == 6.0);

    /* Test foreach loop */
    int count = 0;
    vec_foreach(Entity, it, entities) {
        assert(it->id == count + 1);
        count++;
    }
    assert(count == 2);

    /* Since we dynamically allocated the string inside the struct,
     * we must manually free them before freeing the vector itself */
    vec_foreach(Entity, it, entities) {
        free(it->name);
    }

    vec_free(entities);
}

/* ------------------------------------------------------------ */

void test_block_operations(void) {
    int *v = NULL;
    int raw_data[] = {100, 200, 300};

    /* Test vec_assign */
    vec_assign(v, raw_data, sizeof(raw_data));
    assert(vec_len(v) == 3);
    assert(v[0] == 100 && v[2] == 300);

    /* Test vec_insert_block at the beginning */
    int prefix_data[] = {10, 20};
    vec_insert_block(v, 0, prefix_data, sizeof(prefix_data));
    assert(vec_len(v) == 5);
    assert(v[0] == 10);
    assert(v[2] == 100);

    /* Test vec_insert_block in the middle */
    int mid_data[] = {99};
    vec_insert_block(v, 3, mid_data, sizeof(mid_data));
    assert(vec_len(v) == 6);
    assert(v[3] == 99);
    assert(v[4] == 200);

    vec_free(v);
}

/* ============================================================
 * Main
 * ============================================================ */

int main(void) {
    printf("Running vector tests...\n");

    test_simple_types();
    printf("[-] Simple types tests passed.\n");

    test_complex_types();
    printf("[-] Complex types tests passed.\n");

    test_block_operations();
    printf("[-] Block operations tests passed.\n");

    printf("All tests completed successfully!\n");
    return 0;
}