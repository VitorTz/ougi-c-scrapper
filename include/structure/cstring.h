#ifndef CSTRING_H
#define CSTRING_H
#include "vector.h"
#include <stddef.h>
#include <stdbool.h>


typedef struct {
    char* data;
    size_t size;
    size_t capacity;
} CString;


/* Initialize an empty string */
CString cstring_create(const char* value);

/* Free the allocated memory (equivalent to destructor) */
void cstring_destroy(CString* str);

void cstring_reserve(CString* str, size_t min_capacity);

/* Append a single character (equivalent to push_back) */
void cstring_push_back(CString* str, char c);

/* Append a C-style string (equivalent to append or +=) */
void cstring_append(CString* str, const char* suffix);

bool cstring_assign(CString* str, const char* value);

bool cstring_assign_substr(CString* str, const char* value, size_t len);

CString cstring_copy(const CString* str);


/* Get the C-style string (equivalent to c_str()) */
const char* cstring_c_str(const CString* str);

/* Get current length (equivalent to size() or length()) */
size_t cstring_size(const CString* str);

/* Check if string is empty (equivalent to empty()) */
bool cstring_is_empty(const CString* str);

/* Clear the content without freeing capacity (equivalent to clear()) */
void cstring_clear(CString* str);

/* Compare two strings (equivalent to operator==) */
bool cstring_equals(const CString* str1, const CString* str2);

void cstring_toupper(CString* str);

void cstring_tolower(CString* str);

void cstring_trim(CString* str);

CString cstring_trim_copy(const CString* str);

Vector cstring_split(const CString* str, char delim, bool skip_empty);

void cstring_split_destroy(Vector* parts);

void cstring_split_print(const Vector* parts);

void cstring_print(const CString* str);


#endif