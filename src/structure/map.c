#include "../../include/structure/map.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>


#ifndef CMP_NODE_HASH
// Checks if the node value inside the provided struct matches its hashed_key
#define CMP_NODE_HASH(item) (*((size_t*)(item).node) == (item).hashed_key)
#endif

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

    Vector* it = NULL;
    VECTOR_FOREACH(Vector, it, map->buckets) {
        vector_deinit(it);
    }

    vector_destroy(map->buckets);
    map->buckets = NULL;
    free(map);
}


void* map_allocate(Map* map, const void* key) {
    MapAux aux = get_map_aux(map, key);
    while ((aux.node = (char*) vector_iter_next(&aux.bucket_iter)) != NULL) {
        if (aux.hashed_key == *((size_t*) aux.node)) {
            return aux.node + map->key_size;
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
        if (CMP_NODE_HASH(aux)) {
            return (void*) (aux.node + map->key_size);
        }
    }

    return NULL;
}

MapSearchResult map_search(Map* map, const void* key) {
    MapAux aux = get_map_aux(map, key);
    while ((aux.node = (char*) vector_iter_next(&aux.bucket_iter)) != NULL) {
        if (CMP_NODE_HASH(aux)) {
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
        if (CMP_NODE_HASH(aux)) {
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
        if (CMP_NODE_HASH(aux)) {
            vector_erase(aux.bucket, i);
            map->size--;
            return;
        }
        i += 1;
    }
}


void map_clear(Map* map) {
    if (map == NULL) { return; }
    Iterator iter = vector_iter(map->buckets);
    Vector* it = NULL;
    while ((it = (Vector*) vector_iter_next(&iter)) != NULL) {
        vector_clear(it);
    }
    map->size = 0;
}


bool map_contains(Map *map, const void *key) {
    return map_at(map, key) != NULL;
}


MapIterator map_iter(const Map* map) {
    const Vector* bucket = map->nbuckets > 0 ? (Vector*) vector_at(map->buckets, 0) : NULL;
    return (MapIterator){
        .map = map,
        .vec_iter = vector_iter(bucket),
        .vec_index = 0
    };
}


void* map_iter_next(MapIterator* iter) {
    if (iter->vec_index >= iter->map->nbuckets) {
        return NULL;
    }
    char* data = vector_iter_next(&iter->vec_iter);
    if (data != NULL) {
        return data + iter->map->key_size;
    }
    if (iter->vec_index + 1 < iter->map->nbuckets) {
        iter->vec_index++;
        iter->vec_iter = vector_iter(vector_at(iter->map->buckets, iter->vec_index));
        return map_iter_next(iter);
    }
    return NULL;
}


size_t map_size(const Map *map) {
    return map->size;
}