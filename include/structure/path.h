#ifndef PATH_H
#define PATH_H
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "string_t.h"
#include "../types.h"
#include "../arena.h"


#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"

typedef string_t path_t;


/* Initializes the path with an optional starting string */
path_t path_create(const char* initial_path);

path_t path_create_copy(const path_t* path);

path_t path_tmp();

path_t path_create_empty();

path_t path_create_current_dir();

path_t path_manhwa_root_dir();

/* Frees the memory used by the path */
void path_destroy(path_t* p);

/* Appends a new component to the path (equivalent to operator/= or append) */
void path_append(path_t* p, const char* component);

void path_append_char(path_t* p, const char c);

/* Returns the C-style string representation (equivalent to c_str()) */
const char* path_c_str(const path_t* p);

/* Returns a newly allocated string with the filename component */
char* path_filename(const path_t* p);

/* Extracts the stem (filename without its final extension) from a path.
 * The caller is responsible for freeing the returned dynamically allocated string.
 */
char* path_stem(const path_t* path);

/* Returns a newly allocated string with the extension component */
char* path_extension(const path_t* p);

/*
 * Changes the extension of the given path in-place.
 * It only modifies the path if a valid extension already exists.
 * The new_extension should include the dot (e.g., ".png").
 */
void path_change_extension(path_t* p, const char* new_extension);


/* Returns a newly allocated string with the parent path */
path_t path_parent_path(const path_t* p);

/*
 * Returns a new Path containing the absolute, resolved path.
 * Resolves all symbolic links, extra '/' characters, and references to /./ and /../.
 * Returns an empty Path if the resolution fails (e.g., if the file does not exist).
 */
path_t path_absolute(const path_t* p);

/* Checks if the given path exists on the file system */
bool path_exists(const path_t* p);

/* Checks if the given path is a directory */
bool path_is_dir(const path_t* p);

/* Checks if the given path is a regular file (not a directory, symlink, etc.) */
bool path_is_regular_file(const path_t* p);

/*
 * Creates an empty file if it doesn't exist, or updates its access/modification 
 * timestamps if it already exists (similar to the Unix 'touch' command).
 */
bool path_touch(const path_t* p);

bool path_create_directories(const path_t* p);

/*
 * Moves or renames a file or directory from 'src' to 'dest'.
 * Note: If moving across different filesystems, this might fail as it relies on rename().
 */
bool path_move(const path_t* src, const path_t* dest);

/* Moves all files and directories from src_dir into dest_dir.
 * Both directories must already exist. 
 * Returns true if all items were moved successfully.
 */
bool path_move_directory_contents(const path_t* src_dir, const path_t* dest_dir);

/*
 * Copies a regular file from 'src' to 'dest'.
 * It copies the binary data and attempts to preserve the original file permissions.
 * Fails if 'src' is a directory or if the destination cannot be written.
 */
bool path_copy(const path_t* src, const path_t* dest);

/*
 * Deletes a file or directory. 
 * If the path is a directory, it recursively deletes all its contents first.
 */
bool path_delete(const path_t* p);


/* * Public API to delete all contents of a directory.
 * * @param path: The path_t struct containing the directory path.
 * @param keep_root_dir: If true, empties the directory but keeps the folder itself. 
 * If false, acts like 'rm -rf' and deletes the folder too.
 * @return: true on success, false on failure.
 */
bool path_delete_recursive(const path_t* path, bool keep_root_dir);

/*
 * Returns a null-terminated array of strings (char**) containing the absolute/relative
 * paths of all children inside the directory. Ignores "." and "..".
 * The caller is responsible for freeing each string and the array itself.
 * Returns NULL if the path is not a directory or if allocation fails.
 */
path_t* path_dir_iterator(const path_t* p, SortFunc sort_func, FilterFunc filter_func);


/* Helper to free the array returned by path_dir_iterator */
void path_dir_iterator_free(path_t* vec);


/*
 * Reads the entire contents of a file into a dynamically allocated Vector.
 * Returns a Vector containing the binary data. 
 * If the file cannot be opened, it logs an error and returns an empty Vector.
 */
Read path_read_bytes(const path_t* path);

/*
 * Reads an entire text file and returns it as a null-terminated string_t. 
 * Returns an empty string_t if the file cannot be opened.
 */
string_t path_read_text(const path_t* path);

/*
 * Writes a null-terminated C string to the specified file.
 * Opens the file in text mode ("w"), which handles OS-specific line endings.
 * Automatically creates parent directories if they don't exist.
 * Returns true on success, false on failure.
 */
bool path_write_text(const path_t* path, const char* text);

/*
 * Writes the entire raw Vector data to the specified file.
 * Opens the file in binary mode ("wb") to prevent byte corruption.
 * Automatically creates parent directories if they don't exist.
 * Returns true on success, false on failure.
 */
bool path_write_bytes(
     const path_t* path, 
     const uint8_t* data, 
     size_t size
);

void path_print(const path_t* p);


#endif