#ifndef TYPE_TO_STR_H
#define TYPE_TO_STR_H

#include <stddef.h>
#include <stdbool.h>
#include <raylib.h>
#include "structure/string_t.h"


/* --- Integer Types --- */
char* size_t_to_string(const size_t i);
char* int_to_string(const int i);
char* unsigned_int_to_string(const unsigned int i);
char* long_to_string(const long i);
char* unsigned_long_to_string(const unsigned long i);
char* long_long_to_string(const long long i);
char* unsigned_long_long_to_string(const unsigned long long i);

/* --- Floating-Point Types --- */
char* float_to_string(const float f);
char* double_to_string(const double i);
char* long_double_to_string(const long double i);

/* --- Character & Boolean Types --- */
char* char_to_string(const char c);
char* bool_to_string(const bool b);

/* --- Pointer Types --- */
char* pointer_to_string(const void* ptr);

/* ---     Color     ---*/
Color hex_to_color(const string_t* str);

#endif /* TYPE_TO_STR_H */