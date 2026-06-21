#include <stdint.h>
#include <sys/stat.h>
#include <dirent.h>
#include <utime.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <webp/types.h>

#include "../../include/structure/path.h"
#include "../../include/structure/vector.h"



path_t path_create(const char* initial_path) {
    string_t str = string_new();
    string_assign(&str, initial_path);
    return str;
}


path_t path_tmp() {
    const path_t path = path_create("tmp");
    path_create_directories(&path);
    return path;
}


path_t path_create_copy(const path_t* path) {
    return path_create(path->data);
}


path_t path_create_empty() {
    return path_create("");
}


path_t path_create_current_dir(void) {
    char* cwd = getcwd(NULL, 0);
    
    if (cwd != NULL) {
        path_t current_path = path_create(cwd);
        free(cwd);
        
        return current_path;
    }

    return path_create_empty();
}


void path_destroy(path_t* path) {
    string_free(path);
}


void path_append(path_t* p, const char* component) {
    if (!component || strlen(component) == 0) {
        return;
    }

    bool path_ends_with_sep = string_ends_with(p, PATH_SEPARATOR_STR);
    bool comp_starts_with_sep = (component[0] == PATH_SEPARATOR);

    if (!path_ends_with_sep && !comp_starts_with_sep && !string_empty(p)) {
        string_push_back(p, PATH_SEPARATOR);
    } else if (path_ends_with_sep && comp_starts_with_sep) {
        component++;
    }
    string_append(p, component);
}


const char* path_c_str(const path_t* p) {
    return string_cstr(p);
}


char* path_filename(const path_t* p) {
    const char* last_sep = strrchr(p->data, PATH_SEPARATOR);
    if (last_sep) { return strdup(last_sep + 1); }
    return strdup(p->data);
}

char* path_stem(const path_t* path) {
    if (!path || !path->data) {
        return NULL;
    }

    const char* start = path->data;
    const char* end = path->data + path->length;

    /* Find the last path separator to isolate the filename */
    const char* last_sep = strrchr(path->data, PATH_SEPARATOR);
    
    if (last_sep) {
        /* Move the start pointer right after the separator */
        start = last_sep + 1;
    }

    /* Find the last dot in the isolated filename */
    const char* last_dot = strrchr(start, '.');

    /* Determine the end of the stem.
     * The condition (last_dot != start) ensures that hidden files 
     * like ".env" or ".gitignore" are treated entirely as the stem, 
     * rather than an empty stem with a long extension.
     */
    if (last_dot && last_dot != start) {
        end = last_dot;
    }

    /* Calculate the length of the stem */
    size_t stem_len = (size_t)(end - start);

    /* Allocate memory for the stem plus the null terminator */
    char* stem = (char*)malloc(stem_len + 1);
    
    if (!stem) {
        return NULL; /* Memory allocation failed */
    }

    /* Copy the characters and append the null terminator */
    if (stem_len > 0) {
        memcpy(stem, start, stem_len);
    }
    stem[stem_len] = '\0';

    return stem;
}


