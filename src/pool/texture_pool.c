#include <raylib.h>
#include "../../include/pool/texture_pool.h"
#include "../../include/image.h"
#include "../../include/structure/map.h"


typedef struct {
    const char* key;
    Texture2D value;
} TextureMapItem;

bool is_initialized = false;
TextureMapItem* map = NULL;


/*
 * Loads an image from a custom Path and returns a Raylib Texture2D.
 * Uses the custom loadPixels() implementation to handle WebP and fallback formats.
 */
static Texture2D load_texture_custom(const path_t* path) {
    /* Initialize an empty texture (id = 0 means invalid/failed) */
    Texture2D texture = { 0 };

    /* 1. Get raw pixels using your custom loader */
    PixelBuffer buf = load_img_from_disk(path);

    /* 2. Check for failure (assuming the vector is empty on error) */
    if (!buf.data || buf.num_bytes == 0) {
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
        .data = buf.data,   /* Pointer to your Vector's raw char array */
        .width = buf.w,
        .height = buf.h,
        .mipmaps = 1,           /* Default is 1 (no extra mipmaps) */
        .format = pixel_format
    };

    /* 5. Upload the image data from RAM to the GPU */
    texture = LoadTextureFromImage(img);

    free(buf.data);
    return texture;
}


void texture_pool_init() {
    if (is_initialized) { return; };
    is_initialized = true;
    hashmap_new(map, hashmap_hash_str, hashmap_eq_str);
}


Texture2D texture_pool_load(const char* filepath) {
    Texture2D* texture = hashmap_get(map, filepath);
    if (texture == NULL) {
        string_t str = string_new();
        string_assign(&str, filepath);
        Texture2D t = load_texture_custom(&str);
        hashmap_put(map, filepath, t);
        return t;
    }
    return *texture;
}


Texture2D texture_pool_get(const char* filepath) {
    return *hashmap_get(map, filepath);
}


int texture_pool_unload(const char* filepath) {
    Texture2D* texture = hashmap_get(map, filepath);
    if (texture != NULL) {
        UnloadTexture(*texture);
        hashmap_remove(map, filepath);
        return 1;
    }
    return 0;
}


void texture_pool_clear() {
    hashmap_foreach(TextureMapItem, item, map) {
        UnloadTexture(item->value);
    }
    hashmap_clear(map);
}


void texture_pool_close() {
    texture_pool_clear();
    hashmap_free(map);
    map = NULL;
}