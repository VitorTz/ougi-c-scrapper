#include "../../include/structure/cstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <webp/types.h>



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


void string_destroy(CString* str) {
    if (str->data != NULL) {
        free(str->data);
        str->data = NULL;
    }
    str->size = 0;
    str->capacity = 0;
}

void string_reserve(CString* str, const size_t min_capacity) {
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


void string_push_back(CString* str, char c) {
    string_reserve(str, str->size + 2);
    str->data[str->size] = c;
    str->size++;
    str->data[str->size] = '\0';
}


void string_append(CString* str, const char* suffix) {
    if (suffix == NULL) return;
    size_t suffix_len = strlen(suffix);
    string_reserve(str, str->size + suffix_len + 1);
    memcpy(str->data + str->size, suffix, suffix_len);
    str->size += suffix_len;
    str->data[str->size] = '\0';
}


bool string_assign(CString* str, const char* value) {
    if (!str || !value) return false;
    const size_t len = strlen(value);
    string_reserve(str, len + 1);
    memcpy(str->data, value, len + 1);
    str->size = len;
    return true;
}

bool string_assign_substr(CString* str, const char* value, const size_t len) {
    if (!str || !value) {
        return false;
    }
    string_reserve(str, len + 1);
    memmove(str->data, value, len);
    str->data[len] = '\0';    
    str->size = len;    
    return true;
}


CString string_copy(const CString* str) {
    return string_init(str->data);
}


const char* string_c_str(const CString* str) {
    return str->data;
}


size_t string_size(const CString* str) {
    if (!str || !str->data) {return 0;}
    return str->size;
}


bool string_is_empty(const CString* str) {
    return str->size == 0;
}


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

void string_toupper(CString* str) {
    if (!str || !str->data) { return; }

    for (size_t i = 0; i < str->size; i++) {
        str->data[i] = (char)toupper((unsigned char)str->data[i]);
    }
}

void string_tolower(CString* str) {
    if (!str || !str->data) { return; }

    for (size_t i = 0; i < str->size; i++) {
        str->data[i] = (char) tolower((unsigned char)str->data[i]);
    }
}


void string_trim(CString* str) {
    if (!str || !str->data || str->size == 0) return;

    size_t start = 0;
    while (start < str->size && isspace((unsigned char)str->data[start])) {
        start++;
    }

    if (start == str->size) {
        str->data[0] = '\0';
        str->size = 0;
        return;
    }

    size_t end = str->size;
    while (end > start && isspace((unsigned char)str->data[end - 1])) {
        end--;
    }

    size_t new_size = end - start;
    if (start > 0) {
        memmove(str->data, str->data + start, new_size);
    }
    str->data[new_size] = '\0';
    str->size = new_size;
}


CString string_trim_copy(const CString* str) {
    CString cpy = string_copy(str);
    string_trim(&cpy);
    return cpy;
}

void string_split_destroy(Vector* parts) {
    Iterator iter = vector_iter(parts);
    CString* it = NULL;
    while ((it = (CString*) vector_iter_next(&iter)) != NULL) {
        string_destroy(it);
    }
    vector_destroy(parts);
}

Vector* string_split(const CString* str, char delim, bool skip_empty) {    
    if (!str || !str->data) return NULL;
    Vector* parts = vector_create(sizeof(CString), 4);

    size_t token_start = 0;

    for (size_t i = 0; i <= str->size; i++) {
        bool at_delim = (i == str->size) || (str->data[i] == delim);
        if (!at_delim) continue;

        const size_t token_len = i - token_start;
        if (token_len == 0 && skip_empty) {
            token_start = i + 1;
            continue;
        }

        char* tmp = (char*) malloc(token_len + 1);
        if (!tmp) {
            string_split_destroy(parts);
            return NULL;
        }

        memcpy(tmp, str->data + token_start, token_len);
        tmp[token_len] = '\0';
        CString aux = string_init(tmp);
        vector_push_back(parts, &aux);
        free(tmp);
        token_start = i + 1;
    }

    return parts;
}


void string_split_print(const Vector* parts) {
    if (parts == NULL) { 
        printf("None\n");
        return; 
    }
    
    printf("[");
    
    Iterator iter = vector_iter(parts);
    CString* it = NULL;
    bool is_first = true;
    
    while ((it = (CString*) vector_iter_next(&iter)) != NULL) {
        if (!is_first) printf(", ");
        printf("'%s'", it->data);        
        is_first = false;
    }
    
    printf("]\n");
}


void string_print(const CString *str) {
    printf("%s\n", string_c_str(str));
}