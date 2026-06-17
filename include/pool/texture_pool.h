#ifndef TEXTURE_POOL_H
#define TEXTURE_POOL_H
#include <raylib.h>
#include <stddef.h>


void texture_pool_init();

Texture2D texture_pool_load(const char* filepath, const char* key);

Texture2D texture_pool_get(const char* key);

int texture_pool_unload(const char* key);

size_t texture_pool_size();

void texture_pool_clear();

void texture_pool_close();


#endif