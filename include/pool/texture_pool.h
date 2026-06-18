#ifndef TEXTURE_POOL_H
#define TEXTURE_POOL_H
#include <raylib.h>
#include <stddef.h>


void texture_pool_init();

Texture2D texture_pool_load(const char* filepath);

Texture2D texture_pool_get(const char* filepath);

int texture_pool_unload(const char* filepath);

size_t texture_pool_size();

void texture_pool_clear();

void texture_pool_close();


#endif