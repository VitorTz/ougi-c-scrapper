#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

/*
 * image_utils.h
 *
 * Dependencies:
 *   - libcurl       (download)
 *   - libwebp       (encode/compress WebP)
 *   - stb_image     (decode any image format)
 *   - stb_image_resize2 (resize)
 *
 * Linux:   sudo apt install libcurl4-openssl-dev libwebp-dev
 *
 * In exactly ONE .c file, before including this header, define:
 *
 *   #define IMAGE_UTILS_IMPL
 *   #include "image_utils.h"
 */

#include <string.h>
#include <stdbool.h>
#include <raylib.h>
#include "../types.h"
#include "../structure/path.h"
#include "../structure/cstring.h"


Path download(
    const char* url, 
    const Path* path,
    const char* referer
);


Path to_webp(
    const Path* input,
    const Path* output,
    bool lossless
);


Path compress_webp(
    const Path* input_path,
    const Path* output_path,
    float quality
);


Path resize(
    const Path* input_path,
    int max_width, 
    int max_height,
    float quality, 
    bool encode
);

bool is_webp(const Path* path);

CString rgb_to_hex(const RGB rgb);

Color hex_to_color(const CString* string);

#endif
