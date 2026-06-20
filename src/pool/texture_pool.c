#include <raylib.h>
#include <string.h>
#include <stdio.h>
#include "../../include/pool/texture_pool.h"
#include "../../include/stb/image_utils.h"
#include "../../include/structure/map.h"
#include "../../include/hash.h"


static bool is_initialized = false;
static Map pool = {0};


/*
 * Loads an image from a custom Path and returns a Raylib Texture2D.
 * Uses the custom loadPixels() implementation to handle WebP and fallback formats.
 */
static Texture2D load_texture_custom(const Path* path) {
    /* Initialize an empty texture (id = 0 means invalid/failed) */
    Texture2D texture = { 0 };

    /* 1. Get raw pixels using your custom loader */
    const PixelBuffer buf = loadPixels(path);

    /* 2. Check for failure (assuming the vector is empty on error) */
    if (vector_is_empty(&buf.data) || buf.data.ptr == NULL) {
        /* Error is already logged inside loadPixels */
        return texture;
    }

    /* 3. Map your channel count to the correct Raylib PixelFormat */
    int pixel_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8; /* Default fallback */
    
    if (buf.ch == 1) {
        pixel_format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;
    } else if (buf.ch == 2) {
        pixel_format = PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA;
    } else if (buf.ch == 3) {
        pixel_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    } else if (buf.ch == 4) {
        pixel_format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    }

    /* 4. Construct a Raylib Image struct pointing to your raw data */
    Image img = {
        .data = buf.data.ptr,   /* Pointer to your Vector's raw char array */
        .width = buf.w,
        .height = buf.h,
        .mipmaps = 1,           /* Default is 1 (no extra mipmaps) */
        .format = pixel_format
    };

    /* 5. Upload the image data from RAM to the GPU */
    texture = LoadTextureFromImage(img);

    /* 6. Clean up CPU memory 
     * LoadTextureFromImage synchronously copies the data to OpenGL/VRAM.
     * Once it returns, the CPU buffer is no longer needed.
     */
    vector_destroy(&buf.data);

    return texture;
}


void texture_pool_init() {
    if (is_initialized) { return; };
    is_initialized = true;
    pool = map_create(
        sizeof(size_t), 
        sizeof(Texture2D), 
        128, 
        hash_string
    );
}


Texture2D texture_pool_load(const char* filepath) {
    MapSearchResult it = map_search(&pool, filepath);
    Texture2D texture = {0};
    if (it.exists) {
        texture = *((Texture2D*) it.data);
    } else {
        Path tmp = path_create(filepath);
        if (!path_exists(&tmp)) {
            path_destroy(&tmp);
            fprintf(stderr, "[FATAL ERROR] Texture file not found: %s\n", filepath);
            exit(EXIT_FAILURE);
        }
        texture = load_texture_custom(&tmp);
        memcpy(it.data, &texture, pool.value_size);
        path_destroy(&tmp);
    }
    return texture;
}


Texture2D texture_pool_get(const char* filepath) {
    Texture2D* texture = (Texture2D*) map_at(&pool, filepath);
    if (texture != NULL) { return *texture; }
    return (Texture2D){0};
}


int texture_pool_unload(const char* filepath) {
    Texture2D* texture = (Texture2D*) map_at(&pool, filepath);
    if (texture != NULL) {
        UnloadTexture(*texture);
        map_erase(&pool, filepath);
        return 1;
    }    
    return 0;
}


void texture_pool_clear() {
    MapIterator iter = map_iter(&pool);
    Texture2D* it = NULL;
    while ((it = (Texture2D*) map_iter_next(&iter)) != NULL) {
        UnloadTexture(*it);
    }
    map_clear(&pool);
}


void texture_pool_close() {
    texture_pool_clear();
    map_destroy(&pool);
}