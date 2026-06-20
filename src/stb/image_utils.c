#include <stdint.h>
#include <stdlib.h>
#include <raylib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <webp/encode.h>
#include <webp/decode.h>

#define IMAGE_UTILS_IMPL
#include "../../include/stb/image_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC
#include "../../include/stb/stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include "../../include/stb/stb_image_resize2.h"

#include "../../include/structure/vector.h"
#include "../../include/structure/path.h"


static size_t MIN_PIXEL_BUFFER_CAPACITY = 256 * 1024;


PixelBuffer loadPixels(const Path* path) {
    PixelBuffer buf = {
        .w = 0,
        .h = 0,
        .ch = 0,
        .out_ch = 0,
        .data = vector_create(sizeof(uint8_t), MIN_PIXEL_BUFFER_CAPACITY)
    };    

    const char* path_string = path_c_str(path);

    if (is_webp(path)) {
        /* Open the file manually in binary mode */
        FILE* f = fopen(path_string, "rb");
        if (!f) {
            fprintf(stderr, "Failed to open WebP file: %s\n", path_string);
            goto error;
        }

        /* Determine the file size to allocate a temporary buffer */
        fseek(f, 0, SEEK_END);
        long file_size = ftell(f);
        rewind(f);

        uint8_t* raw_data = (uint8_t*)malloc((size_t)file_size);
        if (!raw_data) {
            fprintf(stderr, "Failed to allocate memory for WebP file: %s\n", path_string);
            fclose(f);
            goto error;
        }

        /* Read the entire WebP file into memory and close the file pointer */
        size_t bytes_read = fread(raw_data, 1, (size_t)file_size, f);
        fclose(f);

        if (bytes_read != (size_t)file_size) {
            fprintf(stderr, "Failed to read full WebP file: %s\n", path_string);
            free(raw_data);
            goto error;
        }

        /* Decode the image using the raw bytes */
        uint8_t* px = WebPDecodeRGBA(raw_data, (size_t)file_size, &buf.w, &buf.h);
        free(raw_data); /* The raw file buffer is no longer needed */

        if (!px) {
            fprintf(stderr, "WebPDecodeRGBA failed: %s\n", path_string);
            goto error;
        }

        buf.ch = 4;
        buf.out_ch = 4;
        
        /* Copy the decoded pixels into the vector and free the libwebp buffer */
        bool assigned = vector_assign(&buf.data, px, (size_t)(buf.w * buf.h * 4));
        WebPFree(px);

        if (!assigned) {
            fprintf(stderr, "Failed to assign WebP pixels to vector: %s\n", path_string);
            goto error;
        }
        
        return buf;
    }

    /* Fallback to stb_image for non-WebP formats */
    int w, h, ch;
    stbi_uc* px = stbi_load(path_string, &w, &h, &ch, 0);
    
    if (!px) {
        const char* r = stbi_failure_reason();
        fprintf(stderr, "stb_image error: %s — %s\n", (r ? r : "?"), path_string);
        goto error;
    }

    buf.w = w;
    buf.h = h;
    buf.ch = ch;
    buf.out_ch = (ch == 4 || ch == 2) ? 4 : 3;

    /* Copy the loaded image into the vector and free the stb_image buffer */
    bool assigned = vector_assign(&buf.data, px, (size_t)(w * h * ch));
    stbi_image_free(px);

    if (!assigned) {
        fprintf(stderr, "Failed to assign stbi pixels to vector: %s\n", path_string);
        goto error;
    }
    
    if (vector_is_empty(&buf.data)) {
        fprintf(stderr, "Failed to load image (empty vector): %s — %s\n", path_string, stbi_failure_reason());
        goto error;
    }

    return buf;

/* Cleanup block to prevent memory leaks on failure */
error:
    vector_destroy(&buf.data);
    buf.data = (Vector){0};
    buf.w = 0;
    buf.h = 0;
    buf.ch = 0;
    buf.out_ch = 0;
    return buf;
}


bool is_webp(const Path *path) {
    FILE* f = fopen(path_c_str(path), "rb");
    if (!f) { return false; }

    char buf[12] = {0};
    const size_t readed_bytes = fread(buf, 1, sizeof(buf), f);
    
    fclose(f);
    return (
        readed_bytes == 12
        && memcmp(buf, "RIFF", 4) == 0 
        && memcmp(buf + 8, "WEBP", 4) == 0
    );
}


CString rgb_to_hex(const RGB rgb) {
    CString string = cstring_create("#");
    char hex_buf[7];
    snprintf(hex_buf, sizeof(hex_buf), "%02X%02X%02X", rgb.r, rgb.g, rgb.b);
    cstring_append(&string, hex_buf);
    return string;
}


Color hex_to_color(const CString *string) {
    if (cstring_size(string) != 7) {
        return WHITE;
    }

    const char* buffer = string->data + 1;
    const unsigned long hex_val = strtoul(buffer, NULL, 16);

    return (Color){
        .r = (unsigned char)((hex_val >> 16) & 0xFF),
        .g = (unsigned char)((hex_val >> 8) & 0xFF),
        .b = (unsigned char)(hex_val & 0xFF),
        .a = 255
    };
}