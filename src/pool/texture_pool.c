#include "../../include/pool/texture_pool.h"
#include "../../include/structure/map.h"
#include "../../include/hash.h"
#include <raylib.h>
#include <string.h>


static Map* pool = NULL;


void texture_pool_init() {
    if (pool != NULL) { return; }
    pool = map_create(
        sizeof(size_t), 
        sizeof(Texture2D), 
        128, 
        hash_string
    );
}


Texture2D texture_pool_load(const char* filepath) {
    MapSearchResult it = map_search(pool, filepath);
    Texture2D texture = {0};
    if (it.exists) {
        texture = *((Texture2D*) it.data);
    } else {
        texture = LoadTexture(filepath);
        memcpy(it.data, &texture, pool->value_size);        
    }
    return texture;
}


Texture2D texture_pool_get(const char* filepath) {
    Texture2D* texture = (Texture2D*) map_at(pool, filepath);
    if (texture != NULL) { return *texture; }
    return (Texture2D){0};
}


int texture_pool_unload(const char* filepath) {
    Texture2D* texture = (Texture2D*) map_at(pool, filepath);
    if (texture != NULL) {
        UnloadTexture(*texture);
        map_erase(pool, filepath);
        return 1;
    }    
    return 0;
}


void texture_pool_clear() {
    MapIterator iter = map_iter(pool);
    Texture2D* it = NULL;
    while ((it = (Texture2D*) map_iter_next(&iter)) != NULL) {
        UnloadTexture(*it);
    }
    map_clear(pool);
}


void texture_pool_close() {
    texture_pool_clear();
    map_destroy(pool);
    pool = NULL;
}