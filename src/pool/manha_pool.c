#include <stdio.h>
#include <stdlib.h>
#include "../../include/pool/manhwa_pool.h"
#include "../../include/structure/map.h"
#include "../../include/structure/vector.h"
#include "../../include/cjson/cJSON.h"
#include "../../include/constants.h"
#include "../../include/uuid.h"


static ManhwaMapItem* map = NULL;
static bool is_initialized = false;
static string_t tmp_string;


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

static cJSON* extract_node(cJSONItem* item_map, const char* key) {
    string_assign(&tmp_string, key);
    cJSON* root = hashmap_get(item_map, tmp_string);
    if (root == NULL) {
        printf("Key not exists: %s\n", key);
        exit(EXIT_FAILURE);
    }
    return root;
}

static string_t extract_string(cJSONItem* item_map, const char* key) {
    cJSON* root = extract_node(item_map, key);
    return string_from(root->valuestring);
}

static float extract_float(cJSONItem* item_map, const char* key) {
    cJSON* root = extract_node(item_map, key);
    return root->valuedouble;
}

static float extract_int(cJSONItem* item_map, const char* key) {
    cJSON* root = extract_node(item_map, key);
    return root->valueint;
}


static string_t* extract_array(cJSONItem* item_map, const char* key) {
    cJSON* root = extract_node(item_map, key);
    string_t* arr = NULL;
    const int array_size = cJSON_GetArraySize(root);
    if (array_size > 0) {
        cJSON* first_element = cJSON_GetArrayItem(root, 0);
        if (cJSON_IsString(first_element)) {
            cJSON* element = NULL;
            cJSON_ArrayForEach(element, root) {
                string_t str = string_from(element->valuestring);
                vec_push_back(arr, str);
            }
        }
    }
    return arr;
}


static SourceID string_t_to_source(const string_t* str) {
    if (string_equals_cstr(str, "MangaDistrict")) {
        return MANGA_DISTRICT;
    } else if (string_equals_cstr(str, "Toondex")) {
        return TOONDEX;
    } else if (string_equals_cstr(str, "OmegaScans")) {
        return OMEGA_SCANS;
    }
    printf("Source %s not exists\n", string_cstr(str));
    exit(EXIT_FAILURE);
}


static ManhwaScrap create_manhwa_scrap(cJSONItem* item_map) {
    const string_t title = extract_string(item_map, "title");
    const string_t slug = string_slugify(&title);
    const string_t descr = extract_string(item_map, "descr");
    const string_t cover = extract_string(item_map, "cover");
    const string_t url = extract_string(item_map, "url");
    const string_t status = extract_string(item_map, "status");
    string_t* alternative_names = extract_array(item_map, "alternative_names");
    string_t* authors = extract_array(item_map, "authors");
    string_t* artists = extract_array(item_map, "artists");
    string_t* genres = extract_array(item_map, "genres");
    string_t* tags = extract_array(item_map, "tags");
    string_t* scans = extract_array(item_map, "scans");
    
    path_t path = path_create(MANHWA_PATH_CSTR);
    path_append(&path, string_cstr(&slug));
    path_create_directories(&path);
        
    string_t local_cover = extract_string(item_map, "local_cover");
    const path_t img_path = path_create(string_cstr(&local_cover));

    string_t source_str = extract_string(item_map, "source");
    const SourceID source = string_t_to_source(&source_str);
    const int release = extract_int(item_map, "release");

    const int should_ignore = extract_int(item_map, "should_ignore");
    const int should_overwrite = extract_int(item_map, "should_overwrite");    

    string_free(&local_cover);
    string_free(&source_str);

    return (ManhwaScrap){ 
        .title = title,
        .slug = slug,
        .descr = descr,
        .cover = cover,
        .id = uuid7(),
        .hex_color = string_new(),
        .url = url,
        .status = status,
        .alternative_names = alternative_names,
        .authors = authors,
        .artists = artists,
        .genres = genres,
        .tags = tags,
        .scans = scans,
        .path = path,
        .img_path = img_path,
        .source = source,
        .release = release,
        .should_ignore = should_ignore,
        .should_overwrite = should_overwrite
    };
}


void manhwa_pool_init() {
    if (is_initialized) { return; }
    is_initialized = true;
    hashmap_new(map, hashmap_hash_str, hashmap_eq_str);
    tmp_string = string_new();

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


ManhwaMapItem* manhwa_pool_map() {
    return map;
}


ManhwaScrap* manhwa_pool_get(const string_t* title) {
    return hashmap_get(map, *title);
}


ManhwaScrap* manhwa_pool_get_cstr(const char* title) {    
    string_assign(&tmp_string, title);
    return hashmap_get(map, tmp_string);
}


void manhwa_pool_close() {
    hashmap_foreach(ManhwaScrap, manhwa, map) {
        manhwa_scrap_free(manhwa);
    }
    hashmap_free(map);
    string_free(&tmp_string);
}