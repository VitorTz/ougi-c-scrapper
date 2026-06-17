#include "../../include/structure/map.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>


typedef struct MapAux {
    size_t hashed_key;
    Vector* bucket;
    Iterator bucket_iter;
    char* node;
} MapAux;


static MapAux get_map_aux(Map* map, const void* key) {
    const size_t hash_key = map->hash(key);
    Vector* bucket = (Vector*) vector_at(map->buckets, hash_key % map->nbuckets);
    return (MapAux){
        .hashed_key = hash_key,
        .bucket = bucket,
        .bucket_iter = vector_iter(bucket),
        .node = NULL
    };
}


Map* map_create(
    const size_t key_size, 
    const size_t value_size, 
    const size_t nbuckets,
    HashFunc hash    
) {
    Map* map = (Map*) malloc(sizeof(Map));
    map->nbuckets = nbuckets > 0 ? nbuckets : 4;
    map->size = 0;
    map->key_size = key_size;
    map->value_size = value_size;
    map->node_size = key_size + value_size;
    map->hash = hash;
    map->buckets = (Vector*) vector_create(sizeof(Vector), map->nbuckets);
    for (size_t i = 0; i < map->nbuckets; i++) {
        Vector* vec = (Vector*) vector_allocate(map->buckets);
        vector_init(vec, map->node_size, 4);
    }    
    return map;
}


void map_destroy(Map* map) {
    if (map == NULL) { return; }

    Iterator iter = vector_iter(map->buckets);
    Vector* it = NULL;

    while ((it = (Vector*) vector_iter_next(&iter)) != NULL) {
        vector_deinit(it);
    }
    vector_destroy(map->buckets);
    map->buckets = NULL;
    free(map);
}


void* map_allocate(Map* map, const void* key) {
    MapAux aux = get_map_aux(map, key);

    while ((aux.node = (char*) vector_iter_next(&aux.bucket_iter)) != NULL) {
        if (*((size_t*) aux.node) == aux.hashed_key) {
            return (void*) (aux.node + map->key_size);
        }
    }

    char* node = (char*) vector_allocate(aux.bucket);
    memcpy(node, &aux.hashed_key, map->key_size);
    map->size++;
    return (void*) (node + map->key_size);
}


void* map_at(Map* map, const void* key) {
    MapAux aux = get_map_aux(map, key);

    while ((aux.node = (char*) vector_iter_next(&aux.bucket_iter)) != NULL) {
        if (*((size_t*) aux.node) == aux.hashed_key) {
            return (void*) (aux.node + map->key_size);
        }
    }

    return NULL;
}

MapSearchResult map_search(Map* map, const void* key) {
    MapAux aux = get_map_aux(map, key);
    while ((aux.node = (char*) vector_iter_next(&aux.bucket_iter)) != NULL) {
        if (*((size_t*) aux.node) == aux.hashed_key) {
            return (MapSearchResult){
                .data = (void*) (aux.node + map->key_size),
                .exists = 1
            };
        }
    }

    char* node = (char*) vector_allocate(aux.bucket);
    memcpy(node, &aux.hashed_key, map->key_size);
    map->size++;

    return (MapSearchResult){
        .data = (void*) (node + map->key_size),
        .exists = 0
    };
}

void map_upsert(Map* map, const void* key, const void* value) {
    MapAux aux = get_map_aux(map, key);

    while ((aux.node = (char*) vector_iter_next(&aux.bucket_iter)) != NULL) {
        if (*((size_t*) aux.node) == aux.hashed_key) {
            memcpy(aux.node + map->key_size, value, map->value_size);
            return;
        }
    }
    char* node = (char*) vector_allocate(aux.bucket);
    memcpy(node, &aux.hashed_key, map->key_size);
    memcpy(node + map->key_size, value, map->value_size);
    map->size++;
}


void map_erase(Map* map, const void* key) {
    MapAux aux = get_map_aux(map, key);
    size_t i = 0;
    while ((aux.node = (char*) vector_iter_next(&aux.bucket_iter)) != NULL) {
        if (*((size_t*) aux.node) == aux.hashed_key) {
            vector_erase(aux.bucket, i);
            map->size--;
            return;
        }
        i += 1;
    }
}


void map_clear(Map* map) {
    if (map == NULL) { return; }
    map->size = 0;
    
    Iterator iter = vector_iter(map->buckets);
    Vector* it = NULL;
    while ((it = (Vector*) vector_iter_next(&iter)) != NULL) {
        vector_clear(it);
    }
}