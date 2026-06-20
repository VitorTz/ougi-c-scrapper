#ifndef UTIL_H
#define UTIL_H

/*
 * Parses the primary and secondary numbers from the filename.
 * Expected format: "path/to/file/XX_Y.webp" -> extracts XX and Y.
 */
void extract_file_numbers(const char* filepath, int* primary, int* secondary);


/*
 * Comparison function for qsort to sort CString items by their filename numbers.
 * Sorts ascendingly by the primary number, then by the secondary number.
 */
int compare_numbered_filenames(const void* a, const void* b);

#endif