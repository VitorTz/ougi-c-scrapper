#ifndef CSTRING_H
#define CSTRING_H
#include <stddef.h>
#include <stdbool.h>


typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} CString;

/* Initialize an empty string */
CString string_init(const char* value);

/* Free the allocated memory (equivalent to destructor) */
void string_destroy(CString* str);

/* Append a single character (equivalent to push_back) */
void string_push_back(CString* str, char c);

/* Append a C-style string (equivalent to append or +=) */
void string_append(CString* str, const char* suffix);

/* Get the C-style string (equivalent to c_str()) */
const char* string_c_str(const CString* str);

/* Get current length (equivalent to size() or length()) */
size_t string_size(const CString* str);

/* Check if string is empty (equivalent to empty()) */
bool string_empty(const CString* str);

/* Clear the content without freeing capacity (equivalent to clear()) */
void string_clear(CString* str);

/* Compare two strings (equivalent to operator==) */
bool string_equals(const CString* str1, const CString* str2);


void string_print(const CString* str);


#endif