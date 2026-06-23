#ifndef MANHWA_POOL
#define MANHWA_POOL
#include "../scrapper/manhwa.h"


void manhwa_pool_init();


ManhwaScrap* manhwa_pool_get(const char* title);

void manhwa_pool_put(ManhwaScrap* manhwa);

void manhwa_pool_close();

#endif