char* path_extension(const path_t* p) {
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


path_t path_parent_path(const path_t* p) {
    const size_t last_sep_index = string_rfind(p, PATH_SEPARATOR_STR);
    return string_from(p->data + last_sep_index);
}


void path_change_extension(path_t* p, const char* new_extension) {
    if (!p || !p->data || p->length == 0 || !new_extension) {
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
        p->length = dot_index;
    }

    string_append(p, new_extension);
}


path_t path_absolute(const path_t* p) {
    path_t path = string_new();
    if (!p || !p->data) {
        return path;
    }

    char* resolved_str = realpath(p->data, NULL);

    if (resolved_str != NULL) {
        string_assign(&path, resolved_str);
        free(resolved_str);
    }

    return path;
}

bool path_exists(const path_t* p) {
    if (!p || !p->data) { return false; }
    struct stat buffer;
    /* stat() returns 0 on success (file exists and is accessible) */
    return (stat(p->data, &buffer) == 0);
}


bool path_is_dir(const path_t* p) {
    if (!p || !p->data) {
        return false;
    }
    
    struct stat buffer;
    if (stat(p->data, &buffer) != 0) {
        return false;
    }

    return S_ISDIR(buffer.st_mode);
}

bool path_is_regular_file(const path_t* p) {
    if (!p || !p->data) {
        return false;
    }
    
    struct stat buffer;
    if (stat(p->data, &buffer) != 0) {
        return false;
    }
    
    return S_ISREG(buffer.st_mode);
}

bool path_touch(const path_t* p) {
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

bool path_create_directories(const path_t* p) {
    if (!p || !p->data || p->length == 0) {
        return false;
    }

    char* tmp = strdup(p->data);
    if (!tmp) { return false; }

    bool success = true;

    /* * Start at index 1 (tmp + 1) to safely handle absolute paths.
     * If the path starts with '/', we don't want to attempt mkdir("").
     */
    for (char* it = tmp + 1; *it != '\0'; it++) {
        if (*it == PATH_SEPARATOR) {
            /* Temporarily truncate the string to represent the parent directory */
            *it = '\0';

            /* * 0777 grants maximum permissions, but the OS automatically 
             * filters it through the user's 'umask' for secure defaults.
             */
            if (mkdir(tmp, 0777) != 0) {
                /* If mkdir fails, it is only acceptable if the directory already exists */
                if (errno != EEXIST) {
                    success = false;
                    break;
                }
            }

            /* Restore the separator to continue advancing down the path */
            *it = PATH_SEPARATOR;
        }
    }

    /* Attempt to create the final component of the path */
    if (success) {
        if (mkdir(tmp, 0777) != 0 && errno != EEXIST) {
            success = false;
        }
    }

    free(tmp);
    return success;
}


bool path_move(const path_t* src, const path_t* dest) {
    if (!src || !src->data || !dest || !dest->data) {
        return false;
    }
    return (rename(src->data, dest->data) == 0);
}

bool path_move_directory_contents(const path_t* src_dir, const path_t* dest_dir) {
    if (!src_dir || !dest_dir) {
        return false;
    }

    bool all_success = true;

    DIR* dir = opendir(src_dir->data);
    if (!dir) {
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        /* 4096 is standard PATH_MAX on most POSIX systems */
        char old_path[4096];
        char new_path[4096];

        snprintf(old_path, sizeof(old_path), "%s/%s", src_dir->data, entry->d_name);
        snprintf(new_path, sizeof(new_path), "%s/%s", dest_dir->data, entry->d_name);

        /* rename() works for both files and directories, moving them instantly */
        if (rename(old_path, new_path) != 0) {
            fprintf(stderr, "[Error] Failed to move: %s\n", old_path);
            all_success = false;
        }
    }

    closedir(dir);
    return all_success;
}


bool path_copy(const path_t* src, const path_t* dest) {
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


bool path_delete(const path_t* p) {
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
        size_t base_len = p->length;
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
                path_t child_path = path_create(child_str);
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

/* * Internal helper to perform the actual recursive deletion using raw C strings.
 * This avoids dependency on your specific string_t append/manipulation functions.
 */
static bool internal_remove_recursive(const char* dir_path, bool remove_root) {

    DIR* dir = opendir(dir_path);
    if (!dir) { return false; }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char child_path[4096];
        snprintf(child_path, sizeof(child_path), "%s/%s", dir_path, entry->d_name);

        struct stat statbuf;
        if (stat(child_path, &statbuf) == 0) {
            /* Check if the current entry is a directory */
            if (S_ISDIR(statbuf.st_mode)) {
                internal_remove_recursive(child_path, true); /* Recursively delete sub-directory */
            } else {
                unlink(child_path); /* Delete standard file */
            }
        }
    }
    closedir(dir);

    if (remove_root) {
        return rmdir(dir_path) == 0;
    }
    return true;
}


bool path_delete_recursive(const path_t* path, const bool keep_root_dir) {
    if (!path || !path->data) { return false; }
    return internal_remove_recursive(path->data, !keep_root_dir);
}


path_t* path_dir_iterator(const path_t* p, SortFunc sort_func, FilterFunc filter_func) {
    path_t* entries = NULL;

    if (!p || !p->data || !path_is_dir(p)) {
        return entries;
    }

    /* Open the directory stream */
    DIR* dir = opendir(p->data);
    if (!dir) { return entries; }

    struct dirent* entry;
    const size_t parent_len = p->length; 
    
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
            
            const path_t tmp = path_create(full_path);
            if (!filter_func || filter_func(&tmp)) {
                vec_push_back(entries, tmp);
            }
        }
    }

    closedir(dir);
    if (sort_func != NULL) { 
        vec_sort(entries, sort_func);
    }
    return entries;
}


void path_dir_iterator_free(path_t* entries) {
    if (!entries) { return; }    
    vec_foreach(path_t, item, entries) {
        path_destroy(item);
    }
}


Read path_read_bytes(const path_t* path) {
    Read read = {0};

    if (!path || !path_exists(path)) { return read; }

    FILE* f = fopen(path->data, "rb");
    if (!f) {
        fprintf(stderr, "[Error] Cannot open file for reading: %s\n", path->data);
        return read;
    }

    // Determine exact file size
    fseek(f, 0, SEEK_END);
    size_t file_size = ftell(f);
    rewind(f);

    if (file_size <= 0) {
        fclose(f);
        return read;
    }
    
    // Malloc
    read.data = malloc(sizeof(uint8_t) * file_size);

    // Read
    read.bytes = fread(read.data, sizeof(uint8_t), file_size, f);
    read.success = read.bytes == file_size;

    fclose(f);
    return read;
}


string_t path_read_text(const path_t* file_path) {
    string_t str = string_new();

    if (!file_path || !file_path->data) {
        return str;
    }

    /* Open in text mode */
    FILE* f = fopen(file_path->data, "r");
    if (!f) {
        fprintf(stderr, "[Error] Cannot open file for text reading: %s\n", file_path->data);
        return str;
    }

    char buffer[4096];
    
    /* * Read the file in chunks. 
     * We read sizeof(buffer) - 1 to guarantee space for a temporary null terminator 
     * if cstring_append requires it, though fread doesn't append '\0' naturally.
     */
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer) - 1, f)) > 0) {
        buffer[bytes_read] = '\0'; /* Temporarily terminate the chunk */
        string_append(&str, buffer);
    }
    
    fclose(f);
    
    return str;
}


