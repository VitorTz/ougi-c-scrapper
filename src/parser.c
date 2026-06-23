#include "../include/parser.h"


/* ========================================================================= *
 * Memory & String Helpers                                                   *
 * ========================================================================= */

/* Initialize an empty node list */
void og_node_list_init(og_node_list_t *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Add a node pointer to the list */
void og_node_list_push(og_node_list_t *list, og_node_t *node) {
    if (list->count >= list->capacity) {
        list->capacity = list->capacity == 0 ? 4 : list->capacity * 2;
        list->items = (og_node_t **)realloc(list->items, list->capacity * sizeof(og_node_t *));
    }
    list->items[list->count++] = node;
}

/* Free the memory allocated by the node list array (not the nodes themselves) */
void og_node_list_free(og_node_list_t *list) {
    if (list->items) {
        free(list->items);
    }
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

/* Convert string to lowercase in place */
static void og_to_lower(char *str) {
    if (!str) return;
    for (; *str; ++str) *str = tolower((unsigned char)*str);
}

/* Check if string is entirely whitespace */
static bool og_is_blank(const char *str) {
    if (!str) return true;
    while (*str) {
        if (!isspace((unsigned char)*str)) return false;
        str++;
    }
    return true;
}

/* ========================================================================= *
 * Node Management                                                           *
 * ========================================================================= */

/* Create a base node */
static og_node_t *og_node_create(og_node_type_t type, og_node_t *parent) {
    og_node_t *node = (og_node_t *)calloc(1, sizeof(og_node_t));
    if (!node) return NULL;
    node->type = type;
    node->parent = parent;
    og_node_list_init(&node->children);
    return node;
}

/* Create an element node */
static og_node_t *og_make_element(const char *tag, og_node_t *parent) {
    og_node_t *node = og_node_create(OG_NODE_ELEMENT, parent);
    if (tag) {
        node->tag = strdup(tag);
        og_to_lower(node->tag);
    }
    return node;
}

/* Create a text node */
static og_node_t *og_make_text(const char *text, og_node_t *parent) {
    og_node_t *node = og_node_create(OG_NODE_TEXT, parent);
    if (text) node->text = strdup(text);
    return node;
}

/* Add an attribute to a node */
static void og_node_add_attr(og_node_t *node, const char *key, const char *value) {
    if (!node || !key) return;
    
    if (node->attr_count >= node->attr_capacity) {
        node->attr_capacity = node->attr_capacity == 0 ? 4 : node->attr_capacity * 2;
        node->attrs = (og_attr_t *)realloc(node->attrs, node->attr_capacity * sizeof(og_attr_t));
    }
    
    og_attr_t *attr = &node->attrs[node->attr_count++];
    attr->key = strdup(key);
    og_to_lower(attr->key);
    attr->value = value ? strdup(value) : strdup("");
}

/* Get an attribute value */
const char *og_node_get_attr(const og_node_t *node, const char *key) {
    if (!node || !key) return NULL;
    for (size_t i = 0; i < node->attr_count; i++) {
        if (strcasecmp(node->attrs[i].key, key) == 0) {
            return node->attrs[i].value;
        }
    }
    return NULL;
}

/* Check if node has a specific class */
bool og_node_has_class(const og_node_t *node, const char *cls) {
    const char *class_attr = og_node_get_attr(node, "class");
    if (!class_attr || !cls) return false;
    
    size_t cls_len = strlen(cls);
    const char *p = class_attr;
    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        const char *end = p;
        while (*end && !isspace((unsigned char)*end)) end++;
        
        if ((size_t)(end - p) == cls_len && strncmp(p, cls, cls_len) == 0) {
            return true;
        }
        p = end;
    }
    return false;
}

/* Recursively free a node and all its children */
void og_node_free(og_node_t *node) {
    if (!node) return;
    
    for (size_t i = 0; i < node->children.count; i++) {
        og_node_free(node->children.items[i]);
    }
    og_node_list_free(&node->children);
    
    for (size_t i = 0; i < node->attr_count; i++) {
        free(node->attrs[i].key);
        free(node->attrs[i].value);
    }
    if (node->attrs) free(node->attrs);
    
    if (node->tag) free(node->tag);
    if (node->text) free(node->text);
    
    free(node);
}

/* ========================================================================= *
 * Element Classification                                                    *
 * ========================================================================= */

static bool og_is_void_element(const char *tag) {
    static const char *voids[] = {
        "area", "base", "br", "col", "embed", "hr", "img",
        "input", "link", "meta", "param", "source", "track", "wbr", NULL
    };
    for (int i = 0; voids[i] != NULL; i++) {
        if (strcasecmp(tag, voids[i]) == 0) return true;
    }
    return false;
}

static bool og_is_raw_element(const char *tag) {
    static const char *raws[] = {
        "script", "style", "textarea", "title", NULL
    };
    for (int i = 0; raws[i] != NULL; i++) {
        if (strcasecmp(tag, raws[i]) == 0) return true;
    }
    return false;
}

/* ========================================================================= *
 * Minimalist CSS Selector Matching (Tag, ID, Class)                         *
 * ========================================================================= */

/* Internal matching logic for recursive search */
static void og_collect_matches(
    og_node_t      *node,
    const char     *tag,
    const char     *id,
    const char     *cls,
    og_node_list_t *out
) {
    for (size_t i = 0; i < node->children.count; i++) {
        og_node_t *child = node->children.items[i];
        if (child->type != OG_NODE_ELEMENT) continue;

        bool match = true;

        if (tag && strcmp(tag, "*") != 0 && strcasecmp(child->tag, tag) != 0)
            match = false;

        if (match && id) {
            const char *node_id = og_node_get_attr(child, "id"); /* pode ser NULL */
            if (!node_id || strcasecmp(node_id, id) != 0)
                match = false;
        }

        if (match && cls && !og_node_has_class(child, cls))
            match = false;

        if (match)
            og_node_list_push(out, child);

        og_collect_matches(child, tag, id, cls, out);
    }
}

/* Find all elements matching tag, id (optional), class (optional) */
og_node_list_t og_node_find_all(og_node_t *root, const char *tag, const char *id, const char *cls) {
    og_node_list_t results;
    og_node_list_init(&results);
    og_collect_matches(root, tag, id, cls, &results);
    return results;
}

/* Find first element matching tag, id (optional), class (optional) */
og_node_t *og_node_find(og_node_t *root, const char *tag, const char *id, const char *cls) {
    og_node_list_t results = og_node_find_all(root, tag, id, cls);
    og_node_t *first = NULL;
    if (results.count > 0) {
        first = results.items[0];
    }
    og_node_list_free(&results);
    return first;
}

/* ========================================================================= *
 * Basic Tokenizer and Parser                                                *
 * ========================================================================= */

/* Advanced entity decoding omitted for brevity, returns a duplicated string */
static char *og_decode_entities(const char *src, size_t len) {
    char *dest = (char *)malloc(len + 1);
    strncpy(dest, src, len);
    dest[len] = '\0';
    return dest;
}


og_node_t *og_parse_html(const char *html) {
    og_node_t *root = og_make_element("#root", NULL);
    og_node_t *cur = root;
    const char *p = html;
    
    while (*p) {
        if (*p == '<') {
            const char *tag_start = p + 1;
            
            /* Handle Comments */
            if (strncmp(tag_start, "!--", 3) == 0) {
                const char *end = strstr(tag_start + 3, "-->");
                if (end) {
                    size_t len = end - (tag_start + 3);
                    char *comment_txt = og_decode_entities(tag_start + 3, len);
                    og_node_t *comment = og_node_create(OG_NODE_COMMENT, cur);
                    comment->text = comment_txt;
                    og_node_list_push(&cur->children, comment);
                    p = end + 3;
                } else {
                    break;
                }
                continue;
            }
            
            /* Handle DOCTYPE */
            if (*tag_start == '!' || *tag_start == '?') {
                while (*p && *p != '>') p++;
                if (*p == '>') p++;
                continue;
            }
            
            /* Handle Closing Tag */
            bool is_close = false;
            if (*tag_start == '/') {
                is_close = true;
                tag_start++;
            }
            
            /* Read Tag Name */
            const char *tag_end = tag_start;
            while (*tag_end && (isalnum((unsigned char)*tag_end) || *tag_end == '-')) tag_end++;
            
            size_t tag_len = tag_end - tag_start;
            char *tag_name = strndup(tag_start, tag_len);
            og_to_lower(tag_name);
            p = tag_end;
            
            if (is_close) {
                while (*p && *p != '>') p++;
                if (*p == '>') p++;
                
                /* Walk up tree to find matching open tag */
                og_node_t *temp = cur;
                while (temp && temp->tag && strcmp(temp->tag, tag_name) != 0 && strcmp(temp->tag, "#root") != 0) {
                    temp = temp->parent;
                }
                if (temp && temp->tag && strcmp(temp->tag, tag_name) == 0) {
                    cur = temp->parent ? temp->parent : root;
                }
                free(tag_name);
                continue;
            }
            
            /* Create New Element */
            og_node_t *node = og_make_element(tag_name, cur);
            free(tag_name);
            
            /* Parse Attributes */
            while (*p && *p != '>' && *p != '/') {
                while (*p && isspace((unsigned char)*p)) p++;
                if (*p == '>' || *p == '/') break;
                
                const char *attr_start = p;
                while (*p && (isalnum((unsigned char)*p) || *p == '-' || *p == '_')) p++;
                size_t attr_len = p - attr_start;
                if (attr_len == 0) { p++; continue; }
                
                char *attr_name = strndup(attr_start, attr_len);
                while (*p && isspace((unsigned char)*p)) p++;
                
                char *attr_val = NULL;
                if (*p == '=') {
                    p++;
                    while (*p && isspace((unsigned char)*p)) p++;
                    if (*p == '"' || *p == '\'') {
                        char q = *p++;
                        const char *val_start = p;
                        while (*p && *p != q) p++;
                        attr_val = og_decode_entities(val_start, p - val_start);
                        if (*p == q) p++;
                    } else {
                        const char *val_start = p;
                        while (*p && !isspace((unsigned char)*p) && *p != '>') p++;
                        attr_val = og_decode_entities(val_start, p - val_start);
                    }
                }
                og_node_add_attr(node, attr_name, attr_val);
                free(attr_name);
                if (attr_val) free(attr_val);
            }
            
            bool self_close = false;
            if (*p == '/') {
                self_close = true;
                p++;
            }
            if (*p == '>') p++;
            
            og_node_list_push(&cur->children, node);
            
            if (!self_close && !og_is_void_element(node->tag)) {
                cur = node;
                
                /* Handle raw text inside script/style */
                if (og_is_raw_element(node->tag)) {
                    char close_tag[64];
                    snprintf(close_tag, sizeof(close_tag), "</%s", node->tag);
                    const char *raw_end = strcasestr(p, close_tag);
                    if (raw_end) {
                        char *raw_txt = og_decode_entities(p, raw_end - p);
                        og_node_list_push(&cur->children, og_make_text(raw_txt, cur));
                        free(raw_txt);
                        p = raw_end;
                    }
                }
            }
        } else {
            /* Parse Text Node */
            const char *txt_start = p;
            while (*p && *p != '<') p++;
            char *text = og_decode_entities(txt_start, p - txt_start);
            
            if (!og_is_blank(text)) {
                og_node_list_push(&cur->children, og_make_text(text, cur));
            }
            free(text);
        }
    }
    
    return root;
}

static int comp_links(const void* a, const void* b) {
    return strcmp(((const string_t*) a)->data, ((const string_t*) b)->data);
}


string_t* extract_links(
    og_node_t* root,
    const char* tag,
    const char* id,
    const char* cls
) {
    og_node_list_t final_links;
    og_node_list_init(&final_links);

    /* Step 1: Find all list items representing the chapters */
    og_node_list_t chapter_items = og_node_find_all(root, tag, id, cls);

    for (size_t i = 0; i < chapter_items.count; i++) {
        og_node_t *li_node = chapter_items.items[i];
        
        /* Step 2: Find all anchor tags inside this specific list item */
        og_node_list_t anchors = og_node_find_all(li_node, "a", NULL, NULL);
        
        for (size_t j = 0; j < anchors.count; j++) {
            og_node_t *a_node = anchors.items[j];
            
            /* Step 3: Filter anchors that actually have an 'href' attribute */
            if (og_node_get_attr(a_node, "href") != NULL) {
                og_node_list_push(&final_links, a_node);
            }
        }
        
        /* Clean up the temporary anchors list (this frees the array, not the nodes) */
        og_node_list_free(&anchors);
    }

    /* Clean up the temporary chapter items list */
    og_node_list_free(&chapter_items);

    string_t* result = NULL;
    for (size_t i = 0; i < final_links.count; i++) {
        const char *href = og_node_get_attr(final_links.items[i], "href");
        string_t t = string_from(href);
        if (!vec_contains(result, t, comp_links)) {
            vec_push_back(result, t);
        }
    }

    og_node_list_free(&final_links);
    return result;
}


string_t* extract_imgs(
    og_node_t* root,
    const char* tag,
    const char* id,
    const char* cls
) {
    /* Initialize our generic vector to hold char* (strings) */
    string_t* images = NULL;

    /* Find all matching elements: bs.find_all("img.wp-manga-chapter-img") */
    og_node_list_t imgs = og_node_find_all(root, tag, id, cls);

    for (size_t i = 0; i < imgs.count; i++) {
        og_node_t *node = imgs.items[i];
        
        /* Prioritize 'src', fallback to 'data-src' */
        const char *src = og_node_get_attr(node, "src");
        if (!src || src[0] == '\0') {
            src = og_node_get_attr(node, "data-src");
        }

        /* If neither exists, skip */
        if (!src) {
            continue;
        }

        /* Strip whitespace from the URL (allocates a new string) */
        string_t clean_src = string_from(src);
        string_trim(&clean_src);
        if (string_empty(&clean_src)) {
            continue;
        }

        /* Check emptiness and custom ignore logic */
        if (clean_src.data[0] == '\0') {
            string_free(&clean_src);
            continue;
        }

        /* Push the dynamically allocated string into our vector */
        vec_push_back(images, clean_src);
    }

    /* Clean up DOM and temporary resources */
    og_node_list_free(&imgs);

    if (vec_len(images) == 0) {
        vec_free(images);
        return NULL;
    }

    return images;
}