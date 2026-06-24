#ifndef MANHWA_POOL
#define MANHWA_POOL
#include "../scrapper/manhwa.h"


typedef struct {
    string_t key;
    ManhwaScrap value;
} ManhwaMapItem;


void manhwa_pool_init();

ManhwaScrap* manhwa_pool_get(const string_t* title);

ManhwaMapItem* manhwa_pool_map();

void manhwa_pool_close();

#endif