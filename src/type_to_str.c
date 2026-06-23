#include "../include/type_to_str.h"
#include <stdio.h>


static char buffer[256];
static char true_str[] = "true";
static char false_str[] = "false";

/* --- Integer Types --- */

char* size_t_to_string(const size_t i) {
    snprintf(buffer, 32, "%zu", i);
    return buffer;
}
char* int_to_string(const int i) {    
    snprintf(buffer, sizeof(buffer), "%d", i);
    return buffer;
}

char* unsigned_int_to_string(const unsigned int i) {
    snprintf(buffer, sizeof(buffer), "%u", i);
    return buffer;
}

char* long_to_string(const long i) {
    snprintf(buffer, sizeof(buffer), "%ld", i);
    return buffer;
}

char* unsigned_long_to_string(const unsigned long i) {
    snprintf(buffer, sizeof(buffer), "%lu", i);
    return buffer;
}

char* long_long_to_string(const long long i) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%lld", i);
    return buffer;
}

char* unsigned_long_long_to_string(const unsigned long long i) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%llu", i);
    return buffer;
}

/* --- Floating-Point Types --- */

char* float_to_string(const float i) {
    snprintf(buffer, 32, "%.2f", i);
    return buffer;
}

char* double_to_string(const double i) {
    static char buffer[64];
    // Using .4f for a bit more precision than float, adjust as needed
    snprintf(buffer, sizeof(buffer), "%.4f", i); 
    return buffer;
}

char* long_double_to_string(const long double i) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.4Lf", i);
    return buffer;
}

/* --- Character & Boolean Types --- */

char* char_to_string(const char c) {
    static char buffer[8];
    snprintf(buffer, sizeof(buffer), "%c", c);
    return buffer;
}

char* bool_to_string(const bool b) {
    return b ? true_str : false_str;
}

/* --- Pointer Types --- */

char* pointer_to_string(const void* ptr) {
    // %p prints the memory address (usually in hex)
    snprintf(buffer, sizeof(buffer), "%p", ptr);
    return buffer;
}