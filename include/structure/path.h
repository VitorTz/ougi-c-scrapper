#ifndef PATH_H
#define PATH_H

#include <stddef.h>


// Linux
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"

/* Structure representing the filesystem path */
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} Path;

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

/* Returns a newly allocated string with the parent path */
char* path_parent_path(const Path* p);


#endif