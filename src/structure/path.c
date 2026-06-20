#include <sys/stat.h>
#include <dirent.h>
#include <utime.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <webp/types.h>

#include "../../include/structure/path.h"


Path path_create(const char* initial_path) {
    return cstring_create(initial_path);
}


Path path_create_copy(const Path* path) {
    return path_create(path->data);
}


Path path_create_empty() {
    return path_create("");
}


Path path_create_current_dir(void) {
    char* cwd = getcwd(NULL, 0);
    
    if (cwd != NULL) {
        Path current_path = path_create(cwd);
        free(cwd);
        
        return current_path;
    }

    return path_create_empty();
}


void path_destroy(Path* path) {
    cstring_destroy(path);
}


void path_append(Path* p, const char* component) {
    if (!component || strlen(component) == 0) {
        return;
    }

    bool path_ends_with_sep = (p->size > 0 && p->data[p->size - 1] == PATH_SEPARATOR);
    bool comp_starts_with_sep = (component[0] == PATH_SEPARATOR);

    if (!path_ends_with_sep && !comp_starts_with_sep && p->size > 0) {
        cstring_push_back(p, PATH_SEPARATOR);
    } else if (path_ends_with_sep && comp_starts_with_sep) {
        component++;
    }

    cstring_append(p, component);
}


const char* path_c_str(const Path* p) {
    return cstring_c_str(p);
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
    Path result = path_create("");
    const char* last_sep = strrchr(p->data, PATH_SEPARATOR);
    if (last_sep == p->data) {
        cstring_assign(&result, PATH_SEPARATOR_STR);
    } else if (last_sep != NULL) {
        cstring_assign_substr(&result, p->data, last_sep - p->data);
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

    cstring_append(p, new_extension);
}


Path path_absolute(const Path* p) {
    Path path = path_create_empty();
    if (!p || !p->data) {
        return path;
    }

    char* resolved_str = realpath(p->data, NULL);

    if (resolved_str != NULL) {
        path_append(&path, resolved_str);
        free(resolved_str);
    }

    return path;
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

bool path_touch(const Path* p) {
    if (!p || !p->data) {
        return false;
    }

    if (utime(p->data, NULL) == 0) {
        return true;
    }

    /* 
        If utime fails, the file likely doesn't exist. Create an empty file. 
        Opening in "a" (append) mode creates it without overwriting existing content. 
    */
    FILE* f = fopen(p->data, "a");
    if (f != NULL) {
        fclose(f);
        return true;
    }

    return false;
}


bool path_move(const Path* src, const Path* dest) {
    if (!src || !src->data || !dest || !dest->data) {
        return false;
    }
    return (rename(src->data, dest->data) == 0);
}


bool path_copy(const Path* src, const Path* dest) {
    /* Safety check for null pointers */
    if (!src || !src->data || !dest || !dest->data) {
        return false;
    }

    /* Prevent attempting to copy a directory with this function */
    if (path_is_dir(src)) {
        return false;
    }

    /* Open source file in binary read mode */
    FILE* f_src = fopen(src->data, "rb");
    if (!f_src) {
        return false;
    }

    /* Open destination file in binary write mode */
    FILE* f_dest = fopen(dest->data, "wb");
    if (!f_dest) {
        fclose(f_src);
        return false;
    }

    /* * Use an 8KB buffer. This is a highly efficient chunk size for modern 
     * filesystems to minimize the number of system calls.
     */
    char buffer[8192];
    size_t bytes_read;
    bool success = true;

    /* Read from source and write to destination until EOF or an error occurs */
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f_src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, f_dest) != bytes_read) {
            success = false;
            break;
        }
    }

    /* Close both file descriptors */
    fclose(f_src);
    fclose(f_dest);

    if (success) {
        /* * Preserve the original file's permissions.
         * We read the permissions of 'src' and apply them to 'dest'.
         */
        struct stat st;
        if (stat(src->data, &st) == 0) {
            chmod(dest->data, st.st_mode);
        }
    } else {
        /* * Clean up: If the copy failed halfway (e.g., out of disk space),
         * delete the incomplete destination file to avoid corrupt data.
         */
        unlink(dest->data);
    }

    return success;
}


bool path_delete(const Path* p) {
    if (!p || !p->data) {
        return false;
    }

    struct stat st;
    
    /* * Use lstat instead of stat. 
     * If the path is a symbolic link pointing to a directory, we want to delete 
     * the link itself, NOT recursively delete the target directory's contents.
     */
    if (lstat(p->data, &st) != 0) {
        return false; /* Path does not exist or we don't have permission */
    }

    /* If it is a directory, we must delete all its contents first */
    if (S_ISDIR(st.st_mode)) {
        DIR* dir = opendir(p->data);
        if (!dir) {
            return false;
        }

        struct dirent* entry;
        size_t base_len = p->size;
        bool needs_separator = (base_len > 0 && p->data[base_len - 1] != PATH_SEPARATOR);

        while ((entry = readdir(dir)) != NULL) {
            /* Skip the current directory "." and parent directory ".." */
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            size_t name_len = strlen(entry->d_name);
            size_t child_len = base_len + name_len + (needs_separator ? 1 : 0) + 1;
            char* child_str = (char*)malloc(child_len);
            
            if (child_str != NULL) {
                /* Construct the absolute/relative path to the child */
                if (needs_separator) {
                    snprintf(child_str, child_len, "%s%c%s", p->data, PATH_SEPARATOR, entry->d_name);
                } else {
                    snprintf(child_str, child_len, "%s%s", p->data, entry->d_name);
                }

                /* Create a temporary Path struct and recursively delete it */
                Path child_path = path_create(child_str);
                path_delete(&child_path);
                path_destroy(&child_path);
                
                free(child_str);
            }
        }
        closedir(dir);
        
        /* After the directory is emptied, remove the directory itself */
        return (rmdir(p->data) == 0);
    } 
    /* If it is a regular file or a symbolic link, delete it directly */
    else {
        return (unlink(p->data) == 0);
    }
}


Vector path_dir_iterator(const Path* p, SortFunc sort_func, FilterFunc filter_func) {
    Vector entries = vector_create(sizeof(Path), 4);

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
            
            const Path tmp = path_create(full_path);            
            if (!filter_func || filter_func(&tmp)) {
                vector_push_back(&entries, &tmp);
            }
        }
    }

    closedir(dir);
    if (sort_func != NULL) { vector_sort(&entries, sort_func); }
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
    cstring_print(p);
}