bool path_write_text(const path_t* path, const char* text) {
    if (!path || !path->data || !text) {
        return false;
    }

    /* Extract the parent directory and ensure it exists */
    path_t parent_dir = path_parent_path(path);
    if (parent_dir.length > 0 && parent_dir.data != NULL) {
        path_create_directories(&parent_dir);
    }
    path_destroy(&parent_dir);

    /* Open the file in text write mode */
    FILE* f = fopen(path->data, "w");
    if (!f) {
        fprintf(stderr, "[Error] Cannot open for text writing: %s\n", path->data);
        return false;
    }

    bool success = true;
    
    /* Write the text only if the string is not empty */
    if (text[0] != '\0') {
        /* fputs returns EOF if a write error occurs */
        if (fputs(text, f) == EOF) {
            success = false;
        }
    }

    fclose(f);
    return success;
}


bool path_write_bytes(const path_t* path, const uint8_t* data, const size_t size) {
    if (!path || !path->data || !data || !data) {
        return false;
    }

    /* Open the file in binary write mode */
    FILE* f = fopen(path->data, "wb");
    if (!f) {
        fprintf(stderr, "[Error] Cannot open for binary writing: %s\n", path->data);
        return false;
    }

    size_t bytes_written = 0;
    
    /* Write the binary data block to the file */
    if (size > 0) {
        bytes_written = fwrite(data, 1, size, f);
    }

    fclose(f);
    
    /* Return true only if the exact amount of expected bytes was written to disk */
    return (bytes_written == size);
}


void path_print(const path_t* p) {
    printf("%s\n", p->data);
}