#include "../../include/pool/manhwa_pool.h"
#include "../../include/structure/map.h"
#include "../../include/cjson/cJSON.h"
#include "../../include/constants.h"
#include <stdio.h>
#include <stdlib.h>


typedef struct {
    string_t key;
    ManhwaScrap value;
} ManhwaMapItem;

static ManhwaMapItem* map = NULL;
static bool is_initialized = false;


typedef struct {
    string_t key;
    cJSON value;
} cJSONItem;


static cJSONItem* extract_map_from_json_item(cJSON* root) {
    cJSON* child = NULL;
    cJSONItem* m = NULL;
    hashmap_new(m, string_hash, string_eq);

    cJSON_ArrayForEach(child, root) {
        string_t key = string_from(child->string);
        hashmap_put(m, key, *child);
    }

    return m;
}

static string_t extract_string(cJSONItem* item_map, const char* key) {
    string_t s = string_from(key);
    cJSON* root = hashmap_get(item_map, s);
    string_assign(&s, root->valuestring);
    return s;
}


static ManhwaScrap create_manhwa_scrap(cJSONItem* item_map) {
    string_t title = extract_string(item_map, "title");
    string_t slug = string_clone(&title); string_slugify(&slug);
    string_t descr = extract_string(item_map, "descr");
    string_t url = extract_string(item_map, "url");
    string_t status = extract_string(item_map, "status");
    string_t source = extract_string(item_map, "source");
    path_t path = path_create(MANHWA_PATH_CSTR);
    path_append(&path, string_cstr(&title));
    path_create_directories(&path);
    return (ManhwaScrap){ };
}


void manhwa_pool_init() {
    if (is_initialized) { return; }
    is_initialized = true;
    hashmap_new(map, hashmap_hash_str, hashmap_eq_str);

    path_t path = path_create(MANHWA_SOURCE_PATH);
    string_t text = path_read_text(&path);

    cJSON* root = cJSON_Parse(string_cstr(&text));

    if (root == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error white reading manhwa source json: %s\n", error_ptr);
        }
        exit(EXIT_FAILURE);
    }

    cJSON* item = NULL;
    cJSON_ArrayForEach(item, root) {
        if (cJSON_IsObject(item)) {
            cJSONItem* itemMap = extract_map_from_json_item(item);
            const ManhwaScrap manhwa = create_manhwa_scrap(itemMap);
            hashmap_put(map, manhwa.title, manhwa);
            hashmap_free(itemMap);
        }
    }

    cJSON_Delete(root);
    string_free(&text);
    path_destroy(&path);
}


ManhwaScrap* manhwa_pool_get(const string_t* title) {
    return hashmap_get(map, *title);
}


void manhwa_pool_close() {
    hashmap_free(map);
}