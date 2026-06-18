#include "../../include/structure/path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


Path path_init(const char* initial_path) {
    Path p = {0};
    p.length = initial_path ? initial_path != NULL && strlen(initial_path) : 0;
    p.capacity = p.length > 16 ? p.length * 2 : 16;
    p.data = (char*) malloc(p.capacity * sizeof(char));
    
    if (p.data != NULL) {
        if (initial_path != NULL) {
            strcpy(p.data, initial_path);
        } else {
            p.data[0] = '\0';
        }
    }
    return p;
}


void path_destroy(Path* p) {
    if (p->data) {
        free(p->data);
        p->data = NULL;
    }
    p->length = 0;
    p->capacity = 0;
}


static void path_reserve(Path* p, size_t min_capacity) {
    if (min_capacity > p->capacity) {
        size_t new_capacity = p->capacity * 2;
        if (new_capacity < min_capacity) new_capacity = min_capacity;
        
        char* new_data = (char*)realloc(p->data, new_capacity * sizeof(char));
        if (new_data) {
            p->data = new_data;
            p->capacity = new_capacity;
        }
    }
}


void path_append(Path* p, const char* component) {
    if (!component || strlen(component) == 0) return;

    size_t comp_len = strlen(component);
    bool needs_separator = false;

    /* Check if we need to insert a separator */
    if (p->length > 0 && p->data[p->length - 1] != PATH_SEPARATOR) {
        if (component[0] != PATH_SEPARATOR) {
            needs_separator = true;
        }
    }

    size_t required_space = p->length + comp_len + (needs_separator ? 1 : 0) + 1;
    path_reserve(p, required_space);

    if (needs_separator) {
        p->data[p->length] = PATH_SEPARATOR;
        p->length++;
    }

    /* Skip leading separator in component if path already has one */
    if (p->length > 0 && p->data[p->length - 1] == PATH_SEPARATOR && component[0] == PATH_SEPARATOR) {
        component++;
        comp_len--;
    }

    memcpy(p->data + p->length, component, comp_len);
    p->length += comp_len;
    p->data[p->length] = '\0';
}


const char* path_c_str(const Path* p) {
    return p->data;
}


char* path_filename(const Path* p) {
    const char* last_sep = strrchr(p->data, PATH_SEPARATOR);
    if (last_sep) {
        return strdup(last_sep + 1);
    }
    return strdup(p->data); /* No separator found, the whole path is the filename */
}


char* path_extension(const Path* p) {
    char* filename = path_filename(p);
    char* dot = strrchr(filename, '.');
    char* ext = NULL;
    
    if (dot && dot != filename) {
        ext = strdup(dot);
    } else {
        ext = strdup(""); /* No extension found */
    }
    
    free(filename);
    return ext;
}


char* path_parent_path(const Path* p) {
    const char* last_sep = strrchr(p->data, PATH_SEPARATOR);
    if (last_sep == p->data) {
        return strdup(PATH_SEPARATOR_STR); /* Root directory */
    } else if (last_sep != NULL) {
        size_t len = last_sep - p->data;
        char* parent = (char*)malloc(len + 1);
        strncpy(parent, p->data, len);
        parent[len] = '\0';
        return parent;
    }
    return strdup(""); /* No parent path (relative path with one component) */
}