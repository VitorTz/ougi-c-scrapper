#ifndef MAP_H
#define MAP_H
#include "vector.h"
#include <stdbool.h>
#include <stddef.h>

typedef size_t (*HashFunc)(const void* key);
typedef int (*EqualFunc)(const void* key1, const void* key2);

typedef struct Map {
    Vector buckets;
    size_t nbuckets;
    size_t size;
    size_t key_size;
    size_t value_size;
    size_t node_size;
    HashFunc hash;
} Map;

typedef struct MapIterator {
    const Map* map;
    Iterator vec_iter;
    size_t vec_index;
} MapIterator;

typedef struct MapSearchResult {
    void* data;
    int exists;
} MapSearchResult;

Map map_create(
    size_t key_size, 
    size_t value_size, 
    size_t nbuckets,
    HashFunc hash
);

void map_destroy(Map* map);

void* map_allocate(Map* map, const void* key);

void* map_at(const Map* map, const void* key);

void map_upsert(Map* map, const void* key, const void* value);

void map_erase(Map* map, const void* key);

void map_clear(Map* map);

bool map_contains(const Map* map, const void* key);

MapSearchResult map_search(Map* map, const void* key);

MapIterator map_iter(const Map* map);

void* map_iter_next(MapIterator* iter);

size_t map_size(const Map* map);


#endif