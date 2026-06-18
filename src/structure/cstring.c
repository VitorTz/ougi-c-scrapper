#include "../../include/structure/cstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Initialize an empty string */
CString string_init(const char* value) {
    CString str = {
        .size = 0,
        .capacity = 16,
        .data = (char*)malloc(16 * sizeof(char))
    };
    if (str.data != NULL) {
        str.data[0] = '\0';
    }
    string_append(&str, value);
    return str;
}

/* Free the allocated memory (equivalent to destructor) */
void string_destroy(CString* str) {
    if (str->data != NULL) {
        free(str->data);
        str->data = NULL;
    }
    str->size = 0;
    str->capacity = 0;
}

/* Internal function to resize capacity if needed */
static void string_reserve(CString* str, size_t min_capacity) {
    if (min_capacity > str->capacity) {
        size_t new_capacity = str->capacity * 2;
        if (new_capacity < min_capacity) {
            new_capacity = min_capacity;
        }
        char* new_data = (char*)realloc(str->data, new_capacity * sizeof(char));
        if (new_data != NULL) {
            str->data = new_data;
            str->capacity = new_capacity;
        }
    }
}

/* Append a single character (equivalent to push_back) */
void string_push_back(CString* str, char c) {
    string_reserve(str, str->size + 2);
    str->data[str->size] = c;
    str->size++;
    str->data[str->size] = '\0';
}

/* Append a C-style string (equivalent to append or +=) */
void string_append(CString* str, const char* suffix) {
    if (suffix == NULL) return;
    size_t suffix_len = strlen(suffix);
    string_reserve(str, str->size + suffix_len + 1);
    memcpy(str->data + str->size, suffix, suffix_len);
    str->size += suffix_len;
    str->data[str->size] = '\0';
}

/* Get the C-style string (equivalent to c_str()) */
const char* string_c_str(const CString* str) {
    return str->data;
}

/* Get current length (equivalent to size() or length()) */
size_t string_size(const CString* str) {
    return str->size;
}

/* Check if string is empty (equivalent to empty()) */
bool string_empty(const CString* str) {
    return str->size == 0;
}

/* Clear the content without freeing capacity (equivalent to clear()) */
void string_clear(CString* str) {
    str->size = 0;
    if (str->capacity > 0 && str->data != NULL) {
        str->data[0] = '\0';
    }
}

/* Compare two strings (equivalent to operator==) */
bool string_equals(const CString* str1, const CString* str2) {
    if (str1->size != str2->size) return false;
    return strcmp(str1->data, str2->data) == 0;
}

void string_print(const CString *str) {
    printf("%s\n", string_c_str(str));
}