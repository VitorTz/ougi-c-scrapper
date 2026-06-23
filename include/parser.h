#ifndef PARSER_H
#define PARSER_H

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structure/string_t.h"
#include "structure/vector.h"


typedef enum {
    OG_NODE_ELEMENT,
    OG_NODE_TEXT,
    OG_NODE_COMMENT,
    OG_NODE_DOCTYPE
} og_node_type_t;

typedef struct {
    char *key;
    char *value;
} og_attr_t;

struct og_node;
typedef struct og_node og_node_t;

typedef struct {
    og_node_t **items;
    size_t count;
    size_t capacity;
} og_node_list_t;

struct og_node {
    og_node_type_t type;
    char* tag;  /* Lowercase for elements */
    char* text; /* Text content for Text/Comment nodes */
    
    og_attr_t *attrs;
    size_t attr_count;
    size_t attr_capacity;
    
    og_node_list_t children;
    og_node_t *parent;
};

/* Initialize an empty node list */
void og_node_list_init(og_node_list_t *list);

/* Add a node pointer to the list */
void og_node_list_push(og_node_list_t *list, og_node_t *node);

/* Free the memory allocated by the node list array (not the nodes themselves) */
void og_node_list_free(og_node_list_t *list);

/* Get an attribute value */
const char* og_node_get_attr(const og_node_t *node, const char *key);

/* Check if node has a specific class */
bool og_node_has_class(const og_node_t *node, const char *cls);

/* Find all elements matching tag, id (optional), class (optional) */
og_node_list_t og_node_find_all(og_node_t *root, const char *tag, const char *id, const char *cls);

/* Find first element matching tag, id (optional), class (optional) */
og_node_t *og_node_find(og_node_t *root, const char *tag, const char *id, const char *cls);

og_node_t *og_parse_html(const char *html);

/* Recursively free a node and all its children */
void og_node_free(og_node_t *node);

string_t* extract_links(
    og_node_t* root,
    const char* tag,
    const char* id,
    const char* cls
);

string_t* extract_imgs(
    og_node_t* root,
    const char* tag,
    const char* id,
    const char* cls
);

#endif