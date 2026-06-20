#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <webp/types.h>

#include "../../include/structure/path.h"


Path path_init(const char* initial_path) {
    const Path path = string_init(initial_path);
    return path;
}


void path_destroy(Path* path) {
    string_destroy(path);
}


void path_append(Path* p, const char* component) {
    if (!component || strlen(component) == 0) {
        return;
    }

    bool path_ends_with_sep = (p->size > 0 && p->data[p->size - 1] == PATH_SEPARATOR);
    bool comp_starts_with_sep = (component[0] == PATH_SEPARATOR);

    if (!path_ends_with_sep && !comp_starts_with_sep && p->size > 0) {
        string_push_back(p, PATH_SEPARATOR);
    } else if (path_ends_with_sep && comp_starts_with_sep) {
        component++;
    }

    string_append(p, component);
}


const char* path_c_str(const Path* p) {
    return string_c_str(p);
}


char* path_filename(const Path* p) {
    const char* last_sep = strrchr(p->data, PATH_SEPARATOR);
    if (last_sep) { return strdup(last_sep + 1); }
    return strdup(p->data);
}


char* path_extension(const Path* p) {
    char* filename = path_filename(p);
    char* dot = strrchr(filename, '.');
    char* ext = NULL;
    
    if (dot && dot != filename) {
        ext = strdup(dot);
    } else {
        ext = strdup("");
    }
    
    free(filename);
    return ext;
}


Path path_parent_path(const Path* p) {
    Path result = path_init("");
    const char* last_sep = strrchr(p->data, PATH_SEPARATOR);
    if (last_sep == p->data) {
        string_assign(&result, PATH_SEPARATOR_STR);
    } else if (last_sep != NULL) {
        string_assign_substr(&result, p->data, last_sep - p->data);
    }
    return result;
}


void path_change_extension(Path* p, const char* new_extension) {
    if (!p || !p->data || p->size == 0 || !new_extension) {
        return;
    }

    /* Find the last separator to isolate the filename portion */
    char* last_sep = strrchr(p->data, PATH_SEPARATOR);
    
    /* If a separator exists, the filename starts after it. Otherwise, the whole path is the filename. */
    char* filename = last_sep ? (last_sep + 1) : p->data;

    /* Find the last dot in the filename */
    char* dot = strrchr(filename, '.');

    /* Check if a dot was found AND it's not the first character (like ".gitignore") */
    if (dot && dot != filename) {
        const size_t dot_index = (size_t)(dot - p->data);
        p->data[dot_index] = '\0';
        p->size = dot_index;
    }

    string_append(p, new_extension);
}


Path path_absolute(const Path* p) {
    if (!p || !p->data) {
        return path_init("");
    }

    char* resolved_str = realpath(p->data, NULL);

    if (resolved_str != NULL) {
        Path abs_path = path_init(resolved_str);
        free(resolved_str);
        return abs_path;
    }

    return path_init("");
}

bool path_exists(const Path* p) {
    if (!p || !p->data) { return false; }    
    struct stat buffer;
    /* stat() returns 0 on success (file exists and is accessible) */
    return (stat(p->data, &buffer) == 0);
}


bool path_is_dir(const Path* p) {
    if (!p || !p->data) {
        return false;
    }
    
    struct stat buffer;
    if (stat(p->data, &buffer) != 0) {
        return false;
    }

    return S_ISDIR(buffer.st_mode);
}

bool path_is_regular_file(const Path* p) {
    if (!p || !p->data) {
        return false;
    }
    
    struct stat buffer;
    if (stat(p->data, &buffer) != 0) {
        return false;
    }
    
    return S_ISREG(buffer.st_mode);
}

Vector path_dir_iterator(const Path* p) {
    Vector entries = {0};
    vector_init(&entries, sizeof(Path), 4);
    if (!p || !p->data || !path_is_dir(p)) {
        return entries;
    }

    /* Open the directory stream */
    DIR* dir = opendir(p->data);
    if (!dir) { return entries; }

    struct dirent* entry;
    const size_t parent_len = p->size; 
    
    /* Check if the parent path already ends with a separator */
    const bool needs_separator = (parent_len > 0 && p->data[parent_len - 1] != PATH_SEPARATOR);

    /* Read each entry in the directory */
    while ((entry = readdir(dir)) != NULL) {
        /* Skip current "." and parent ".." directory references */
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        /* Calculate the required memory for the full child path */
        size_t name_len = strlen(entry->d_name);
        size_t full_len = parent_len + name_len + (needs_separator ? 1 : 0) + 1;
        
        char* full_path = (char*)malloc(full_len);
        if (full_path) {
            /* Construct the path avoiding double separators */
            if (needs_separator) {
                snprintf(full_path, full_len, "%s%c%s", p->data, PATH_SEPARATOR, entry->d_name);
            } else {
                snprintf(full_path, full_len, "%s%s", p->data, entry->d_name);
            }
            const Path tmp = path_init(full_path);
            vector_push_back(&entries, &tmp);
        }
    }

    closedir(dir);    
    return entries;
}


void path_dir_iterator_free(const Vector* entries) {
    if (!entries) { return; }
    
    Path* it = NULL;
    VECTOR_FOREACH(Path, it, entries) {
        path_destroy(it);
    }
}

void path_print(const Path* p) {
    string_print(p);
}