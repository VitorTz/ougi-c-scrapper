#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../include/structure/string_t.h"
#include "../include/structure/vector.h"

/* ============================================================
 * Test 1: Construction and Destruction
 * ============================================================ */
void test_construction(void) {
    /* Empty string */
    string_t s1 = string_new();
    assert(string_empty(&s1));
    assert(string_length(&s1) == 0);
    assert(strcmp(string_cstr(&s1), "") == 0);

    /* From C-string */
    string_t s2 = string_from("Hello");
    assert(!string_empty(&s2));
    assert(string_length(&s2) == 5);
    assert(strcmp(string_cstr(&s2), "Hello") == 0);

    /* From fixed buffer */
    string_t s3 = string_from_n("Hello World", 5);
    assert(string_length(&s3) == 5);
    assert(strcmp(string_cstr(&s3), "Hello") == 0);

    /* Clone */
    string_t s4 = string_clone(&s2);
    assert(string_length(&s4) == 5);
    assert(strcmp(string_cstr(&s4), "Hello") == 0);
    assert(string_data(&s4) != string_data(&s2)); /* Must be a deep copy */

    /* Format */
    string_t s5 = string_format("Value: %d", 42);
    assert(strcmp(string_cstr(&s5), "Value: 42") == 0);

    string_free(&s1);
    string_free(&s2);
    string_free(&s3);
    string_free(&s4);
    string_free(&s5);
}

/* ============================================================
 * Test 2: Capacity and Memory Management
 * ============================================================ */
void test_capacity(void) {
    string_t s = string_with_capacity(100);
    assert(string_capacity(&s) >= 100);
    assert(string_empty(&s));

    string_append(&s, "Test");
    assert(string_length(&s) == 4);

    /* Shrink to fit */
    string_shrink_to_fit(&s);
    assert(string_capacity(&s) >= 4);

    /* Resize larger */
    string_resize(&s, 8, 'x');
    assert(string_length(&s) == 8);
    assert(strcmp(string_cstr(&s), "Testxxxx") == 0);

    /* Resize smaller */
    string_resize(&s, 4, 'y');
    assert(string_length(&s) == 4);
    assert(strcmp(string_cstr(&s), "Test") == 0);

    string_free(&s);
}

/* ============================================================
 * Test 3: Element Access and Modification
 * ============================================================ */
void test_modifiers(void) {
    string_t s = string_new();

    /* Push and Pop */
    string_push_back(&s, 'A');
    string_push_back(&s, 'B');
    assert(string_length(&s) == 2);
    assert(string_front(&s) == 'A');
    assert(string_back(&s) == 'B');

    string_pop_back(&s);
    assert(strcmp(string_cstr(&s), "A") == 0);

    /* Assign and Append */
    string_assign(&s, "Start");
    string_append_char(&s, '-');
    string_append(&s, "End");
    assert(strcmp(string_cstr(&s), "Start-End") == 0);

    /* Insert */
    string_insert(&s, 5, "-Middle");
    assert(strcmp(string_cstr(&s), "Start-Middle-End") == 0);

    /* Erase */
    string_erase(&s, 5, 7); /* Erase "-Middle" */
    assert(strcmp(string_cstr(&s), "Start-End") == 0);

    /* Replace */
    string_replace(&s, 0, 5, "Begin");
    assert(strcmp(string_cstr(&s), "Begin-End") == 0);

    /* Swap */
    string_t s2 = string_from("Other");
    string_swap(&s, &s2);
    assert(strcmp(string_cstr(&s), "Other") == 0);
    assert(strcmp(string_cstr(&s2), "Begin-End") == 0);

    string_free(&s);
    string_free(&s2);
}

/* ============================================================
 * Test 4: Transformations (Trim, Case, Slugify)
 * ============================================================ */
void test_transformations(void) {
    /* Trimming */
    string_t s1 = string_from("   Spaces   ");
    string_trim(&s1);
    assert(strcmp(string_cstr(&s1), "Spaces") == 0);
    string_free(&s1);

    string_t s2 = string_from("---Data---");
    string_strip_char(&s2, '-');
    assert(strcmp(string_cstr(&s2), "Data") == 0);
    string_free(&s2);

    /* Case conversion */
    string_t s3 = string_from("HeLlO");
    string_to_lower(&s3);
    assert(strcmp(string_cstr(&s3), "hello") == 0);
    string_to_upper(&s3);
    assert(strcmp(string_cstr(&s3), "HELLO") == 0);
    string_free(&s3);

    /* Slugify (Assuming it converts to lowercase, replaces spaces/symbols with hyphens) */
    string_t s4 = string_from("This is a Tést!");
    string_t slug = string_slugify(&s4);
    assert(strcmp(string_cstr(&slug), "this-is-a-test") == 0 || 
           strcmp(string_cstr(&slug), "this-is-a-t-st") == 0); /* Depends on specific implementation */
    string_free(&s4);
    string_free(&slug);
}

/* ============================================================
 * Test 5: Search and Operations
 * ============================================================ */
void test_operations(void) {
    string_t s1 = string_from("Apple");
    string_t s2 = string_from("Banana");

    /* Compare */
    assert(string_compare(&s1, &s2) < 0);
    assert(string_equals_cstr(&s1, "Apple"));

    /* Concat */
    string_t s3 = string_concat(&s1, &s2);
    assert(strcmp(string_cstr(&s3), "AppleBanana") == 0);

    /* Substr */
    string_t sub = string_substr(&s3, 5, 6);
    assert(strcmp(string_cstr(&sub), "Banana") == 0);

    /* Find */
    string_t haystack = string_from("hello world, hello universe");
    assert(string_find(&haystack, "hello", 0) == 0);
    assert(string_find(&haystack, "hello", 5) == 13);
    assert(string_rfind(&haystack, "hello") == 13);
    assert(string_find_char(&haystack, 'w', 0) == 6);
    assert(string_find(&haystack, "missing", 0) == STRING_NPOS);

    /* Prefixes and Suffixes */
    assert(string_starts_with(&haystack, "hello"));
    assert(!string_starts_with(&haystack, "world"));
    assert(string_ends_with(&haystack, "universe"));
    assert(string_contains(&haystack, "world"));

    string_free(&s1);
    string_free(&s2);
    string_free(&s3);
    string_free(&sub);
    string_free(&haystack);
}

/* ============================================================
 * Test 6: Split Array
 * ============================================================ */
void test_split(void) {
    string_t s = string_from("apple,banana,orange");
    
    string_t *parts = string_split(&s, ',');
    assert(parts != NULL);
    
    /* Assuming vec_len is available from vector.h */
    assert(vec_len(parts) == 3);
    assert(strcmp(string_cstr(&parts[0]), "apple") == 0);
    assert(strcmp(string_cstr(&parts[1]), "banana") == 0);
    assert(strcmp(string_cstr(&parts[2]), "orange") == 0);

    string_array_free(&parts);
    assert(parts == NULL);

    string_free(&s);
}

/* ============================================================
 * Main
 * ============================================================ */
int main(void) {
    printf("Running string_t tests...\n");

    test_construction();
    printf("[-] Construction and Destruction tests passed.\n");

    test_capacity();
    printf("[-] Capacity and Memory tests passed.\n");

    test_modifiers();
    printf("[-] Element Modification tests passed.\n");

    test_transformations();
    printf("[-] Transformations (Trim/Case/Slugify) tests passed.\n");

    test_operations();
    printf("[-] Search and Compare operations tests passed.\n");

    test_split();
    printf("[-] Split operations tests passed.\n");

    printf("All string_t tests completed successfully!\n");
    return 0;
}