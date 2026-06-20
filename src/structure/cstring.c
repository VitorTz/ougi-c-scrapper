#include "../../include/structure/cstring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <webp/types.h>



CString cstring_create(const char* value) {
    CString str = {0};

    str.size = (value != NULL) ? strlen(value) : 0;
    str.capacity = (str.size >= 16) ? (str.size + 1) * 2 : 16;
    str.data = (char*) malloc(str.capacity * sizeof(char));
    
    if (str.data != NULL) {
        if (str.size > 0) {
            memcpy(str.data, value, str.size);
            str.data[str.size] = '\0';
        } else {
            str.data[0] = '\0';
        }
    } else {
        str.size = 0;
        str.capacity = 0;
    }
    
    return str;
}


void cstring_destroy(CString* str) {
    if (str->data != NULL) {
        free(str->data);
        str->data = NULL;
    }
    str->size = 0;
    str->capacity = 0;
}


void cstring_reserve(CString* str, const size_t min_capacity) {
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


void cstring_push_back(CString* str, char c) {
    cstring_reserve(str, str->size + 2);
    str->data[str->size] = c;
    str->size++;
    str->data[str->size] = '\0';
}


void cstring_append(CString* str, const char* suffix) {
    if (suffix == NULL) return;
    size_t suffix_len = strlen(suffix);
    cstring_reserve(str, str->size + suffix_len + 1);
    memcpy(str->data + str->size, suffix, suffix_len);
    str->size += suffix_len;
    str->data[str->size] = '\0';
}


bool cstring_assign(CString* str, const char* value) {
    if (!str || !value) return false;
    const size_t len = strlen(value);
    cstring_reserve(str, len + 1);
    memcpy(str->data, value, len + 1);
    str->size = len;
    return true;
}

bool cstring_assign_substr(CString* str, const char* value, const size_t len) {
    if (!str || !value) {
        return false;
    }
    cstring_reserve(str, len + 1);
    memmove(str->data, value, len);
    str->data[len] = '\0';    
    str->size = len;    
    return true;
}


CString cstring_copy(const CString* str) {
    return cstring_create(str->data);
}


const char* cstring_c_str(const CString* str) {
    return str->data;
}


size_t cstring_size(const CString* str) {
    if (!str || !str->data) {return 0;}
    return str->size;
}


bool cstring_is_empty(const CString* str) {
    return str->size == 0;
}


void cstring_clear(CString* str) {
    str->size = 0;
    if (str->capacity > 0 && str->data != NULL) {
        str->data[0] = '\0';
    }
}

/* Compare two strings (equivalent to operator==) */
bool cstring_equals(const CString* str1, const CString* str2) {
    if (str1->size != str2->size) return false;
    return strcmp(str1->data, str2->data) == 0;
}

void cstring_toupper(CString* str) {
    if (!str || !str->data) { return; }

    for (size_t i = 0; i < str->size; i++) {
        str->data[i] = (char)toupper((unsigned char)str->data[i]);
    }
}

void cstring_tolower(CString* str) {
    if (!str || !str->data) { return; }

    for (size_t i = 0; i < str->size; i++) {
        str->data[i] = (char) tolower((unsigned char)str->data[i]);
    }
}


void cstring_trim(CString* str) {
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


CString cstring_trim_copy(const CString* str) {
    CString cpy = cstring_copy(str);
    cstring_trim(&cpy);
    return cpy;
}

void string_split_destroy(Vector* parts) {
    Iterator iter = vector_iter(parts);
    CString* it = NULL;
    while ((it = (CString*) vector_iter_next(&iter)) != NULL) {
        cstring_destroy(it);
    }
    vector_destroy(parts);
}

Vector cstring_split(const CString* str, char delim, bool skip_empty) {    
    Vector parts = vector_create(sizeof(CString), 4);
    
    if (!str || !str->data) {
        return parts;
    }

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
            string_split_destroy(&parts);
            return vector_create(sizeof(CString), 4);
        }

        memcpy(tmp, str->data + token_start, token_len);
        tmp[token_len] = '\0';
        CString aux = cstring_create(tmp);
        vector_push_back(&parts, &aux);
        free(tmp);
        token_start = i + 1;
    }

    return parts;
}


void cstring_split_print(const Vector* parts) {
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


void cstring_print(const CString *str) {
    printf("%s\n", cstring_c_str(str));
}