#ifndef PATH_H
#define PATH_H
#include <stddef.h>
#include "cstring.h"


#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"

typedef CString Path;


/* Initializes the path with an optional starting string */
Path path_init(const char* initial_path);

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
 * Returns a null-terminated array of strings (char**) containing the absolute/relative
 * paths of all children inside the directory. Ignores "." and "..".
 * The caller is responsible for freeing each string and the array itself.
 * Returns NULL if the path is not a directory or if allocation fails.
 */
Vector path_dir_iterator(const Path* p);

/* Helper to free the array returned by path_dir_iterator */
void path_dir_iterator_free(const Vector* vec);

void path_print(const Path* p);


#endif