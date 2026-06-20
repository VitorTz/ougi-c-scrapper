#ifndef PATH_H
#define PATH_H
#include <stddef.h>
#include "cstring.h"


#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"

typedef CString Path;


/* Initializes the path with an optional starting string */
Path path_create(const char* initial_path);

Path path_create_copy(const Path* path);

Path path_create_empty();

Path path_create_current_dir();

/* Frees the memory used by the path */
void path_destroy(Path* p);

/* Appends a new component to the path (equivalent to operator/= or append) */
void path_append(Path* p, const char* component);

/* Returns the C-style string representation (equivalent to c_str()) */
const char* path_c_str(const Path* p);

/* Returns a newly allocated string with the filename component */
char* path_filename(const Path* p);

/* Returns a newly allocated string with the extension component */
char* path_extension(const Path* p);

/*
 * Changes the extension of the given path in-place.
 * It only modifies the path if a valid extension already exists.
 * The new_extension should include the dot (e.g., ".png").
 */
void path_change_extension(Path* p, const char* new_extension);


/* Returns a newly allocated string with the parent path */
Path path_parent_path(const Path* p);

/*
 * Returns a new Path containing the absolute, resolved path.
 * Resolves all symbolic links, extra '/' characters, and references to /./ and /../.
 * Returns an empty Path if the resolution fails (e.g., if the file does not exist).
 */
Path path_absolute(const Path* p);

/* Checks if the given path exists on the file system */
bool path_exists(const Path* p);

/* Checks if the given path is a directory */
bool path_is_dir(const Path* p);

/* Checks if the given path is a regular file (not a directory, symlink, etc.) */
bool path_is_regular_file(const Path* p);

/*
 * Creates an empty file if it doesn't exist, or updates its access/modification 
 * timestamps if it already exists (similar to the Unix 'touch' command).
 */
bool path_touch(const Path* p);

/*
 * Moves or renames a file or directory from 'src' to 'dest'.
 * Note: If moving across different filesystems, this might fail as it relies on rename().
 */
bool path_move(const Path* src, const Path* dest);

/*
 * Copies a regular file from 'src' to 'dest'.
 * It copies the binary data and attempts to preserve the original file permissions.
 * Fails if 'src' is a directory or if the destination cannot be written.
 */
bool path_copy(const Path* src, const Path* dest);

/*
 * Deletes a file or directory. 
 * If the path is a directory, it recursively deletes all its contents first.
 */
bool path_delete(const Path* p);

/*
 * Returns a null-terminated array of strings (char**) containing the absolute/relative
 * paths of all children inside the directory. Ignores "." and "..".
 * The caller is responsible for freeing each string and the array itself.
 * Returns NULL if the path is not a directory or if allocation fails.
 */
Vector path_dir_iterator(const Path* p, SortFunc sort_func, FilterFunc filter_func);

/* Helper to free the array returned by path_dir_iterator */
void path_dir_iterator_free(const Vector* vec);

void path_print(const Path* p);


/*
 * PATH_DIR_FOREACH: Safely iterates over a directory path.
 * Automatically allocates the Vector and frees it when the loop terminates 
 * (naturally or via 'break').
 * * Parameters:
 * it_name  - The variable name for the current Path* inside the loop.
 * path_ptr - Pointer to the target Path.
 * sort_f   - SortFunc callback (or NULL).
 * filter_f - FilterFunc callback (or NULL).
 */
#define PATH_DIR_FOREACH(it_name, path_ptr, sort_f, filter_f) \
    for (int _state_##it_name = 0; _state_##it_name < 1; ) \
        for (Vector _entries_##it_name = path_dir_iterator((path_ptr), (sort_f), (filter_f)); \
             _state_##it_name < 1; \
             path_dir_iterator_free(&_entries_##it_name), _state_##it_name = 1) \
            for (Iterator _iter_##it_name = vector_iter(&_entries_##it_name); \
                 _state_##it_name < 1; \
                 _state_##it_name = 1) \
                for (Path* it_name = NULL; \
                     (it_name = (Path*) vector_iter_next(&_iter_##it_name)) != NULL; \
                    )


#endif