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


Texture2D texture_pool_load(const char* filepath, const char* key) {
    MapSearchResult it = map_search(pool, key);
    Texture2D texture;
    if (it.exists) {
        texture = *((Texture2D*) it.data);
    } else {
        texture = LoadTexture(filepath);
        memcpy(it.data, &texture, pool->value_size);        
    }
    return texture;
}


Texture2D texture_pool_get(const char* key) {
    Texture2D texture = {0};
    
    void* data = map_at(pool, key);
    if (data != NULL) {
        texture = *((Texture2D*) data);
    }
    
    return texture;
}


int texture_pool_unload(const char *key) {
    Texture2D* texture = (Texture2D*) map_at(pool, key);
    if (texture != NULL) {
        UnloadTexture(*texture);
        map_erase(pool, key);
        return 1;
    }    
    return 0;
}


void texture_pool_clear() {
    Vector* vec = NULL;
    Iterator vec_iter = vector_iter(pool->buckets);
    while ((vec = (Vector*) vector_iter_next(&vec_iter)) != NULL) {
        char* node = NULL;
        Iterator node_iter = vector_iter(vec);
        while ((node = vector_iter_next(&node_iter)) != NULL) {
            Texture texture = *((Texture2D*) (node + pool->key_size));
            UnloadTexture(texture);
        }
        vector_clear(vec);
    }
    pool->size = 0;
}


void texture_pool_close() {
    texture_pool_clear();
    map_destroy(pool);
    pool = NULL;
}